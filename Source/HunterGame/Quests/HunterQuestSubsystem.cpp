#include "HunterQuestSubsystem.h"

namespace
{
    bool IsValidObjectiveType(EHunterObjectiveType Type)
    {
        return static_cast<uint8>(Type) <= static_cast<uint8>(EHunterObjectiveType::Survive);
    }
}

bool UHunterQuestSubsystem::StartQuest(FName NewQuestId, const TArray<FHunterObjective>& NewObjectives)
{
    if (NewQuestId.IsNone() || NewObjectives.IsEmpty() || IsQuestActive())
    {
        return false;
    }

    TSet<FName> SeenIds;
    for (const FHunterObjective& Objective : NewObjectives)
    {
        if (Objective.ObjectiveId.IsNone() || Objective.EventTag.IsNone() ||
            !IsValidObjectiveType(Objective.Type) || Objective.RequiredCount < 1 ||
            SeenIds.Contains(Objective.ObjectiveId))
        {
            return false;
        }
        SeenIds.Add(Objective.ObjectiveId);
    }

    ActiveQuestId = NewQuestId;
    Objectives = NewObjectives;
    for (FHunterObjective& Objective : Objectives)
    {
        Objective.RequiredCount = FMath::Max(1, Objective.RequiredCount);
        Objective.CurrentCount = 0;
        Objective.bCompleted = false;
    }
    bQuestCompleted = false;
    OnQuestChanged.Broadcast();
    return true;
}

bool UHunterQuestSubsystem::RegisterQuestEvent(FName EventTag, int32 Amount)
{
    if (!IsQuestActive() || bQuestCompleted || EventTag.IsNone() || Amount <= 0)
    {
        return false;
    }

    const FName QuestBeingUpdated = ActiveQuestId;
    TArray<TPair<FName, int32>> UpdatedObjectives;
    for (FHunterObjective& Objective : Objectives)
    {
        if (Objective.bCompleted || Objective.EventTag != EventTag)
        {
            continue;
        }

        const int64 UpdatedCount = static_cast<int64>(Objective.CurrentCount) + static_cast<int64>(Amount);
        Objective.CurrentCount = static_cast<int32>(FMath::Min<int64>(Objective.RequiredCount, UpdatedCount));
        Objective.bCompleted = Objective.CurrentCount >= Objective.RequiredCount;
        UpdatedObjectives.Emplace(Objective.ObjectiveId, Objective.CurrentCount);
    }

    if (UpdatedObjectives.IsEmpty())
    {
        return false;
    }

    bQuestCompleted = true;
    for (const FHunterObjective& Objective : Objectives)
    {
        if (!Objective.bCompleted)
        {
            bQuestCompleted = false;
            break;
        }
    }

    // Notify only after all objective mutations are complete. Blueprint listeners
    // may clear or replace the quest while handling a notification.
    for (const TPair<FName, int32>& Update : UpdatedObjectives)
    {
        OnObjectiveUpdated.Broadcast(Update.Key, Update.Value);
        if (ActiveQuestId != QuestBeingUpdated)
        {
            return true;
        }
    }

    if (bQuestCompleted && ActiveQuestId == QuestBeingUpdated)
    {
        OnQuestCompleted.Broadcast(QuestBeingUpdated);
    }

    return true;
}

void UHunterQuestSubsystem::ClearActiveQuest()
{
    ActiveQuestId = NAME_None;
    Objectives.Reset();
    bQuestCompleted = false;
    OnQuestChanged.Broadcast();
}

bool UHunterQuestSubsystem::RestoreQuestState(FName QuestId, const TArray<FHunterObjective>& SavedObjectives, bool bWasCompleted)
{
    if (QuestId.IsNone())
    {
        ClearActiveQuest();
        return SavedObjectives.IsEmpty();
    }

    if (SavedObjectives.IsEmpty())
    {
        return false;
    }

    TSet<FName> SeenIds;
    TArray<FHunterObjective> RestoredObjectives = SavedObjectives;
    bool bAllCompleted = true;
    for (FHunterObjective& Objective : RestoredObjectives)
    {
        if (Objective.ObjectiveId.IsNone() || Objective.EventTag.IsNone() ||
            !IsValidObjectiveType(Objective.Type) || Objective.RequiredCount < 1 ||
            SeenIds.Contains(Objective.ObjectiveId))
        {
            return false;
        }

        SeenIds.Add(Objective.ObjectiveId);
        Objective.CurrentCount = FMath::Clamp(Objective.CurrentCount, 0, Objective.RequiredCount);
        Objective.bCompleted = Objective.CurrentCount >= Objective.RequiredCount;
        bAllCompleted &= Objective.bCompleted;
    }

    ActiveQuestId = QuestId;
    Objectives = MoveTemp(RestoredObjectives);
    bQuestCompleted = bWasCompleted && bAllCompleted;

    for (const FHunterObjective& Objective : Objectives)
    {
        OnObjectiveUpdated.Broadcast(Objective.ObjectiveId, Objective.CurrentCount);
    }
    if (bQuestCompleted)
    {
        OnQuestCompleted.Broadcast(ActiveQuestId);
    }
    OnQuestChanged.Broadcast();
    return true;
}
