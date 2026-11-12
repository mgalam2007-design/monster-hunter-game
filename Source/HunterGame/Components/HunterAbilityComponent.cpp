#include "HunterAbilityComponent.h"

#include "ArinProgressionComponent.h"
#include "ArinValeCharacter.h"
#include "Characters/HunterMonsterBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"

UHunterAbilityComponent::UHunterAbilityComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UHunterAbilityComponent::BeginPlay()
{
    Super::BeginPlay();
    MaxEnergy = FMath::Max(0.0f, MaxEnergy);
    CurrentEnergy = FMath::Clamp(CurrentEnergy, 0.0f, MaxEnergy);
    OnEnergyChanged.Broadcast(CurrentEnergy, MaxEnergy);
}

float UHunterAbilityComponent::RestoreEnergy(float Amount)
{
    if (Amount <= 0.0f || MaxEnergy <= 0.0f) return 0.0f;
    const float PreviousEnergy = CurrentEnergy;
    CurrentEnergy = FMath::Clamp(CurrentEnergy + Amount, 0.0f, MaxEnergy);
    const float Restored = CurrentEnergy - PreviousEnergy;
    if (Restored > 0.0f) OnEnergyChanged.Broadcast(CurrentEnergy, MaxEnergy);
    return Restored;
}

void UHunterAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    for (auto It = CooldownRemaining.CreateIterator(); It; ++It)
    {
        It.Value() = FMath::Max(0.0f, It.Value() - DeltaTime);
    }

    const float PreviousEnergy = CurrentEnergy;
    CurrentEnergy = FMath::Clamp(CurrentEnergy + EnergyRegenerationPerSecond * DeltaTime, 0.0f, MaxEnergy);
    if (!FMath::IsNearlyEqual(PreviousEnergy, CurrentEnergy))
    {
        OnEnergyChanged.Broadcast(CurrentEnergy, MaxEnergy);
    }
}

bool UHunterAbilityComponent::TryActivateAbility(EHunterAbility Ability)
{
    if (!IsAbilityUnlocked(Ability) || GetCooldownRemaining(Ability) > 0.0f)
    {
        return false;
    }

    const float Cost = GetAbilityCost(Ability);
    if (CurrentEnergy < Cost)
    {
        return false;
    }

    if (!ExecuteAbility(Ability))
    {
        return false;
    }

    CurrentEnergy -= Cost;
    CooldownRemaining.Add(Ability, GetAbilityCooldown(Ability));
    OnEnergyChanged.Broadcast(CurrentEnergy, MaxEnergy);
    OnAbilityActivated.Broadcast(Ability);
    BP_OnAbilityActivated(Ability);
    return true;
}

float UHunterAbilityComponent::GetCooldownRemaining(EHunterAbility Ability) const
{
    const float* Remaining = CooldownRemaining.Find(Ability);
    return Remaining ? FMath::Max(0.0f, *Remaining) : 0.0f;
}

bool UHunterAbilityComponent::IsAbilityUnlocked(EHunterAbility Ability) const
{
    const AArinValeCharacter* Arin = Cast<AArinValeCharacter>(GetOwner());
    const UArinProgressionComponent* Progression = Arin ? Arin->GetProgressionComponent() : nullptr;
    if (!Progression)
    {
        return false;
    }

    switch (Ability)
    {
        case EHunterAbility::EnergyStrike: return Progression->HasUnlockedEnergyStrike();
        case EHunterAbility::ShadowStep: return Progression->HasUnlockedShadowStep();
        case EHunterAbility::ShadowEcho: return Progression->HasUnlockedShadowEcho();
        default: return false;
    }
}

float UHunterAbilityComponent::GetAbilityCost(EHunterAbility Ability) const
{
    switch (Ability)
    {
        case EHunterAbility::EnergyStrike: return 25.0f;
        case EHunterAbility::ShadowStep: return 35.0f;
        case EHunterAbility::ShadowEcho: return 60.0f;
        default: return MaxEnergy;
    }
}

float UHunterAbilityComponent::GetAbilityCooldown(EHunterAbility Ability) const
{
    switch (Ability)
    {
        case EHunterAbility::EnergyStrike: return 4.0f;
        case EHunterAbility::ShadowStep: return 7.0f;
        case EHunterAbility::ShadowEcho: return 25.0f;
        default: return 1.0f;
    }
}

bool UHunterAbilityComponent::ExecuteAbility(EHunterAbility Ability)
{
    AArinValeCharacter* Arin = Cast<AArinValeCharacter>(GetOwner());
    if (!Arin || !GetWorld())
    {
        return false;
    }

    switch (Ability)
    {
        case EHunterAbility::EnergyStrike:
        {
            const FVector Origin = Arin->GetActorLocation() + Arin->GetActorForwardVector() * 150.0f;
            const FCollisionShape Shape = FCollisionShape::MakeSphere(FMath::Max(0.0f, EnergyStrikeRadius));
            FCollisionQueryParams Params(SCENE_QUERY_STAT(EnergyStrike), false, Arin);
            TArray<FOverlapResult> Overlaps;
            GetWorld()->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity,
                FCollisionObjectQueryParams(ECC_Pawn), Shape, Params);

            TSet<AActor*> DamagedActors;
            for (const FOverlapResult& Overlap : Overlaps)
            {
                AActor* Target = Overlap.GetActor();
                if (!Target || Target == Arin || !Target->IsA<AHunterMonsterBase>() || DamagedActors.Contains(Target))
                {
                    continue;
                }

                DamagedActors.Add(Target);
                UGameplayStatics::ApplyDamage(Target, EnergyStrikeDamage, Arin->GetController(), Arin, UDamageType::StaticClass());
            }
            return true;
        }

        case EHunterAbility::ShadowStep:
        {
            const FVector Direction = Arin->GetActorForwardVector();
            Arin->LaunchCharacter(Direction * ShadowStepStrength, true, false);
            return true;
        }

        case EHunterAbility::ShadowEcho:
        {
            if (!ShadowEchoActorClass)
            {
                return false;
            }

            const FVector SpawnLocation = Arin->GetActorLocation() - Arin->GetActorForwardVector() * ShadowEchoSpawnDistance;
            const FTransform SpawnTransform(Arin->GetActorRotation(), SpawnLocation);
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = Arin;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
            return GetWorld()->SpawnActor<AActor>(ShadowEchoActorClass, SpawnTransform, SpawnParams) != nullptr;
        }

        default:
            return false;
    }
}
