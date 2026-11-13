#include "HunterMonsterBase.h"
#include "Items/HunterPickupActor.h"

#include "ArinValeCharacter.h"
#include "ArinProgressionComponent.h"
#include "Components/HunterCurrencyComponent.h"
#include "Quests/HunterQuestSubsystem.h"
#include "Engine/GameInstance.h"

#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AHunterMonsterBase::AHunterMonsterBase()
{
    PrimaryActorTick.bCanEverTick = true;
    GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

    // Temporary readable silhouette. The inherited capsule remains responsible for collision.
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (!CubeMesh.Succeeded() || !SphereMesh.Succeeded())
    {
        return;
    }

    auto AddVisual = [this](const FName Name, UStaticMesh* Mesh, const FVector& Location, const FVector& Scale)
    {
        UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
        Part->SetupAttachment(GetRootComponent());
        Part->SetStaticMesh(Mesh);
        Part->SetRelativeLocation(Location);
        Part->SetRelativeScale3D(Scale);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Part->SetGenerateOverlapEvents(false);
        Part->SetCanEverAffectNavigation(false);
        BlockoutVisuals.Add(Part);
    };

    AddVisual(TEXT("MonsterTorso"), CubeMesh.Object, FVector(0, 0, 4), FVector(0.78f, 0.58f, 1.02f));
    AddVisual(TEXT("MonsterShoulderLeft"), CubeMesh.Object, FVector(0, -48, 42), FVector(0.52f, 0.46f, 0.48f));
    AddVisual(TEXT("MonsterShoulderRight"), CubeMesh.Object, FVector(0, 48, 42), FVector(0.52f, 0.46f, 0.48f));
    AddVisual(TEXT("MonsterArmLeft"), CubeMesh.Object, FVector(0, -55, -28), FVector(0.34f, 0.34f, 0.78f));
    AddVisual(TEXT("MonsterArmRight"), CubeMesh.Object, FVector(0, 55, -28), FVector(0.34f, 0.34f, 0.78f));
    AddVisual(TEXT("MonsterLegLeft"), CubeMesh.Object, FVector(0, -24, -88), FVector(0.34f, 0.34f, 0.72f));
    AddVisual(TEXT("MonsterLegRight"), CubeMesh.Object, FVector(0, 24, -88), FVector(0.34f, 0.34f, 0.72f));
    AddVisual(TEXT("MonsterHead"), SphereMesh.Object, FVector(0, 0, 104), FVector(0.52f, 0.48f, 0.58f));
    AddVisual(TEXT("MonsterHornLeft"), CubeMesh.Object, FVector(0, -30, 145), FVector(0.12f, 0.12f, 0.48f));
    AddVisual(TEXT("MonsterHornRight"), CubeMesh.Object, FVector(0, 30, 145), FVector(0.12f, 0.12f, 0.48f));
}

void AHunterMonsterBase::BeginPlay()
{
    Super::BeginPlay();
    // Normalize editable Blueprint defaults before they are used by combat or movement.
    MaxHealth = FMath::IsFinite(MaxHealth) ? FMath::Max(1.0f, MaxHealth) : 120.0f;
    CurrentHealth = MaxHealth;
    DetectionRadius = FMath::IsFinite(DetectionRadius) ? FMath::Max(0.0f, DetectionRadius) : 1400.0f;
    AttackRange = FMath::IsFinite(AttackRange) ? FMath::Max(0.0f, AttackRange) : 180.0f;
    AttackDamage = FMath::IsFinite(AttackDamage) ? FMath::Max(0.0f, AttackDamage) : 12.0f;
    AttackCooldown = FMath::IsFinite(AttackCooldown) ? FMath::Max(0.1f, AttackCooldown) : 1.6f;
    ChaseSpeed = FMath::IsFinite(ChaseSpeed) ? FMath::Max(0.0f, ChaseSpeed) : 430.0f;
    ChaseRepathInterval = FMath::IsFinite(ChaseRepathInterval) ? FMath::Max(0.05f, ChaseRepathInterval) : 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
    FindTarget();
}

void AHunterMonsterBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bIsDead)
    {
        return;
    }

    AttackTimer = FMath::Max(0.0f, AttackTimer - DeltaSeconds);
    ChaseRepathTimer = FMath::Max(0.0f, ChaseRepathTimer - DeltaSeconds);

    if (!TargetCharacter.IsValid())
    {
        FindTarget();
    }

    UpdateCombat(DeltaSeconds);
}

void AHunterMonsterBase::FindTarget()
{
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (PlayerCharacter && FVector::DistSquared(GetActorLocation(), PlayerCharacter->GetActorLocation()) <= FMath::Square(DetectionRadius))
    {
        TargetCharacter = PlayerCharacter;
    }
}

