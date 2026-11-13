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
    ++QuestStateRevision;
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

    // A revision token catches even a clear-and-restart using the same quest ID.
    ++QuestStateRevision;
    const uint64 UpdatedRevision = QuestStateRevision;
    for (const TPair<FName, int32>& Update : UpdatedObjectives)
    {
        OnObjectiveUpdated.Broadcast(Update.Key, Update.Value);
        if (QuestStateRevision != UpdatedRevision || ActiveQuestId != QuestBeingUpdated)
        {
            return true;
        }
    }

    if (bQuestCompleted && QuestStateRevision == UpdatedRevision && ActiveQuestId == QuestBeingUpdated)
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
    ++QuestStateRevision;
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
    ++QuestStateRevision;
    const uint64 RestoredRevision = QuestStateRevision;
    const bool bRestoredQuestCompleted = bQuestCompleted;
    const FName RestoredQuestId = ActiveQuestId;

    // Publish the installed snapshot first. Stop if a listener replaces or clears it.
    OnQuestChanged.Broadcast();
    if (QuestStateRevision != RestoredRevision || ActiveQuestId != RestoredQuestId)
    {
        return true;
    }

    const TArray<FHunterObjective> NotificationSnapshot = Objectives;
    for (const FHunterObjective& Objective : NotificationSnapshot)
    {
        OnObjectiveUpdated.Broadcast(Objective.ObjectiveId, Objective.CurrentCount);
        if (QuestStateRevision != RestoredRevision || ActiveQuestId != RestoredQuestId)
        {
            return true;
        }
    }

    if (bRestoredQuestCompleted && QuestStateRevision == RestoredRevision && ActiveQuestId == RestoredQuestId)
    {
        OnQuestCompleted.Broadcast(RestoredQuestId);
    }
    return true;
}
