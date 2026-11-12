#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Campaign/HunterCampaignSubsystem.h"
#include "HunterChapterSelectWidget.generated.h"

USTRUCT(BlueprintType)
struct FHunterChapterSelectSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Chapter Select")
    TArray<FHunterChapter> Chapters;

    UPROPERTY(BlueprintReadOnly, Category="Chapter Select")
    int32 CurrentChapterNumber = 1;

    UPROPERTY(BlueprintReadOnly, Category="Chapter Select")
    int32 CompletedChapterCount = 0;

    UPROPERTY(BlueprintReadOnly, Category="Chapter Select")
    int32 UnlockedChapterCount = 1;

    UPROPERTY(BlueprintReadOnly, Category="Chapter Select")
    bool bCampaignComplete = false;
};

/** Blueprint-ready chapter selection data bridge. Build the actual menu in a Widget Blueprint child. */
UCLASS()
class HUNTERGAME_API UHunterChapterSelectWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category="Chapter Select")
    FHunterChapterSelectSnapshot GetChapterSelectSnapshot() const { return Snapshot; }

    UFUNCTION(BlueprintCallable, Category="Chapter Select")
    bool StartChapterByNumber(int32 ChapterNumber);

    UFUNCTION(BlueprintCallable, Category="Chapter Select")
    void RefreshChapterSelect();

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeDestruct() override;

    UFUNCTION(BlueprintImplementableEvent, Category="Chapter Select")
    void BP_OnChapterSelectSnapshotUpdated(const FHunterChapterSelectSnapshot& NewSnapshot);

    UFUNCTION(BlueprintImplementableEvent, Category="Chapter Select")
    void BP_OnChapterStartResult(int32 ChapterNumber, bool bStarted);

private:
    UPROPERTY(Transient)
    TObjectPtr<UHunterCampaignSubsystem> CampaignSubsystem;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Chapter Select", meta=(AllowPrivateAccess="true"))
    FHunterChapterSelectSnapshot Snapshot;

    UFUNCTION()
    void HandleChapterChanged(int32 ChapterNumber);

    UFUNCTION()
    void HandleChapterCompleted(int32 ChapterNumber);

    void BindCampaign();
    void UnbindCampaign();
};
