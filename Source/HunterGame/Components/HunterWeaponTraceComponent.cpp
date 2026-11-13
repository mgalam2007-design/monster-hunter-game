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

void UHunterWeaponTraceComponent::BeginWeaponTrace()
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    USkeletalMeshComponent* Mesh = Character ? Character->GetMesh() : nullptr;
    if (!Mesh || !Mesh->DoesSocketExist(BaseSocket) || !Mesh->DoesSocketExist(TipSocket))
    {
        UE_LOG(LogTemp, Warning, TEXT("Weapon trace requires valid '%s' and '%s' sockets on the character mesh."), *BaseSocket.ToString(), *TipSocket.ToString());
        EndWeaponTrace();
        return;
    }

    PreviousBase = Mesh->GetSocketLocation(BaseSocket);
    PreviousTip = Mesh->GetSocketLocation(TipSocket);
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
    if (!bIsTracing) return;

    ACharacter* Character = Cast<ACharacter>(GetOwner());
    USkeletalMeshComponent* Mesh = Character ? Character->GetMesh() : nullptr;
    if (!Mesh || !Mesh->DoesSocketExist(BaseSocket) || !Mesh->DoesSocketExist(TipSocket))
    {
        EndWeaponTrace();
        return;
    }

    const FVector CurrentBase = Mesh->GetSocketLocation(BaseSocket);
    const FVector CurrentTip = Mesh->GetSocketLocation(TipSocket);
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
    const FVector SweepSegments[][2] =
    {
        { PreviousBase, CurrentBase },
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
