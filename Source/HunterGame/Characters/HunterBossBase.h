#pragma once

#include "CoreMinimal.h"
#include "HunterMonsterBase.h"
#include "HunterBossBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHunterBossPhaseChanged, int32, NewPhase, float, HealthPercent);

/**
 * Boss extension for multi-phase encounters. Configure presentation and special
 * attacks in a Blueprint child; the C++ class handles phase transitions.
 */
UCLASS()
class HUNTERGAME_API AHunterBossBase : public AHunterMonsterBase
{
    GENERATED_BODY()

public:
    AHunterBossBase();

    virtual void BeginPlay() override;

    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintPure, Category="Boss")
    int32 GetCurrentBossPhase() const { return CurrentBossPhase; }

    UPROPERTY(BlueprintAssignable, Category="Boss")
    FHunterBossPhaseChanged OnBossPhaseChanged;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss|Phases")
    TArray<float> PhaseHealthThresholds;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss|Phases")
    float AttackDamageMultiplierPerPhase = 0.25f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss|Phases", meta=(ClampMin="0.0", ClampMax="0.8"))
    float AttackCooldownReductionPerPhase = 0.12f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss|Phases", meta=(ClampMin="0.0", ClampMax="0.5"))
    float ChaseSpeedIncreasePerPhase = 0.15f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Boss|Phases", meta=(ClampMin="0.0", ClampMax="0.2"))
    float AttackWindupReductionPerPhase = 0.10f;

    UFUNCTION(BlueprintImplementableEvent, Category="Boss|Phases")
    void BP_OnBossPhaseChanged(int32 NewPhase, float HealthPercent);

private:
    int32 CurrentBossPhase = 0;
    float BaseAttackDamage = 0.0f;
    float BaseAttackCooldown = 0.0f;
    float BaseChaseSpeed = 0.0f;
    float BaseAttackWindupDuration = 0.0f;

    void UpdateBossPhase();
};
