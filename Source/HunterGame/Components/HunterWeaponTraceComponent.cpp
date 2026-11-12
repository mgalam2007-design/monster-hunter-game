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
}

void UHunterWeaponTraceComponent::BeginWeaponTrace()
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    USkeletalMeshComponent* Mesh = Character ? Character->GetMesh() : nullptr;
    if (!Mesh || !Mesh->DoesSocketExist(BaseSocket) || !Mesh->DoesSocketExist(TipSocket))
    {
        UE_LOG(LogTemp, Warning, TEXT("Weapon trace requires valid '%s' and '%s' sockets on the character mesh."), *BaseSocket.ToString(), *TipSocket.ToString());
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
    BaseDamage = FMath::Max(1.0f, NewDamage);
}

void UHunterWeaponTraceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!bIsTracing) return;

    ACharacter* Character = Cast<ACharacter>(GetOwner());
    USkeletalMeshComponent* Mesh = Character ? Character->GetMesh() : nullptr;
    if (!Mesh) { EndWeaponTrace(); return; }

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
    if (!World || !OwnerActor) return;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(HunterWeaponTrace), false, OwnerActor);
    Params.AddIgnoredActor(OwnerActor);
    TArray<FHitResult> Hits;

    const FVector Samples[] = { PreviousBase, PreviousTip, CurrentBase, CurrentTip };
    for (int32 Index = 0; Index < 4; ++Index)
    {
        const FVector& Start = Samples[Index];
        const FVector& End = Samples[(Index + 1) % 4];
        World->LineTraceMultiByChannel(Hits, Start, End, TraceChannel, Params);
    }

    for (const FHitResult& Hit : Hits)
    {
        AActor* HitActor = Hit.GetActor();
        if (!IsValid(HitActor) || HitActors.Contains(HitActor)) continue;
        HitActors.Add(HitActor);

        const float Damage = BaseDamage;
        UGameplayStatics::ApplyPointDamage(HitActor, Damage, (CurrentTip - PreviousTip).GetSafeNormal(),
            Hit, Cast<ACharacter>(OwnerActor) ? Cast<ACharacter>(OwnerActor)->GetController() : nullptr,
            OwnerActor, UDamageType::StaticClass());
    }
}
