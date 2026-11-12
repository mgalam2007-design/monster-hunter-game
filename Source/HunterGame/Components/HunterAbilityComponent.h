#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HunterAbilityComponent.generated.h"

class UArinProgressionComponent;

UENUM(BlueprintType)
enum class EHunterAbility : uint8
{
    EnergyStrike,
    ShadowStep,
    ShadowEcho
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHunterEnergyChanged, float, CurrentEnergy, float, MaxEnergy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHunterAbilityActivated, EHunterAbility, Ability);

UCLASS(ClassGroup=(Hunter), meta=(BlueprintSpawnableComponent))
class HUNTERGAME_API UHunterAbilityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHunterAbilityComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category="Abilities")
    bool TryActivateAbility(EHunterAbility Ability);

    UFUNCTION(BlueprintPure, Category="Abilities")
    float GetCurrentEnergy() const { return CurrentEnergy; }

    UFUNCTION(BlueprintPure, Category="Abilities")
    float GetMaxEnergy() const { return MaxEnergy; }

    UFUNCTION(BlueprintPure, Category="Abilities")
    float GetEnergyPercent() const { return MaxEnergy > 0.0f ? CurrentEnergy / MaxEnergy : 0.0f; }

    UFUNCTION(BlueprintCallable, Category="Abilities")
    float RestoreEnergy(float Amount);

    UFUNCTION(BlueprintPure, Category="Abilities")
    float GetCooldownRemaining(EHunterAbility Ability) const;

    UPROPERTY(BlueprintAssignable, Category="Abilities")
    FHunterEnergyChanged OnEnergyChanged;

    UPROPERTY(BlueprintAssignable, Category="Abilities")
    FHunterAbilityActivated OnAbilityActivated;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Abilities|Resource")
    float MaxEnergy = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Abilities|Resource")
    float EnergyRegenerationPerSecond = 8.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Abilities|EnergyStrike", meta=(ClampMin="0.0"))
    float EnergyStrikeDamage = 65.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Abilities|EnergyStrike", meta=(ClampMin="0.0"))
    float EnergyStrikeRadius = 420.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Abilities|ShadowStep", meta=(ClampMin="0.0"))
    float ShadowStepStrength = 1250.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Abilities|ShadowEcho")
    TSubclassOf<AActor> ShadowEchoActorClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Abilities|ShadowEcho", meta=(ClampMin="0.0"))
    float ShadowEchoSpawnDistance = 180.0f;

    UFUNCTION(BlueprintImplementableEvent, Category="Abilities")
    void BP_OnAbilityActivated(EHunterAbility Ability);

private:
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Abilities|Resource", meta=(AllowPrivateAccess="true"))
    float CurrentEnergy = 100.0f;

    TMap<EHunterAbility, float> CooldownRemaining;

    bool IsAbilityUnlocked(EHunterAbility Ability) const;
    bool ExecuteAbility(EHunterAbility Ability);
    float GetAbilityCost(EHunterAbility Ability) const;
    float GetAbilityCooldown(EHunterAbility Ability) const;
};
