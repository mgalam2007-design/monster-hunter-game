#include "HunterBossBase.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

AHunterBossBase::AHunterBossBase()
{
    MaxHealth = 1200.0f;
    AttackDamage = 35.0f;
    ChaseSpeed = 350.0f;
    ExperienceReward = 300;
    CurrencyReward = 100;
    PhaseHealthThresholds = { 0.70f, 0.40f, 0.15f };

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (CubeMesh.Succeeded() && SphereMesh.Succeeded())
    {
        auto AddBossPart = [this](const FName Name, UStaticMesh* Mesh, const FVector& Location, const FVector& Scale)
        {
            UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
            Part->SetupAttachment(GetRootComponent());
            Part->SetStaticMesh(Mesh);
            Part->SetRelativeLocation(Location);
            Part->SetRelativeScale3D(Scale);
            Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Part->SetCanEverAffectNavigation(false);
            BlockoutVisuals.Add(Part);
        };

        // Larger, crowned silhouette distinguishes the first boss from standard enemies.
        AddBossPart(TEXT("BossCrownCore"), SphereMesh.Object, FVector(0, 0, 155), FVector(0.72f, 0.72f, 0.78f));
        AddBossPart(TEXT("BossChestPlate"), CubeMesh.Object, FVector(0, 0, 35), FVector(1.05f, 0.82f, 1.05f));
        AddBossPart(TEXT("BossShoulderLeft"), CubeMesh.Object, FVector(0, -78, 72), FVector(0.78f, 0.72f, 0.72f));
        AddBossPart(TEXT("BossShoulderRight"), CubeMesh.Object, FVector(0, 78, 72), FVector(0.78f, 0.72f, 0.72f));
        AddBossPart(TEXT("BossCrownHornLeft"), CubeMesh.Object, FVector(0, -42, 222), FVector(0.18f, 0.18f, 0.72f));
        AddBossPart(TEXT("BossCrownHornRight"), CubeMesh.Object, FVector(0, 42, 222), FVector(0.18f, 0.18f, 0.72f));
        AddBossPart(TEXT("BossBackSpine"), CubeMesh.Object, FVector(-35, 0, 95), FVector(0.28f, 0.42f, 1.45f));
    }
}

void AHunterBossBase::BeginPlay()
{
    Super::BeginPlay();

    AttackDamageMultiplierPerPhase = FMath::IsFinite(AttackDamageMultiplierPerPhase)
        ? FMath::Max(0.0f, AttackDamageMultiplierPerPhase) : 0.0f;
    for (float& Threshold : PhaseHealthThresholds)
    {
        Threshold = FMath::IsFinite(Threshold) ? FMath::Clamp(Threshold, 0.0f, 1.0f) : 0.0f;
    }
    PhaseHealthThresholds.Sort([](const float A, const float B) { return A > B; });
    AttackCooldownReductionPerPhase = FMath::IsFinite(AttackCooldownReductionPerPhase)
        ? FMath::Clamp(AttackCooldownReductionPerPhase, 0.0f, 0.8f) : 0.12f;
    ChaseSpeedIncreasePerPhase = FMath::IsFinite(ChaseSpeedIncreasePerPhase)
        ? FMath::Clamp(ChaseSpeedIncreasePerPhase, 0.0f, 0.5f) : 0.15f;
    AttackWindupReductionPerPhase = FMath::IsFinite(AttackWindupReductionPerPhase)
        ? FMath::Clamp(AttackWindupReductionPerPhase, 0.0f, 0.2f) : 0.10f;
    AttackRangeIncreasePerPhase = FMath::IsFinite(AttackRangeIncreasePerPhase)
        ? FMath::Clamp(AttackRangeIncreasePerPhase, 0.0f, 0.5f) : 0.12f;
    BaseAttackDamage = AttackDamage;
    BaseAttackCooldown = AttackCooldown;
    BaseChaseSpeed = ChaseSpeed;
    BaseAttackWindupDuration = AttackWindupDuration;
    BaseAttackRange = AttackRange;
}

float AHunterBossBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (!IsMonsterDead())
    {
        UpdateBossPhase();
    }
    return AppliedDamage;
}

void AHunterBossBase::UpdateBossPhase()
{
    const float HealthPercent = GetHealthPercent();
    int32 NewPhase = 0;

    for (const float Threshold : PhaseHealthThresholds)
    {
        if (HealthPercent <= FMath::Clamp(Threshold, 0.0f, 1.0f))
        {
            ++NewPhase;
        }
    }

    if (NewPhase <= CurrentBossPhase)
    {
        return;
    }

    CurrentBossPhase = NewPhase;
    AttackDamage = BaseAttackDamage * (1.0f + AttackDamageMultiplierPerPhase * CurrentBossPhase);
    AttackCooldown = BaseAttackCooldown * FMath::Max(
        0.25f, 1.0f - AttackCooldownReductionPerPhase * CurrentBossPhase);
    ChaseSpeed = BaseChaseSpeed * (1.0f + ChaseSpeedIncreasePerPhase * CurrentBossPhase);
    AttackWindupDuration = BaseAttackWindupDuration * FMath::Max(
        0.4f, 1.0f - AttackWindupReductionPerPhase * CurrentBossPhase);
    // Later phases threaten a wider melee area, so Arin must dodge earlier and reposition.
    AttackRange = BaseAttackRange * (1.0f + AttackRangeIncreasePerPhase * CurrentBossPhase);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
    }

    OnBossPhaseChanged.Broadcast(CurrentBossPhase, HealthPercent);
    BP_OnBossPhaseChanged(CurrentBossPhase, HealthPercent);
}
