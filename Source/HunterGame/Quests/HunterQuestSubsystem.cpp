#include "HunterQuestSubsystem.h"

bool UHunterQuestSubsystem::StartQuest(FName NewQuestId, const TArray<FHunterObjective>& NewObjectives)
{
    if (NewQuestId.IsNone() || NewObjectives.IsEmpty())
    {
        return false;
    }

    TSet<FName> SeenIds;
    for (const FHunterObjective& Objective : NewObjectives)
    {
        if (Objective.ObjectiveId.IsNone() || Objective.EventTag.IsNone() ||
            Objective.RequiredCount < 1 || SeenIds.Contains(Objective.ObjectiveId))
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
    return true;
}

bool UHunterQuestSubsystem::RegisterQuestEvent(FName EventTag, int32 Amount)
{
    if (!IsQuestActive() || bQuestCompleted || EventTag.IsNone() || Amount <= 0)
    {
        return false;
    }

    bool bUpdatedAny = false;
    for (FHunterObjective& Objective : Objectives)
    {
        if (Objective.bCompleted || Objective.EventTag != EventTag)
        {
            continue;
        }

        Objective.CurrentCount = FMath::Min(Objective.RequiredCount, Objective.CurrentCount + Amount);
        Objective.bCompleted = Objective.CurrentCount >= Objective.RequiredCount;
        OnObjectiveUpdated.Broadcast(Objective.ObjectiveId, Objective.CurrentCount);
        bUpdatedAny = true;
    }

    if (bUpdatedAny)
    {
        bQuestCompleted = true;
        for (const FHunterObjective& Objective : Objectives)
        {
            if (!Objective.bCompleted)
            {
                bQuestCompleted = false;
                break;
            }
        }

        if (bQuestCompleted)
        {
            OnQuestCompleted.Broadcast(ActiveQuestId);
        }
    }

    return bUpdatedAny;
}

void UHunterQuestSubsystem::ClearActiveQuest()
{
    ActiveQuestId = NAME_None;
    Objectives.Reset();
    bQuestCompleted = false;
}
