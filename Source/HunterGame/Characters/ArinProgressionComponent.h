#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ArinProgressionComponent.generated.h"

UENUM(BlueprintType)
enum class EArinEvolutionStage : uint8
{
    Awakening UMETA(DisplayName="Awakening"),
    Evolved UMETA(DisplayName="Evolved"),
    Elite UMETA(DisplayName="Elite"),
    Legendary UMETA(DisplayName="Legendary"),
    UltimateAwakening UMETA(DisplayName="Ultimate Awakening")
};

USTRUCT(BlueprintType)
struct FArinStageProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progression")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progression")
    int32 RequiredExperience = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progression")
    float HealthMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progression")
    float DamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progression")
    float MovementMultiplier = 1.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FArinStageChanged, EArinEvolutionStage, PreviousStage, EArinEvolutionStage, NewStage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FArinExperienceChanged, int32, NewExperience);

UCLASS(ClassGroup=(Hunter), meta=(BlueprintSpawnableComponent))
class HUNTERGAME_API UArinProgressionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UArinProgressionComponent();

    UFUNCTION(BlueprintCallable, Category="Progression")
    void AddExperience(int32 Amount);

    UFUNCTION(BlueprintCallable, Category="Progression")
    bool SetEvolutionStage(EArinEvolutionStage NewStage);

    /** Restores an exact saved progression state without adding saved XP twice. */
    UFUNCTION(BlueprintCallable, Category="Progression|Save")
    void RestoreProgression(int32 SavedExperience, EArinEvolutionStage SavedStage);

    UFUNCTION(BlueprintPure, Category="Progression")
    EArinEvolutionStage GetEvolutionStage() const { return CurrentStage; }

    UFUNCTION(BlueprintPure, Category="Progression")
    int32 GetExperience() const { return Experience; }

    UFUNCTION(BlueprintPure, Category="Progression")
    int32 GetExperienceForNextStage() const;

    UFUNCTION(BlueprintPure, Category="Progression")
    FArinStageProfile GetCurrentStageProfile() const;

    UFUNCTION(BlueprintPure, Category="Progression")
    bool HasUnlockedShadowStep() const;

    UFUNCTION(BlueprintPure, Category="Progression")
    bool HasUnlockedEnergyStrike() const;

    UFUNCTION(BlueprintPure, Category="Progression")
    bool HasUnlockedShadowEcho() const;

    UPROPERTY(BlueprintAssignable, Category="Progression")
    FArinStageChanged OnStageChanged;

    UPROPERTY(BlueprintAssignable, Category="Progression")
    FArinExperienceChanged OnExperienceChanged;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Progression")
    TArray<FArinStageProfile> StageProfiles;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Progression")
    int32 Experience = 0;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Progression")
    EArinEvolutionStage CurrentStage = EArinEvolutionStage::Awakening;

private:
    int32 GetStageIndex(EArinEvolutionStage Stage) const;
    void RecalculateStage();
};
