#include "HunterQuestTrackerWidget.h"

#include "Engine/GameInstance.h"

void UHunterQuestTrackerWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BindSubsystems();
    RefreshTracker();
}

void UHunterQuestTrackerWidget::NativeDestruct()
{
    UnbindSubsystems();
    Super::NativeDestruct();
}

void UHunterQuestTrackerWidget::BindSubsystems()
{
    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance)
    {
        return;
    }

    QuestSubsystem = GameInstance->GetSubsystem<UHunterQuestSubsystem>();
    CampaignSubsystem = GameInstance->GetSubsystem<UHunterCampaignSubsystem>();

    if (QuestSubsystem)
    {
        QuestSubsystem->OnObjectiveUpdated.AddDynamic(this, &UHunterQuestTrackerWidget::HandleObjectiveUpdated);
        QuestSubsystem->OnQuestCompleted.AddDynamic(this, &UHunterQuestTrackerWidget::HandleQuestCompleted);
        QuestSubsystem->OnQuestChanged.AddDynamic(this, &UHunterQuestTrackerWidget::HandleQuestChanged);
    }
    if (CampaignSubsystem)
    {
        CampaignSubsystem->OnChapterChanged.AddDynamic(this, &UHunterQuestTrackerWidget::HandleChapterChanged);
        CampaignSubsystem->OnChapterCompleted.AddDynamic(this, &UHunterQuestTrackerWidget::HandleChapterCompleted);
    }
}

void UHunterQuestTrackerWidget::UnbindSubsystems()
{
    if (QuestSubsystem)
    {
        QuestSubsystem->OnObjectiveUpdated.RemoveDynamic(this, &UHunterQuestTrackerWidget::HandleObjectiveUpdated);
        QuestSubsystem->OnQuestCompleted.RemoveDynamic(this, &UHunterQuestTrackerWidget::HandleQuestCompleted);
        QuestSubsystem->OnQuestChanged.RemoveDynamic(this, &UHunterQuestTrackerWidget::HandleQuestChanged);
    }
    if (CampaignSubsystem)
    {
        CampaignSubsystem->OnChapterChanged.RemoveDynamic(this, &UHunterQuestTrackerWidget::HandleChapterChanged);
        CampaignSubsystem->OnChapterCompleted.RemoveDynamic(this, &UHunterQuestTrackerWidget::HandleChapterCompleted);
    }
    QuestSubsystem = nullptr;
    CampaignSubsystem = nullptr;
}

void UHunterQuestTrackerWidget::RefreshTracker()
{
    Snapshot = FHunterQuestTrackerSnapshot();

    if (QuestSubsystem)
    {
        Snapshot.QuestId = QuestSubsystem->GetActiveQuestId();
        Snapshot.Objectives = QuestSubsystem->GetObjectives();
        Snapshot.bQuestActive = QuestSubsystem->IsQuestActive();
        Snapshot.bQuestCompleted = QuestSubsystem->IsQuestComplete();
        Snapshot.TotalObjectives = Snapshot.Objectives.Num();

        for (const FHunterObjective& Objective : Snapshot.Objectives)
        {
            if (Objective.bCompleted)
            {
                ++Snapshot.CompletedObjectives;
            }
        }

        Snapshot.CompletionPercent = Snapshot.TotalObjectives > 0
            ? static_cast<float>(Snapshot.CompletedObjectives) / Snapshot.TotalObjectives
            : 0.0f;
    }

    if (CampaignSubsystem)
    {
        const TArray<FHunterChapter> Chapters = CampaignSubsystem->GetChapters();
        Snapshot.CurrentChapterNumber = CampaignSubsystem->GetCurrentChapterNumber();
        Snapshot.TotalChapters = Chapters.Num();
        const FHunterChapter CurrentChapter = CampaignSubsystem->GetCurrentChapter();
        Snapshot.ChapterTitle = CurrentChapter.Title;
        Snapshot.ChapterBriefing = CurrentChapter.Briefing;

        for (const FHunterChapter& Chapter : Chapters)
        {
            if (Chapter.bCompleted)
            {
                ++Snapshot.CompletedChapters;
            }
        }
    }

    BP_OnTrackerSnapshotUpdated(Snapshot);
}

void UHunterQuestTrackerWidget::HandleObjectiveUpdated(FName ObjectiveId, int32 CurrentCount)
{
    RefreshTracker();
}

void UHunterQuestTrackerWidget::HandleQuestCompleted(FName QuestId)
{
    RefreshTracker();
}

void UHunterQuestTrackerWidget::HandleQuestChanged()
{
    RefreshTracker();
}

void UHunterQuestTrackerWidget::HandleChapterChanged(int32 ChapterNumber)
{
    RefreshTracker();
}

void UHunterQuestTrackerWidget::HandleChapterCompleted(int32 ChapterNumber)
{
    RefreshTracker();
}
