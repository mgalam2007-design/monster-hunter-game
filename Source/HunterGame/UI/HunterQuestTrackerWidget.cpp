#include "HunterQuestTrackerWidget.h"

#include "Engine/GameInstance.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"

void UHunterQuestTrackerWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BuildNativeLayout();
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

    ApplyNativeLayout();
    BP_OnTrackerSnapshotUpdated(Snapshot);
}

void UHunterQuestTrackerWidget::BuildNativeLayout()
{
    if (!WidgetTree || WidgetTree->RootWidget)
    {
        return;
    }

    NativeRoot = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("NativeQuestRoot"));
    WidgetTree->RootWidget = NativeRoot;
    UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("QuestPanel"));
    Panel->SetBrushColor(FLinearColor(0.015f, 0.025f, 0.045f, 0.86f));
    Panel->SetPadding(FMargin(16.0f));
    UCanvasPanelSlot* PanelSlot = NativeRoot->AddChildToCanvas(Panel);
    PanelSlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 0.0f));
    PanelSlot->SetAlignment(FVector2D(1.0f, 0.0f));
    PanelSlot->SetPosition(FVector2D(-28.0f, 28.0f));
    PanelSlot->SetSize(FVector2D(390.0f, 220.0f));
    NativeStack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("QuestStack"));
    Panel->SetContent(NativeStack);
}

void UHunterQuestTrackerWidget::ApplyNativeLayout()
{
    if (!NativeStack || !WidgetTree)
    {
        return;
    }

    NativeStack->ClearChildren();
    auto AddLine = [this](const FText& Value, int32 FontSize, const FLinearColor& Color)
    {
        UTextBlock* Line = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        Line->SetText(Value);
        FSlateFontInfo Font = Line->GetFont();
        Font.Size = FontSize;
        Line->SetFont(Font);
        Line->SetColorAndOpacity(FSlateColor(Color));
        UVerticalBoxSlot* Slot = NativeStack->AddChildToVerticalBox(Line);
        Slot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 5.0f));
    };

    AddLine(FText::FromString(FString::Printf(TEXT("HUNTER GUILD  //  CHAPTER %02d"), Snapshot.CurrentChapterNumber)),
        11, FLinearColor(0.25f, 0.68f, 1.0f, 1.0f));
    AddLine(Snapshot.ChapterTitle, 19, FLinearColor(0.95f, 0.96f, 1.0f, 1.0f));

    if (!Snapshot.bQuestActive)
    {
        AddLine(FText::FromString(TEXT("No active contract")), 13, FLinearColor(0.68f, 0.73f, 0.82f, 1.0f));
        return;
    }

    for (const FHunterObjective& Objective : Snapshot.Objectives)
    {
        const FString State = Objective.bCompleted ? TEXT("[DONE] ") : TEXT("[ ] ");
        const FString Progress = FString::Printf(TEXT("  %d/%d"), Objective.CurrentCount, Objective.RequiredCount);
        AddLine(FText::FromString(State + Objective.Description.ToString() + Progress),
            12, Objective.bCompleted ? FLinearColor(0.42f, 0.9f, 0.62f, 1.0f) : FLinearColor(0.86f, 0.89f, 0.95f, 1.0f));
    }
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
