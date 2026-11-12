#include "HunterChapterSelectWidget.h"

#include "Engine/GameInstance.h"

void UHunterChapterSelectWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BindCampaign();
    RefreshChapterSelect();
}

void UHunterChapterSelectWidget::NativeDestruct()
{
    UnbindCampaign();
    Super::NativeDestruct();
}

void UHunterChapterSelectWidget::BindCampaign()
{
    if (!GetGameInstance())
    {
        return;
    }

    CampaignSubsystem = GetGameInstance()->GetSubsystem<UHunterCampaignSubsystem>();
    if (CampaignSubsystem)
    {
        CampaignSubsystem->OnChapterChanged.AddDynamic(this, &UHunterChapterSelectWidget::HandleChapterChanged);
        CampaignSubsystem->OnChapterCompleted.AddDynamic(this, &UHunterChapterSelectWidget::HandleChapterCompleted);
    }
}

void UHunterChapterSelectWidget::UnbindCampaign()
{
    if (CampaignSubsystem)
    {
        CampaignSubsystem->OnChapterChanged.RemoveDynamic(this, &UHunterChapterSelectWidget::HandleChapterChanged);
        CampaignSubsystem->OnChapterCompleted.RemoveDynamic(this, &UHunterChapterSelectWidget::HandleChapterCompleted);
        CampaignSubsystem = nullptr;
    }
}

bool UHunterChapterSelectWidget::StartChapterByNumber(int32 ChapterNumber)
{
    const bool bStarted = CampaignSubsystem && CampaignSubsystem->StartChapter(ChapterNumber);
    if (bStarted)
    {
        RefreshChapterSelect();
    }

    BP_OnChapterStartResult(ChapterNumber, bStarted);
    return bStarted;
}

void UHunterChapterSelectWidget::RefreshChapterSelect()
{
    if (!CampaignSubsystem)
    {
        BindCampaign();
    }

    Snapshot = FHunterChapterSelectSnapshot();
    if (!CampaignSubsystem)
    {
        BP_OnChapterSelectSnapshotUpdated(Snapshot);
        return;
    }

    Snapshot.Chapters = CampaignSubsystem->GetChapters();
    Snapshot.CurrentChapterNumber = CampaignSubsystem->GetCurrentChapterNumber();

    for (const FHunterChapter& Chapter : Snapshot.Chapters)
    {
        Snapshot.CompletedChapterCount += Chapter.bCompleted ? 1 : 0;
        Snapshot.UnlockedChapterCount += Chapter.bUnlocked ? 1 : 0;
    }

    Snapshot.bCampaignComplete = Snapshot.CompletedChapterCount >= Snapshot.Chapters.Num() &&
        !Snapshot.Chapters.IsEmpty();

    BP_OnChapterSelectSnapshotUpdated(Snapshot);
}

void UHunterChapterSelectWidget::HandleChapterChanged(int32 ChapterNumber)
{
    RefreshChapterSelect();
}

void UHunterChapterSelectWidget::HandleChapterCompleted(int32 ChapterNumber)
{
    RefreshChapterSelect();
}
