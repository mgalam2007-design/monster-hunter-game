#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ArinProgressionComponent.h"
#include "HunterHUDWidget.generated.h"

class AArinValeCharacter;
class UHunterHealthComponent;
class UHunterAbilityComponent;
class UHunterCurrencyComponent;
class UArinProgressionComponent;

USTRUCT(BlueprintType)
struct FHunterHUDSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="HUD")
    float Health = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="HUD")
    float MaxHealth = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="HUD")
    float HealthPercent = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="HUD")
    float Energy = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="HUD")
    float MaxEnergy = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="HUD")
    float EnergyPercent = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="HUD")
    int32 Experience = 0;

    UPROPERTY(BlueprintReadOnly, Category="HUD")
    int32 ExperienceForNextStage = 0;

    UPROPERTY(BlueprintReadOnly, Category="HUD")
    float ExperiencePercent = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="HUD")
    EArinEvolutionStage EvolutionStage = EArinEvolutionStage::Awakening;

    UPROPERTY(BlueprintReadOnly, Category="HUD")
    FText EvolutionStageName;

    UPROPERTY(BlueprintReadOnly, Category="HUD")
    int64 Currency = 0;

    UPROPERTY(BlueprintReadOnly, Category="HUD")
    bool bIsDead = false;
};

/** Blueprint-presentable HUD data bridge. Create a Widget Blueprint child to design the actual screen. */
UCLASS()
class HUNTERGAME_API UHunterHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category="HUD")
    FHunterHUDSnapshot GetHUDSnapshot() const { return Snapshot; }

    UFUNCTION(BlueprintCallable, Category="HUD")
    void RefreshHUD();

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeDestruct() override;

    UFUNCTION(BlueprintImplementableEvent, Category="HUD")
    void BP_OnHUDSnapshotUpdated(const FHunterHUDSnapshot& NewSnapshot);

private:
    UPROPERTY(Transient)
    TObjectPtr<AArinValeCharacter> ArinCharacter;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="HUD", meta=(AllowPrivateAccess="true"))
    FHunterHUDSnapshot Snapshot;

    UFUNCTION()
    void HandleHealthChanged(float CurrentHealth, float MaxHealth);

    UFUNCTION()
    void HandleEnergyChanged(float CurrentEnergy, float MaxEnergy);

    UFUNCTION()
    void HandleExperienceChanged(int32 NewExperience);

    UFUNCTION()
    void HandleStageChanged(EArinEvolutionStage PreviousStage, EArinEvolutionStage NewStage);

    UFUNCTION()
    void HandleCurrencyChanged(int64 NewBalance);

    void BindToCharacter();
    void UnbindFromCharacter();
};
