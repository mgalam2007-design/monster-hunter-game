#pragma once

#include "CoreMinimal.h"
#include "HunterBossBase.h"
#include "HunterFinalBossBase.generated.h"

/**
 * Unique Chapter 20 boss. Adds a telegraphed Origin Pulse to the shared
 * multi-phase boss foundation; Blueprint events provide presentation hooks.
 */
UCLASS()
class HUNTERGAME_API AHunterFinalBossBase : public AHunterBossBase
{
    GENERATED_BODY()

public:
    AHunterFinalBossBase();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Final Boss|Origin Pulse", meta=(ClampMin="100.0", ClampMax="3000.0"))
    float OriginPulseRadius = 650.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Final Boss|Origin Pulse", meta=(ClampMin="1.0", ClampMax="500.0"))
    float OriginPulseDamage = 48.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Final Boss|Origin Pulse", meta=(ClampMin="0.5", ClampMax="5.0"))
    float OriginPulseWindup = 1.35f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Final Boss|Origin Pulse", meta=(ClampMin="2.0", ClampMax="30.0"))
    float OriginPulseCooldown = 8.0f;

    UFUNCTION(BlueprintImplementableEvent, Category="Final Boss|Origin Pulse")
    void BP_OnOriginPulseTelegraph(float Radius, float WindupSeconds);

    UFUNCTION(BlueprintImplementableEvent, Category="Final Boss|Origin Pulse")
    void BP_OnOriginPulseImpact(float Radius);

    UFUNCTION(BlueprintImplementableEvent, Category="Final Boss|Origin Pulse")
    void BP_OnOriginAftershockTelegraph(float Radius, float WindupSeconds);

    UFUNCTION(BlueprintImplementableEvent, Category="Final Boss|Origin Pulse")
    void BP_OnOriginAftershockImpact(float Radius);

private:
    FTimerHandle OriginPulseCooldownHandle;
    FTimerHandle OriginPulseImpactHandle;
    FTimerHandle OriginAftershockHandle;

    void BeginOriginPulse();
    void ResolveOriginPulse();
    void ResolveOriginAftershock();
    void ApplyPulseDamage(float Radius, float BaseDamage);
    void ScheduleNextOriginPulse();
};
