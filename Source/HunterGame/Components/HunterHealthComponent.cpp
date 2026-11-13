#include "HunterHealthComponent.h"

UHunterHealthComponent::UHunterHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHunterHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    MaxHealth = FMath::IsFinite(MaxHealth) ? FMath::Max(1.0f, MaxHealth) : 100.0f;
    CurrentHealth = MaxHealth;
    bIsDead = false;
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

float UHunterHealthComponent::ApplyDamage(float Amount)
{
    if (bIsDead || !FMath::IsFinite(Amount) || Amount <= 0.0f)
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
    if (bIsDead || !FMath::IsFinite(Amount) || Amount <= 0.0f)
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

void UHunterHealthComponent::ReviveFullHealth()
{
    bIsDead = false;
    CurrentHealth = MaxHealth;
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
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
    return FMath::IsFinite(MaxHealth) && MaxHealth > 0.0f
        ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f)
        : 0.0f;
}

void UHunterHealthComponent::SetMaxHealth(float NewMaxHealth, bool bPreserveHealthPercentage)
{
    if (!FMath::IsFinite(NewMaxHealth))
    {
        return;
    }

    const float PreviousMax = FMath::IsFinite(MaxHealth) ? FMath::Max(1.0f, MaxHealth) : 100.0f;
    const float HealthRatio = FMath::IsFinite(CurrentHealth)
        ? FMath::Clamp(CurrentHealth / PreviousMax, 0.0f, 1.0f)
        : 1.0f;
    MaxHealth = FMath::Max(1.0f, NewMaxHealth);
    CurrentHealth = bPreserveHealthPercentage
        ? FMath::Clamp(HealthRatio * MaxHealth, 0.0f, MaxHealth)
        : FMath::Clamp(FMath::IsFinite(CurrentHealth) ? CurrentHealth : MaxHealth, 0.0f, MaxHealth);
    if (CurrentHealth > 0.0f)
    {
        bIsDead = false;
    }
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}
