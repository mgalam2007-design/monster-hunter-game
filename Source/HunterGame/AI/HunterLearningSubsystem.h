#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HunterLearningSubsystem.generated.h"

UENUM(BlueprintType)
enum class EHunterCombatLearningState : uint8
{
    PlayerFar = 0,
    PlayerApproaching = 1,
    PlayerInRange = 2,
    PlayerLowHealth = 3,
    PlayerRetreating = 4,
    PlayerDodging = 5
};

UENUM(BlueprintType)
enum class EHunterCombatLearningAction : uint8
{
    Balanced = 0,
    Aggressive = 1,
    Cautious = 2
};

/**
 * Small tabular Q-learning agent. This is an original offline game AI system,
 * not a hosted language model and not a recreation of GPT.
 */
UCLASS()
class HUNTERGAME_API UHunterLearningSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category="AI|Learning")
    EHunterCombatLearningAction ChooseCombatAction(EHunterCombatLearningState State);

    UFUNCTION(BlueprintCallable, Category="AI|Learning")
    void RecordCombatOutcome(EHunterCombatLearningState State,
        EHunterCombatLearningAction Action, float Reward,
        EHunterCombatLearningState NextState);

    UFUNCTION(BlueprintCallable, Category="AI|Learning|Save")
    bool SaveLearning();

    UFUNCTION(BlueprintCallable, Category="AI|Learning|Save")
    bool LoadLearning();

    UFUNCTION(BlueprintCallable, Category="AI|Learning|Save")
    void ResetLearning();

    UFUNCTION(BlueprintPure, Category="AI|Learning")
    int64 GetLearningUpdateCount() const { return UpdateCount; }

    UFUNCTION(BlueprintPure, Category="AI|Learning")
    float GetExplorationRate() const { return ExplorationRate; }

private:
    static constexpr int32 StateCount = 6;
    static constexpr int32 ActionCount = 3;
    static constexpr int32 ExpectedQValueCount = StateCount * ActionCount;

    UPROPERTY(Transient)
    TArray<float> QValues;

    UPROPERTY(Transient)
    float ExplorationRate = 0.10f;

    UPROPERTY(Transient)
    int64 UpdateCount = 0;

    float LearningRate = 0.15f;
    float DiscountFactor = 0.85f;
    float MinimumExplorationRate = 0.03f;
    int32 UpdatesSinceSave = 0;

    int32 GetIndex(EHunterCombatLearningState State, EHunterCombatLearningAction Action) const;
    float GetBestFutureValue(EHunterCombatLearningState State) const;
    void EnsureValidTable();
};
