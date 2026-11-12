#include "HunterMonsterBase.h"
#include "Items/HunterPickupActor.h"

#include "ArinValeCharacter.h"
#include "ArinProgressionComponent.h"
#include "Quests/HunterQuestSubsystem.h"
#include "Engine/GameInstance.h"

#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AHunterMonsterBase::AHunterMonsterBase()
{
    PrimaryActorTick.bCanEverTick = true;
    GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
}

void AHunterMonsterBase::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = FMath::Max(1.0f, MaxHealth);
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
        if (bCanChasePlayer)
        {
            if (AAIController* AI = Cast<AAIController>(GetController()))
            {
                AI->MoveToActor(Target, AttackRange * 0.8f);
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
    const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    ReceiveHunterDamage(AppliedDamage, DamageCauser);
    return AppliedDamage;
}

void AHunterMonsterBase::ReceiveHunterDamage(float DamageAmount, AActor* DamageSource)
{
    if (bIsDead || DamageAmount <= 0.0f)
    {
        return;
    }

    CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

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
    return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
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
