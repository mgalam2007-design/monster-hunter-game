#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Quests/HunterQuestSubsystem.h"
#include "Campaign/HunterCampaignSubsystem.h"
#include "HunterQuestTrackerWidget.generated.h"

USTRUCT(BlueprintType)
struct FHunterQuestTrackerSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Quest Tracker")
    FName QuestId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="Quest Tracker")
    TArray<FHunterObjective> Objectives;

    UPROPERTY(BlueprintReadOnly, Category="Quest Tracker")
    int32 CompletedObjectives = 0;

    UPROPERTY(BlueprintReadOnly, Category="Quest Tracker")
    int32 TotalObjectives = 0;

    UPROPERTY(BlueprintReadOnly, Category="Quest Tracker")
    float CompletionPercent = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="Quest Tracker")
    bool bQuestActive = false;

    UPROPERTY(BlueprintReadOnly, Category="Quest Tracker")
    bool bQuestCompleted = false;

    UPROPERTY(BlueprintReadOnly, Category="Campaign")
    int32 CurrentChapterNumber = 1;

    UPROPERTY(BlueprintReadOnly, Category="Campaign")
    int32 TotalChapters = 20;

    UPROPERTY(BlueprintReadOnly, Category="Campaign")
    FText ChapterTitle;

    UPROPERTY(BlueprintReadOnly, Category="Campaign")
    FText ChapterBriefing;

    UPROPERTY(BlueprintReadOnly, Category="Campaign")
    int32 CompletedChapters = 0;
};

/** Blueprint-ready quest and chapter data bridge; visuals are authored in a Widget Blueprint child. */
UCLASS()
class HUNTERGAME_API UHunterQuestTrackerWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category="Quest Tracker")
    FHunterQuestTrackerSnapshot GetTrackerSnapshot() const { return Snapshot; }

    UFUNCTION(BlueprintCallable, Category="Quest Tracker")
    void RefreshTracker();

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeDestruct() override;

    UFUNCTION(BlueprintImplementableEvent, Category="Quest Tracker")
    void BP_OnTrackerSnapshotUpdated(const FHunterQuestTrackerSnapshot& NewSnapshot);

private:
    UPROPERTY(Transient)
    TObjectPtr<UHunterQuestSubsystem> QuestSubsystem;

    UPROPERTY(Transient)
    TObjectPtr<UHunterCampaignSubsystem> CampaignSubsystem;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Quest Tracker", meta=(AllowPrivateAccess="true"))
    FHunterQuestTrackerSnapshot Snapshot;

    UFUNCTION()
    void HandleObjectiveUpdated(FName ObjectiveId, int32 CurrentCount);

    UFUNCTION()
    void HandleQuestCompleted(FName QuestId);

    UFUNCTION()
    void HandleChapterChanged(int32 ChapterNumber);

    UFUNCTION()
    void HandleChapterCompleted(int32 ChapterNumber);

    void BindSubsystems();
    void UnbindSubsystems();
};
