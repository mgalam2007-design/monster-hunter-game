#include "AI/HunterBehaviourPredictionSubsystem.h"
#include "AI/HunterBehaviourModelSaveGame.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    const TCHAR* BehaviourModelSlot = TEXT("HunterBehaviourModel");
    constexpr int32 BehaviourModelUser = 0;
}

void UHunterBehaviourPredictionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ResetModel();
    LoadModel();
}

void UHunterBehaviourPredictionSubsystem::Deinitialize()
{
    SaveModel();
    Super::Deinitialize();
}

void UHunterBehaviourPredictionSubsystem::ValidateModel()
{
    if (Weights.Num() != FeatureCount)
    {
        Weights.Init(0.0f, FeatureCount);
    }
    for (float& Weight : Weights)
    {
        if (!FMath::IsFinite(Weight))
        {
            Weight = 0.0f;
        }
        Weight = FMath::Clamp(Weight, -8.0f, 8.0f);
    }
    TrainingSamples = FMath::Max<int64>(0, TrainingSamples);
}

void UHunterBehaviourPredictionSubsystem::ResetModel()
{
    Weights.Init(0.0f, FeatureCount);
    TrainingSamples = 0;
    SamplesSinceSave = 0;
}

TArray<float> UHunterBehaviourPredictionSubsystem::BuildFeatures(
    float DistanceRatio, float SpeedRatio, float HealthDeficit) const
{
    TArray<float> Features;
    Features.Reserve(FeatureCount);
    Features.Add(1.0f);
    Features.Add(FMath::IsFinite(DistanceRatio) ? FMath::Clamp(DistanceRatio, 0.0f, 1.5f) : 0.0f);
    Features.Add(FMath::IsFinite(SpeedRatio) ? FMath::Clamp(SpeedRatio, 0.0f, 2.0f) : 0.0f);
    Features.Add(FMath::IsFinite(HealthDeficit) ? FMath::Clamp(HealthDeficit, 0.0f, 1.0f) : 0.0f);
    return Features;
}

float UHunterBehaviourPredictionSubsystem::Predict(const TArray<float>& Features) const
{
    if (Features.Num() != FeatureCount || Weights.Num() != FeatureCount)
    {
        return 0.5f;
    }

    float Logit = 0.0f;
    for (int32 Index = 0; Index < FeatureCount; ++Index)
    {
        if (!FMath::IsFinite(Features[Index]) || !FMath::IsFinite(Weights[Index]))
        {
            return 0.5f;
        }
        Logit += Features[Index] * Weights[Index];
    }
    Logit = FMath::Clamp(Logit, -12.0f, 12.0f);
    return 1.0f / (1.0f + FMath::Exp(-Logit));
}

float UHunterBehaviourPredictionSubsystem::PredictEvasionProbability(
    float DistanceRatio, float SpeedRatio, float HealthDeficit) const
{
    return Predict(BuildFeatures(DistanceRatio, SpeedRatio, HealthDeficit));
}

void UHunterBehaviourPredictionSubsystem::RecordOutcome(
    float DistanceRatio, float SpeedRatio, float HealthDeficit, bool bEvaded)
{
    ValidateModel();
    const TArray<float> Features = BuildFeatures(DistanceRatio, SpeedRatio, HealthDeficit);
    const float Prediction = Predict(Features);
    const float Label = bEvaded ? 1.0f : 0.0f;

    // Online logistic-regression gradient descent with bounded weights.
    for (int32 Index = 0; Index < FeatureCount; ++Index)
    {
        const float Gradient = (Label - Prediction) * Features[Index];
        Weights[Index] = FMath::Clamp(
            Weights[Index] + LearningRate * Gradient, -8.0f, 8.0f);
    }

    ++TrainingSamples;
    ++SamplesSinceSave;
    if (SamplesSinceSave >= 20)
    {
        SaveModel();
    }
}

bool UHunterBehaviourPredictionSubsystem::SaveModel()
{
    ValidateModel();
    UHunterBehaviourModelSaveGame* Save = Cast<UHunterBehaviourModelSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UHunterBehaviourModelSaveGame::StaticClass()));
    if (!Save)
    {
        return false;
    }
    Save->SaveVersion = 1;
    Save->Weights = Weights;
    Save->TrainingSamples = TrainingSamples;
    const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, BehaviourModelSlot, BehaviourModelUser);
    if (bSaved)
    {
        SamplesSinceSave = 0;
    }
    return bSaved;
}

bool UHunterBehaviourPredictionSubsystem::LoadModel()
{
    if (!UGameplayStatics::DoesSaveGameExist(BehaviourModelSlot, BehaviourModelUser))
    {
        return false;
    }
    UHunterBehaviourModelSaveGame* Save = Cast<UHunterBehaviourModelSaveGame>(
        UGameplayStatics::LoadGameFromSlot(BehaviourModelSlot, BehaviourModelUser));
    if (!Save || Save->SaveVersion != 1 || Save->Weights.Num() != FeatureCount)
    {
        ResetModel();
        return false;
    }
    Weights = Save->Weights;
    TrainingSamples = Save->TrainingSamples;
    ValidateModel();
    SamplesSinceSave = 0;
    return true;
}
