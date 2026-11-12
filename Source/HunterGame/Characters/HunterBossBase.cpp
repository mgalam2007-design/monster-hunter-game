#include "HunterBossBase.h"

AHunterBossBase::AHunterBossBase()
{
    PhaseHealthThresholds = { 0.70f, 0.40f, 0.15f };
}

float AHunterBossBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    UpdateBossPhase();
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
    if (BaseAttackDamage <= 0.0f)
    {
        BaseAttackDamage = AttackDamage;
    }
    AttackDamage = BaseAttackDamage * (1.0f + AttackDamageMultiplierPerPhase * CurrentBossPhase);

    OnBossPhaseChanged.Broadcast(CurrentBossPhase, HealthPercent);
    BP_OnBossPhaseChanged(CurrentBossPhase, HealthPercent);
}
