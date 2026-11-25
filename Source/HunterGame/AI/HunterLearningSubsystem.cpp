#include "HunterLearningSubsystem.h"

#include "HunterLearningSaveGame.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    constexpr TCHAR LearningSlotName[] = TEXT("HunterAILearning");
}

void UHunterLearningSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    EnsureValidTable();
    LoadLearning();
}

void UHunterLearningSubsystem::Deinitialize()
{
    SaveLearning();
    Super::Deinitialize();
}

void UHunterLearningSubsystem::EnsureValidTable()
{
    if (QValues.Num() != ExpectedQValueCount)
    {
        QValues.Init(0.0f, ExpectedQValueCount);
    }

    for (float& Value : QValues)
    {
        if (!FMath::IsFinite(Value))
        {
            Value = 0.0f;
        }
        Value = FMath::Clamp(Value, -25.0f, 25.0f);
    }

    ExplorationRate = FMath::IsFinite(ExplorationRate)
        ? FMath::Clamp(ExplorationRate, MinimumExplorationRate, 1.0f) : 0.10f;
    UpdateCount = FMath::Max<int64>(0, UpdateCount);
}

int32 UHunterLearningSubsystem::GetIndex(EHunterCombatLearningState State,
    EHunterCombatLearningAction Action) const
{
    const int32 StateIndex = FMath::Clamp(static_cast<int32>(State), 0, StateCount - 1);
    const int32 ActionIndex = FMath::Clamp(static_cast<int32>(Action), 0, ActionCount - 1);
    return StateIndex * ActionCount + ActionIndex;
}

float UHunterLearningSubsystem::GetBestFutureValue(EHunterCombatLearningState State) const
{
    const int32 StateIndex = FMath::Clamp(static_cast<int32>(State), 0, StateCount - 1);
    float BestValue = -25.0f;
    for (int32 ActionIndex = 0; ActionIndex < ActionCount; ++ActionIndex)
    {
        const int32 Index = StateIndex * ActionCount + ActionIndex;
        if (QValues.IsValidIndex(Index) && FMath::IsFinite(QValues[Index]))
        {
            BestValue = FMath::Max(BestValue, QValues[Index]);
        }
    }
    return BestValue;
}

EHunterCombatLearningAction UHunterLearningSubsystem::ChooseCombatAction(
    EHunterCombatLearningState State)
{
    EnsureValidTable();

    if (FMath::FRand() < ExplorationRate)
    {
        return static_cast<EHunterCombatLearningAction>(FMath::RandRange(0, ActionCount - 1));
    }

    const int32 StateIndex = FMath::Clamp(static_cast<int32>(State), 0, StateCount - 1);
    int32 BestAction = 0;
    float BestValue = QValues[StateIndex * ActionCount];
    for (int32 ActionIndex = 1; ActionIndex < ActionCount; ++ActionIndex)
    {
        const float Candidate = QValues[StateIndex * ActionCount + ActionIndex];
        if (Candidate > BestValue)
        {
            BestValue = Candidate;
            BestAction = ActionIndex;
        }
    }
    return static_cast<EHunterCombatLearningAction>(BestAction);
}

void UHunterLearningSubsystem::RecordCombatOutcome(EHunterCombatLearningState State,
    EHunterCombatLearningAction Action, float Reward,
    EHunterCombatLearningState NextState)
{
    if (!FMath::IsFinite(Reward))
    {
        return;
    }

    EnsureValidTable();
    const int32 Index = GetIndex(State, Action);
    if (!QValues.IsValidIndex(Index))
    {
        return;
    }

    const float SafeReward = FMath::Clamp(Reward, -5.0f, 5.0f);
    const float OldValue = QValues[Index];
    const float TargetValue = SafeReward + DiscountFactor * GetBestFutureValue(NextState);
    const float UpdatedValue = OldValue + LearningRate * (TargetValue - OldValue);
    if (!FMath::IsFinite(UpdatedValue))
    {
        return;
    }

    QValues[Index] = FMath::Clamp(UpdatedValue, -25.0f, 25.0f);
    ++UpdateCount;
    ++UpdatesSinceSave;
    ExplorationRate = FMath::Max(MinimumExplorationRate, ExplorationRate * 0.9995f);

    if (UpdatesSinceSave >= 20)
    {
        SaveLearning();
    }
}

bool UHunterLearningSubsystem::SaveLearning()
{
    EnsureValidTable();
    UHunterLearningSaveGame* SaveObject = Cast<UHunterLearningSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UHunterLearningSaveGame::StaticClass()));
    if (!SaveObject)
    {
        return false;
    }

    SaveObject->QValues = QValues;
    SaveObject->ExplorationRate = ExplorationRate;
    SaveObject->UpdateCount = UpdateCount;
    const bool bSaved = UGameplayStatics::SaveGameToSlot(SaveObject, LearningSlotName, 0);
    if (bSaved)
    {
        UpdatesSinceSave = 0;
    }
    return bSaved;
}

bool UHunterLearningSubsystem::LoadLearning()
{
    if (!UGameplayStatics::DoesSaveGameExist(LearningSlotName, 0))
    {
        EnsureValidTable();
        return false;
    }

    UHunterLearningSaveGame* SaveObject = Cast<UHunterLearningSaveGame>(
        UGameplayStatics::LoadGameFromSlot(LearningSlotName, 0));
    if (!SaveObject || SaveObject->SaveVersion != 1 ||
        SaveObject->QValues.Num() != ExpectedQValueCount)
    {
        ResetLearning();
        return false;
    }

    QValues = SaveObject->QValues;
    ExplorationRate = SaveObject->ExplorationRate;
    UpdateCount = SaveObject->UpdateCount;
    EnsureValidTable();
    UpdatesSinceSave = 0;
    return true;
}

void UHunterLearningSubsystem::ResetLearning()
{
    QValues.Init(0.0f, ExpectedQValueCount);
    ExplorationRate = 0.10f;
    UpdateCount = 0;
    UpdatesSinceSave = 0;
    SaveLearning();
}
