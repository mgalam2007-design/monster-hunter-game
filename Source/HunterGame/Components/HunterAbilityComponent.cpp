#include "HunterAbilityComponent.h"

#include "ArinProgressionComponent.h"
#include "ArinValeCharacter.h"

UHunterAbilityComponent::UHunterAbilityComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
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
