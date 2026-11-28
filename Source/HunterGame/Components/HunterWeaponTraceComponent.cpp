#include "HunterWeaponTraceComponent.h"

#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UHunterWeaponTraceComponent::UHunterWeaponTraceComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UHunterWeaponTraceComponent::BeginPlay()
{
    Super::BeginPlay();
    BaseDamage = FMath::IsFinite(BaseDamage) ? FMath::Max(1.0f, BaseDamage) : 35.0f;
    TraceRadius = FMath::IsFinite(TraceRadius) ? FMath::Clamp(TraceRadius, 0.0f, 40.0f) : 12.0f;
}

bool UHunterWeaponTraceComponent::GetBladePoints(FVector& OutBase, FVector& OutTip) const
{
    const ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        return false;
    }

    const USkeletalMeshComponent* Mesh = Character->GetMesh();
    if (Mesh && Mesh->DoesSocketExist(BaseSocket) && Mesh->DoesSocketExist(TipSocket))
    {
        OutBase = Mesh->GetSocketLocation(BaseSocket);
        OutTip = Mesh->GetSocketLocation(TipSocket);
        return true;
    }

    if (!bAllowProceduralFallback)
    {
        return false;
    }

    const FTransform ActorTransform = Character->GetActorTransform();
    OutBase = ActorTransform.TransformPosition(FallbackBaseOffset);
    OutTip = ActorTransform.TransformPosition(FallbackTipOffset);
    return true;
}

void UHunterWeaponTraceComponent::BeginWeaponTrace()
{
    FVector CurrentBase;
    FVector CurrentTip;
    if (!GetBladePoints(CurrentBase, CurrentTip))
    {
        UE_LOG(LogTemp, Warning, TEXT("Weapon trace needs '%s' and '%s' sockets or the procedural fallback enabled."),
            *BaseSocket.ToString(), *TipSocket.ToString());
        EndWeaponTrace();
        return;
    }

    PreviousBase = CurrentBase;
    PreviousTip = CurrentTip;
    HitActors.Reset();
    bIsTracing = true;
    SetComponentTickEnabled(true);
}

void UHunterWeaponTraceComponent::EndWeaponTrace()
{
    bIsTracing = false;
    SetComponentTickEnabled(false);
    HitActors.Reset();
}

void UHunterWeaponTraceComponent::SetBaseDamage(float NewDamage)
{
    BaseDamage = FMath::IsFinite(NewDamage) ? FMath::Max(1.0f, NewDamage) : 35.0f;
}

void UHunterWeaponTraceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!bIsTracing)
    {
        return;
    }

    FVector CurrentBase;
    FVector CurrentTip;
    if (!GetBladePoints(CurrentBase, CurrentTip))
    {
        EndWeaponTrace();
        return;
    }

    TraceWeaponSegment(CurrentBase, CurrentTip);
    PreviousBase = CurrentBase;
    PreviousTip = CurrentTip;
}

void UHunterWeaponTraceComponent::TraceWeaponSegment(const FVector& CurrentBase, const FVector& CurrentTip)
{
    UWorld* World = GetWorld();
    AActor* OwnerActor = GetOwner();
    if (!World || !IsValid(OwnerActor))
    {
        return;
    }

    FCollisionQueryParams Params(SCENE_QUERY_STAT(HunterWeaponTrace), false, OwnerActor);
    Params.AddIgnoredActor(OwnerActor);

    const FCollisionShape BladeShape = FCollisionShape::MakeSphere(TraceRadius);

    // Sweep intermediate points along the blade as well as its edges. Tracing
    // only the perimeter can miss a target crossing the interior of a fast,
    // wide swing between frames.
    const FVector PreviousQuarter = FMath::Lerp(PreviousBase, PreviousTip, 0.25f);
    const FVector PreviousMidpoint = FMath::Lerp(PreviousBase, PreviousTip, 0.50f);
    const FVector PreviousThreeQuarter = FMath::Lerp(PreviousBase, PreviousTip, 0.75f);
    const FVector CurrentQuarter = FMath::Lerp(CurrentBase, CurrentTip, 0.25f);
    const FVector CurrentMidpoint = FMath::Lerp(CurrentBase, CurrentTip, 0.50f);
    const FVector CurrentThreeQuarter = FMath::Lerp(CurrentBase, CurrentTip, 0.75f);
    const FVector SweepSegments[][2] =
    {
        { PreviousBase, CurrentBase },
        { PreviousQuarter, CurrentQuarter },
        { PreviousMidpoint, CurrentMidpoint },
        { PreviousThreeQuarter, CurrentThreeQuarter },
        { PreviousTip, CurrentTip },
        { PreviousBase, PreviousTip },
        { CurrentBase, CurrentTip }
    };

    const FVector BladeMotion = (CurrentTip - PreviousTip).GetSafeNormal();
    const FVector FallbackDirection = OwnerActor->GetActorForwardVector();
    const FVector DamageDirection = BladeMotion.IsNearlyZero() ? FallbackDirection : BladeMotion;

    for (const auto& Segment : SweepSegments)
    {
        TArray<FHitResult> SegmentHits;
        World->SweepMultiByChannel(
            SegmentHits,
            Segment[0],
            Segment[1],
            FQuat::Identity,
            TraceChannel,
            BladeShape,
            Params);

        for (const FHitResult& Hit : SegmentHits)
        {
            AActor* HitActor = Hit.GetActor();
            if (!IsValid(HitActor) || HitActors.Contains(HitActor))
            {
                continue;
            }

            HitActors.Add(HitActor);
            ACharacter* AttackingCharacter = Cast<ACharacter>(OwnerActor);
            UGameplayStatics::ApplyPointDamage(
                HitActor,
                BaseDamage,
                DamageDirection,
                Hit,
                AttackingCharacter ? AttackingCharacter->GetController() : nullptr,
                OwnerActor,
                UDamageType::StaticClass());
        }
    }
}
