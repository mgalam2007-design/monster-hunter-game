#include "HunterHealthComponent.h"

UHunterHealthComponent::UHunterHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHunterHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    MaxHealth = FMath::Max(1.0f, MaxHealth);
    CurrentHealth = MaxHealth;
    bIsDead = false;
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

float UHunterHealthComponent::ApplyDamage(float Amount)
{
    if (bIsDead || Amount <= 0.0f)
    {
        return 0.0f;
    }

    const float PreviousHealth = CurrentHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.0f, MaxHealth);
    const float AppliedDamage = PreviousHealth - CurrentHealth;

    if (AppliedDamage > 0.0f)
    {
        OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
    }

    if (CurrentHealth <= 0.0f)
    {
        bIsDead = true;
        OnOwnerDied.Broadcast();
    }

    return AppliedDamage;
}

float UHunterHealthComponent::Heal(float Amount)
{
    if (bIsDead || Amount <= 0.0f)
    {
        return 0.0f;
    }

    const float PreviousHealth = CurrentHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);
    const float RestoredHealth = CurrentHealth - PreviousHealth;

    if (RestoredHealth > 0.0f)
    {
        OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
    }

    return RestoredHealth;
}

void UHunterHealthComponent::RestoreFullHealth()
{
    if (bIsDead)
    {
        return;
    }

    CurrentHealth = MaxHealth;
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

float UHunterHealthComponent::GetHealthPercent() const
{
    return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

void UHunterHealthComponent::SetMaxHealth(float NewMaxHealth, bool bPreserveHealthPercentage)
{
    const float PreviousMax = FMath::Max(1.0f, MaxHealth);
    const float HealthRatio = CurrentHealth / PreviousMax;
    MaxHealth = FMath::Max(1.0f, NewMaxHealth);
    CurrentHealth = bPreserveHealthPercentage
        ? FMath::Clamp(HealthRatio * MaxHealth, 0.0f, MaxHealth)
        : FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
    if (CurrentHealth > 0.0f)
    {
        bIsDead = false;
    }
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}
