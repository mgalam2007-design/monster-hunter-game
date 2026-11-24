#include "HunterShadowEchoActor.h"

#include "ArinValeCharacter.h"
#include "ArinProgressionComponent.h"
#include "Characters/HunterMonsterBase.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AHunterShadowEchoActor::AHunterShadowEchoActor()
{
    PrimaryActorTick.bCanEverTick = false;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    EchoVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EchoVisual"));
    EchoVisual->SetupAttachment(SceneRoot);
    EchoVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    EchoVisual->SetGenerateOverlapEvents(false);
    EchoVisual->SetCanEverAffectNavigation(false);
    EchoVisual->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.8f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        EchoVisual->SetStaticMesh(SphereMesh.Object);
    }
}

void AHunterShadowEchoActor::BeginPlay()
{
    Super::BeginPlay();

    PulseRadius = FMath::IsFinite(PulseRadius) ? FMath::Clamp(PulseRadius, 0.0f, 3000.0f) : 360.0f;
    PulseDamage = FMath::IsFinite(PulseDamage) ? FMath::Clamp(PulseDamage, 0.0f, 10000.0f) : 22.0f;
    PulseInterval = FMath::IsFinite(PulseInterval) ? FMath::Clamp(PulseInterval, 0.05f, 30.0f) : 0.8f;
    EchoLifetime = FMath::IsFinite(EchoLifetime) ? FMath::Clamp(EchoLifetime, 0.1f, 60.0f) : 6.0f;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(PulseTimerHandle, this,
            &AHunterShadowEchoActor::PulseNearbyMonsters, PulseInterval, true, 0.0f);
        World->GetTimerManager().SetTimer(LifetimeTimerHandle, this,
            &AHunterShadowEchoActor::ExpireEcho, EchoLifetime, false);
    }
}

void AHunterShadowEchoActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    bEnding = true;
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PulseTimerHandle);
        World->GetTimerManager().ClearTimer(LifetimeTimerHandle);
    }
    Super::EndPlay(EndPlayReason);
}

void AHunterShadowEchoActor::PulseNearbyMonsters()
{
    if (bEnding || PulseDamage <= 0.0f || PulseRadius <= 0.0f || !GetWorld())
    {
        return;
    }

    AArinValeCharacter* Arin = Cast<AArinValeCharacter>(GetOwner());
    if (!Arin || Arin->IsActorBeingDestroyed())
    {
        ExpireEcho();
        return;
    }

    const UArinProgressionComponent* Progression = Arin->GetProgressionComponent();
    const FArinStageProfile Profile = Progression
        ? Progression->GetCurrentStageProfile() : FArinStageProfile();
    const float DamageMultiplier = FMath::IsFinite(Profile.DamageMultiplier)
        ? FMath::Clamp(Profile.DamageMultiplier, 0.5f, 3.0f) : 1.0f;
    const float Damage = PulseDamage * DamageMultiplier;

    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(ShadowEchoPulse), false, this);
    Params.AddIgnoredActor(Arin);
    GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity,
        FCollisionObjectQueryParams(ECC_Pawn),
        FCollisionShape::MakeSphere(PulseRadius), Params);

    TSet<AHunterMonsterBase*> HitThisPulse;
    for (const FOverlapResult& Overlap : Overlaps)
    {
        AHunterMonsterBase* Monster = Cast<AHunterMonsterBase>(Overlap.GetActor());
        if (!Monster || Monster->IsMonsterDead() || HitThisPulse.Contains(Monster))
        {
            continue;
        }

        HitThisPulse.Add(Monster);
        UGameplayStatics::ApplyDamage(Monster, Damage, Arin->GetController(),
            this, UDamageType::StaticClass());
    }
}

void AHunterShadowEchoActor::ExpireEcho()
{
    if (bEnding)
    {
        return;
    }

    bEnding = true;
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PulseTimerHandle);
        World->GetTimerManager().ClearTimer(LifetimeTimerHandle);
    }
    Destroy();
}
