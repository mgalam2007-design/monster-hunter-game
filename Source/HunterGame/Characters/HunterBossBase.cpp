#include "HunterBossBase.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
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
    BaseAttackDamage = AttackDamage;
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

    OnBossPhaseChanged.Broadcast(CurrentBossPhase, HealthPercent);
    BP_OnBossPhaseChanged(CurrentBossPhase, HealthPercent);
}
