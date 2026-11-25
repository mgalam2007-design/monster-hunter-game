#include "HunterMonsterBase.h"
#include "AI/HunterLearningSubsystem.h"
#include "AI/HunterPlayerProfileSubsystem.h"
#include "Items/HunterPickupActor.h"

#include "ArinValeCharacter.h"
#include "ArinProgressionComponent.h"
#include "Components/HunterCurrencyComponent.h"
#include "Components/HunterHealthComponent.h"
#include "Quests/HunterQuestSubsystem.h"
#include "Engine/GameInstance.h"

#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/TextRenderComponent.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AHunterMonsterBase::AHunterMonsterBase()
{
    PrimaryActorTick.bCanEverTick = true;
    // Spawned wave enemies need an AI controller to execute MoveToActor pursuit.
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AAIController::StaticClass();
    GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

    // Native warning marker makes the melee windup readable without a Blueprint asset.
    AttackWarningVisual = CreateDefaultSubobject<UTextRenderComponent>(TEXT("MonsterAttackWarning"));
    AttackWarningVisual->SetupAttachment(GetRootComponent());
    AttackWarningVisual->SetText(FText::FromString(TEXT("!")));
    AttackWarningVisual->SetWorldSize(76.0f);
    AttackWarningVisual->SetTextRenderColor(FColor(255, 170, 35));
    AttackWarningVisual->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    AttackWarningVisual->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    AttackWarningVisual->SetRelativeLocation(FVector(0.0f, 0.0f, 205.0f));
    AttackWarningVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AttackWarningVisual->SetCanEverAffectNavigation(false);
    AttackWarningVisual->SetHiddenInGame(true);

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
    AttackWindupDuration = FMath::IsFinite(AttackWindupDuration)
        ? FMath::Clamp(AttackWindupDuration, 0.0f, 2.0f) : 0.45f;
    ChaseSpeed = FMath::IsFinite(ChaseSpeed) ? FMath::Max(0.0f, ChaseSpeed) : 430.0f;
    ChaseRepathInterval = FMath::IsFinite(ChaseRepathInterval) ? FMath::Max(0.05f, ChaseRepathInterval) : 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
    FindTarget();
}

void AHunterMonsterBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bIsDead || !FMath::IsFinite(DeltaSeconds) || DeltaSeconds <= 0.0f)
    {
        return;
    }

    AttackTimer = FMath::Max(0.0f, AttackTimer - DeltaSeconds);
    AttackWindupTimer = FMath::Max(0.0f, AttackWindupTimer - DeltaSeconds);
    ChaseRepathTimer = FMath::Max(0.0f, ChaseRepathTimer - DeltaSeconds);

    // Pulse the warning marker and intensify its color as the strike approaches.
    if (AttackWarningVisual && bAttackPending)
    {
        const float WindupRatio = AttackWindupDuration > KINDA_SMALL_NUMBER
            ? FMath::Clamp(AttackWindupTimer / AttackWindupDuration, 0.0f, 1.0f) : 0.0f;
        const float Pulse = 0.5f + 0.5f * FMath::Sin(GetWorld()->GetTimeSeconds() * 12.0f);
        AttackWarningVisual->SetWorldSize(72.0f + Pulse * 18.0f);
        const uint8 GreenChannel = static_cast<uint8>(FMath::Lerp(70.0f, 175.0f, 1.0f - WindupRatio));
        AttackWarningVisual->SetTextRenderColor(FColor(255, GreenChannel, 25));
    }

    if (!TargetCharacter.IsValid())
    {
        FindTarget();
    }

    UpdateCombat(DeltaSeconds);
}

void AHunterMonsterBase::FindTarget()
{
    AArinValeCharacter* PlayerCharacter = Cast<AArinValeCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    const UHunterHealthComponent* PlayerHealth = PlayerCharacter ? PlayerCharacter->GetHealthComponent() : nullptr;
    if (PlayerCharacter && (!PlayerHealth || !PlayerHealth->IsDead()) &&
        FVector::DistSquared(GetActorLocation(), PlayerCharacter->GetActorLocation()) <= FMath::Square(DetectionRadius))
    {
        TargetCharacter = PlayerCharacter;
    }
}