void AHunterMonsterBase::UpdateCombat(float DeltaSeconds)
{
    ACharacter* Target = TargetCharacter.Get();
    if (!Target || Target->IsActorBeingDestroyed())
    {
        TargetCharacter.Reset();
        return;
    }

    const float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (Distance > DetectionRadius * 1.35f)
    {
        TargetCharacter.Reset();
        if (AAIController* AI = Cast<AAIController>(GetController()))
        {
            AI->StopMovement();
        }
        return;
    }

    if (Distance > AttackRange)
    {
        if (bCanChasePlayer && ChaseRepathTimer <= 0.0f)
        {
            if (AAIController* AI = Cast<AAIController>(GetController()))
            {
                AI->MoveToActor(Target, AttackRange * 0.8f);
                ChaseRepathTimer = FMath::Max(0.05f, ChaseRepathInterval);
            }
        }
        return;
    }

    if (AttackTimer <= 0.0f)
    {
        AttackTimer = FMath::Max(0.1f, AttackCooldown);
        BP_OnMonsterAttack(Target);
        UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, nullptr);
    }
}

float AHunterMonsterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (!FMath::IsFinite(DamageAmount) || DamageAmount <= 0.0f || bIsDead)
    {
        return 0.0f;
    }

    const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (!FMath::IsFinite(AppliedDamage) || AppliedDamage <= 0.0f)
    {
        return 0.0f;
    }

    ReceiveHunterDamage(AppliedDamage, DamageCauser);
    return AppliedDamage;
}

void AHunterMonsterBase::ReceiveHunterDamage(float DamageAmount, AActor* DamageSource)
{
    if (bIsDead || !FMath::IsFinite(DamageAmount) || DamageAmount <= 0.0f)
    {
        return;
    }

    const float SafeMaxHealth = FMath::IsFinite(MaxHealth) ? FMath::Max(1.0f, MaxHealth) : 120.0f;
    CurrentHealth = FMath::Clamp(
        (FMath::IsFinite(CurrentHealth) ? CurrentHealth : SafeMaxHealth) - DamageAmount,
        0.0f,
        SafeMaxHealth);

    if (!TargetCharacter.IsValid())
    {
        if (ACharacter* PlayerCharacter = Cast<ACharacter>(DamageSource))
        {
            TargetCharacter = PlayerCharacter;
        }
        else
        {
            FindTarget();
        }
    }

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
}

float AHunterMonsterBase::GetHealthPercent() const
{
    const float SafeMaxHealth = FMath::IsFinite(MaxHealth) ? FMath::Max(1.0f, MaxHealth) : 120.0f;
    const float SafeCurrentHealth = FMath::IsFinite(CurrentHealth) ? CurrentHealth : SafeMaxHealth;
    return FMath::Clamp(SafeCurrentHealth / SafeMaxHealth, 0.0f, 1.0f);
}

void AHunterMonsterBase::Die()
{
    if (bIsDead)
    {
        return;
    }

    bIsDead = true;
    GetCharacterMovement()->DisableMovement();
    SetActorEnableCollision(false);
    BP_OnMonsterDeath();

    if (AArinValeCharacter* Arin = Cast<AArinValeCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
    {
        if (UArinProgressionComponent* Progression = Arin->GetProgressionComponent())
        {
            Progression->AddExperience(FMath::Max(0, ExperienceReward));
        }
        if (UHunterCurrencyComponent* Currency = Arin->GetCurrencyComponent())
        {
            Currency->AddCurrency(FMath::Max<int64>(0, CurrencyReward));
        }
    }

    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UHunterQuestSubsystem* QuestSubsystem = GameInstance->GetSubsystem<UHunterQuestSubsystem>())
        {
            if (!DefeatQuestEventTag.IsNone())
            {
                QuestSubsystem->RegisterQuestEvent(DefeatQuestEventTag, 1);
            }
        }
    }

    SpawnLootDrops();
    OnMonsterDied.Broadcast(this);
    SetLifeSpan(4.0f);
}

void AHunterMonsterBase::SpawnLootDrops()
{
    if (!PickupActorClass || LootDrops.IsEmpty() || !GetWorld())
    {
        return;
    }

    for (const FHunterMonsterLootDrop& Drop : LootDrops)
    {
        if (Drop.Item.ItemId.IsNone() || Drop.Item.Quantity <= 0 ||
            FMath::FRand() > FMath::Clamp(Drop.DropChance, 0.0f, 1.0f))
        {
            continue;
        }

        const FVector Offset(FMath::FRandRange(-75.0f, 75.0f), FMath::FRandRange(-75.0f, 75.0f), 35.0f);
        const FTransform SpawnTransform(FRotator::ZeroRotator, GetActorLocation() + Offset);
        AHunterPickupActor* Pickup = GetWorld()->SpawnActorDeferred<AHunterPickupActor>(
            PickupActorClass, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
        if (Pickup)
        {
            Pickup->SetPickupItem(Drop.Item);
            UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
        }
    }
}
