#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HunterBehaviourPredictionSubsystem.generated.h"

/**
 * Tiny local supervised logistic-regression model. It predicts whether a
 * player will evade a telegraphed attack from normalized combat observations.
 * Training labels are supplied by gameplay outcomes; no hosted AI is used.
 */
UCLASS()
class HUNTERGAME_API UHunterBehaviourPredictionSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category="AI|Supervised Learning")
    float PredictEvasionProbability(float DistanceRatio, float SpeedRatio, float HealthDeficit) const;

    UFUNCTION(BlueprintCallable, Category="AI|Supervised Learning")
    void RecordOutcome(float DistanceRatio, float SpeedRatio, float HealthDeficit, bool bEvaded);

    UFUNCTION(BlueprintCallable, Category="AI|Supervised Learning|Save")
    bool SaveModel();

    UFUNCTION(BlueprintCallable, Category="AI|Supervised Learning|Save")
    bool LoadModel();

    UFUNCTION(BlueprintCallable, Category="AI|Supervised Learning|Save")
    void ResetModel();

    UFUNCTION(BlueprintPure, Category="AI|Supervised Learning")
    int64 GetTrainingSampleCount() const { return TrainingSamples; }

private:
    static constexpr int32 FeatureCount = 4; // bias + three normalized inputs

    UPROPERTY(Transient)
    TArray<float> Weights;

    UPROPERTY(Transient)
    int64 TrainingSamples = 0;

    int32 SamplesSinceSave = 0;
    float LearningRate = 0.08f;

    TArray<float> BuildFeatures(float DistanceRatio, float SpeedRatio, float HealthDeficit) const;
    float Predict(const TArray<float>& Features) const;
    void ValidateModel();
};