void AHunterMonsterBase::UpdateCombat(float DeltaSeconds)
{
    ACharacter* Target = TargetCharacter.Get();
    const AArinValeCharacter* ArinTarget = Cast<AArinValeCharacter>(Target);
    const UHunterHealthComponent* TargetHealth = ArinTarget ? ArinTarget->GetHealthComponent() : nullptr;
    if (!Target || Target->IsActorBeingDestroyed() || (TargetHealth && TargetHealth->IsDead()))
    {
        TargetCharacter.Reset();
        if (AttackWarningVisual) AttackWarningVisual->SetHiddenInGame(true);
        bAttackPending = false;
        PendingAttackTarget.Reset();
        AttackWindupTimer = 0.0f;
        return;
    }

    const float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (Distance > DetectionRadius * 1.35f)
    {
        bAttackPending = false;
        PendingAttackTarget.Reset();
        AttackWindupTimer = 0.0f;
        if (AttackWarningVisual) AttackWarningVisual->SetHiddenInGame(true);
        TargetCharacter.Reset();
        if (AAIController* AI = Cast<AAIController>(GetController()))
        {
            AI->StopMovement();
        }
        return;
    }

    if (bAttackPending)
    {
        if (!PendingAttackTarget.IsValid() || PendingAttackTarget.Get() != Target || Distance > AttackRange)
        {
            bAttackPending = false;
            PendingAttackTarget.Reset();
            AttackWindupTimer = 0.0f;
            if (AttackWarningVisual) AttackWarningVisual->SetHiddenInGame(true);
            AttackTimer = FMath::Min(AttackTimer, 0.2f);
        }
        else if (AttackWindupTimer <= 0.0f)
        {
            bAttackPending = false;
            PendingAttackTarget.Reset();
            const float CooldownScale = LearningAction == EHunterCombatLearningAction::Aggressive ? 0.85f
                : (LearningAction == EHunterCombatLearningAction::Cautious ? 1.20f : 1.0f);
            const float DamageScale = LearningAction == EHunterCombatLearningAction::Aggressive ? 1.20f
                : (LearningAction == EHunterCombatLearningAction::Cautious ? 0.85f : 1.0f);
            AttackTimer = FMath::Max(0.1f, AttackCooldown * CooldownScale);
            if (AttackWarningVisual) AttackWarningVisual->SetHiddenInGame(true);
            BP_OnMonsterAttack(Target);
            float AppliedDamage = 0.0f;
            if (FMath::IsFinite(AttackDamage) && AttackDamage > 0.0f)
            {
                AppliedDamage = UGameplayStatics::ApplyDamage(
                    Target, AttackDamage * DamageScale, GetController(), this, nullptr);
            }

            if (UGameInstance* GameInstance = GetGameInstance())
            {
                if (UHunterLearningSubsystem* Learning = GameInstance->GetSubsystem<UHunterLearningSubsystem>())
                {
                    Learning->RecordCombatOutcome(LearningState, LearningAction,
                        AppliedDamage > 0.0f ? 0.8f : -0.2f, LearningState);
                }
            }
        }
        else
        {
            return;
        }
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

    if (AttackTimer <= 0.0f && !bAttackPending)
    {
        LearningState = EHunterCombatLearningState::PlayerInRange;
        if (ArinTarget && TargetHealth && TargetHealth->GetHealthPercent() <= 0.30f)
        {
            LearningState = EHunterCombatLearningState::PlayerLowHealth;
        }
        else if (Distance > AttackRange * 0.75f)
        {
            const FVector ToMonster = (GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
            const float MovementIntent = FVector::DotProduct(Target->GetVelocity().GetSafeNormal(), ToMonster);
            LearningState = MovementIntent > 0.2f
                ? EHunterCombatLearningState::PlayerApproaching
                : EHunterCombatLearningState::PlayerRetreating;
        }

        LearningAction = EHunterCombatLearningAction::Balanced;
        if (UGameInstance* GameInstance = GetGameInstance())
        {
            if (UHunterLearningSubsystem* Learning = GameInstance->GetSubsystem<UHunterLearningSubsystem>())
            {
                LearningAction = Learning->ChooseCombatAction(LearningState);
            }

            // Online k-means identifies the player's broad combat style and helps
            // the learned policy choose a counter-tactic without external services.
            if (ArinTarget)
            {
                if (UHunterPlayerProfileSubsystem* Profile = GameInstance->GetSubsystem<UHunterPlayerProfileSubsystem>())
                {
                    const EHunterPlayerArchetype Archetype =
                        Profile->ObservePlayer(const_cast<AArinValeCharacter*>(ArinTarget), GetActorLocation());
                    if (LearningAction == EHunterCombatLearningAction::Balanced)
                    {
                        if (Archetype == EHunterPlayerArchetype::Aggressive ||
                            Archetype == EHunterPlayerArchetype::Evasive)
                        {
                            LearningAction = EHunterCombatLearningAction::Cautious;
                        }
                        else if (Archetype == EHunterPlayerArchetype::Defensive)
                        {
                            LearningAction = EHunterCombatLearningAction::Aggressive;
                        }
                    }
                }
            }
        }

        bAttackPending = true;
        PendingAttackTarget = Target;
        AttackWindupTimer = AttackWindupDuration;
        if (LearningAction == EHunterCombatLearningAction::Aggressive)
        {
            AttackWindupTimer *= 0.85f;
        }
        else if (LearningAction == EHunterCombatLearningAction::Cautious)
        {
            AttackWindupTimer *= 1.20f;
        }
        if (AttackWarningVisual) AttackWarningVisual->SetHiddenInGame(false);
        BP_OnMonsterAttackTelegraph(Target);
        if (AttackWindupTimer <= 0.0f)
        {
            AttackWindupTimer = 0.0f;
        }
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

    BP_OnMonsterDamaged(DamageAmount, GetHealthPercent(), DamageSource);

    if (DamageSource && DamageSource != this)
    {
        if (UGameInstance* GameInstance = GetGameInstance())
        {
            if (UHunterLearningSubsystem* Learning = GameInstance->GetSubsystem<UHunterLearningSubsystem>())
            {
                Learning->RecordCombatOutcome(LearningState, LearningAction, -0.35f, LearningState);
            }
        }
    }

    // Lightweight native hit feedback remains visible even before a monster Blueprint exists.
    if (UWorld* World = GetWorld())
    {
        UTextRenderComponent* DamageText = NewObject<UTextRenderComponent>(this);
        if (DamageText)
        {
            DamageText->SetupAttachment(GetRootComponent());
            DamageText->SetText(FText::AsNumber(FMath::RoundToInt(DamageAmount)));
            DamageText->SetWorldSize(36.0f);
            DamageText->SetTextRenderColor(FColor::White);
            DamageText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
            DamageText->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
            DamageText->SetRelativeLocation(FVector(0.0f, 0.0f, 170.0f + FMath::FRandRange(0.0f, 24.0f)));
            DamageText->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            DamageText->SetCanEverAffectNavigation(false);
            DamageText->RegisterComponent();

            const TWeakObjectPtr<UTextRenderComponent> WeakDamageText(DamageText);
            FTimerHandle DamageTextTimer;
            World->GetTimerManager().SetTimer(
                DamageTextTimer,
                FTimerDelegate::CreateLambda([WeakDamageText]()
                {
                    if (UTextRenderComponent* Text = WeakDamageText.Get())
                    {
                        Text->DestroyComponent();
                    }
                }),
                0.85f,
                false);
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
    if (AttackWarningVisual) AttackWarningVisual->SetHiddenInGame(true);
    bAttackPending = false;
    PendingAttackTarget.Reset();
    AttackWindupTimer = 0.0f;
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
        // Reject invalid probabilities instead of letting NaN bypass the random-roll comparison.
        if (Drop.Item.ItemId.IsNone() || Drop.Item.Quantity <= 0 || !FMath::IsFinite(Drop.DropChance))
        {
            continue;
        }

        const float SafeDropChance = FMath::Clamp(Drop.DropChance, 0.0f, 1.0f);
        if (SafeDropChance <= 0.0f || (SafeDropChance < 1.0f && FMath::FRand() >= SafeDropChance))
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
