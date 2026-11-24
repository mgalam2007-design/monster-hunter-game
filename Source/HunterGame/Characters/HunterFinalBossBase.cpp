#include "HunterFinalBossBase.h"

#include "ArinValeCharacter.h"
#include "Components/HunterHealthComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"

AHunterFinalBossBase::AHunterFinalBossBase()
{
    MaxHealth = 2600.0f;
    AttackDamage = 42.0f;
    AttackCooldown = 1.45f;
    ChaseSpeed = 365.0f;
    ExperienceReward = 1200;
    CurrencyReward = 500;
    OriginPulseRadius = 650.0f;
    OriginPulseDamage = 48.0f;
    OriginPulseWindup = 1.35f;
    OriginPulseCooldown = 8.0f;
}

void AHunterFinalBossBase::BeginPlay()
{
    Super::BeginPlay();

    OriginPulseRadius = FMath::IsFinite(OriginPulseRadius)
        ? FMath::Clamp(OriginPulseRadius, 100.0f, 3000.0f) : 650.0f;
    OriginPulseDamage = FMath::IsFinite(OriginPulseDamage)
        ? FMath::Clamp(OriginPulseDamage, 1.0f, 500.0f) : 48.0f;
    OriginPulseWindup = FMath::IsFinite(OriginPulseWindup)
        ? FMath::Clamp(OriginPulseWindup, 0.5f, 5.0f) : 1.35f;
    OriginPulseCooldown = FMath::IsFinite(OriginPulseCooldown)
        ? FMath::Clamp(OriginPulseCooldown, 2.0f, 30.0f) : 8.0f;

    GetWorldTimerManager().SetTimer(
        OriginPulseCooldownHandle,
        this,
        &AHunterFinalBossBase::BeginOriginPulse,
        4.0f,
        false);
}

void AHunterFinalBossBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(OriginPulseCooldownHandle);
    GetWorldTimerManager().ClearTimer(OriginPulseImpactHandle);
    Super::EndPlay(EndPlayReason);
}

void AHunterFinalBossBase::BeginOriginPulse()
{
    if (IsMonsterDead() || !GetWorld())
    {
        return;
    }

    const int32 Phase = FMath::Clamp(GetCurrentBossPhase(), 0, 3);
    const float SafeWindup = OriginPulseWindup * FMath::Max(0.55f, 1.0f - 0.10f * Phase);
    BP_OnOriginPulseTelegraph(OriginPulseRadius, SafeWindup);
    GetWorldTimerManager().SetTimer(
        OriginPulseImpactHandle,
        this,
        &AHunterFinalBossBase::ResolveOriginPulse,
        SafeWindup,
        false);
}

void AHunterFinalBossBase::ResolveOriginPulse()
{
    if (IsMonsterDead() || !GetWorld())
    {
        return;
    }

    BP_OnOriginPulseImpact(OriginPulseRadius);

    AArinValeCharacter* Arin = Cast<AArinValeCharacter>(
        UGameplayStatics::GetPlayerCharacter(this, 0));
    if (Arin && !Arin->IsActorBeingDestroyed() &&
        FVector::DistSquared(GetActorLocation(), Arin->GetActorLocation()) <= FMath::Square(OriginPulseRadius))
    {
        const UHunterHealthComponent* Health = Arin->GetHealthComponent();
        if (!Health || !Health->IsDead())
        {
            const int32 Phase = FMath::Clamp(GetCurrentBossPhase(), 0, 3);
            const float Damage = OriginPulseDamage * (1.0f + 0.20f * Phase);
            UGameplayStatics::ApplyDamage(Arin, Damage, GetController(), this, nullptr);
        }
    }

    ScheduleNextOriginPulse();
}

void AHunterFinalBossBase::ScheduleNextOriginPulse()
{
    if (IsMonsterDead() || !GetWorld())
    {
        return;
    }

    const int32 Phase = FMath::Clamp(GetCurrentBossPhase(), 0, 3);
    const float Cooldown = OriginPulseCooldown * FMath::Max(0.45f, 1.0f - 0.15f * Phase);
    GetWorldTimerManager().SetTimer(
        OriginPulseCooldownHandle,
        this,
        &AHunterFinalBossBase::BeginOriginPulse,
        Cooldown,
        false);
}
