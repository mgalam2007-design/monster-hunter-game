#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HunterCampaignSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FHunterChapter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Campaign")
    int32 ChapterNumber = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Campaign")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Campaign", meta=(MultiLine=true))
    FText Briefing;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Campaign")
    int32 RecommendedEvolutionStage = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Campaign")
    bool bCompleted = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Campaign")
    bool bUnlocked = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHunterChapterChanged, int32, ChapterNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHunterChapterCompleted, int32, ChapterNumber);

/**
 * Owns the original 20-chapter campaign progression. Level designers can
 * replace briefing text and recommended stages in a Blueprint subclass.
 */
UCLASS()
class HUNTERGAME_API UHunterCampaignSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UHunterCampaignSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category="Campaign")
    bool StartChapter(int32 ChapterNumber);

    UFUNCTION(BlueprintCallable, Category="Campaign")
    bool CompleteCurrentChapter();

    UFUNCTION(BlueprintPure, Category="Campaign")
    int32 GetCurrentChapterNumber() const { return CurrentChapterNumber; }

    UFUNCTION(BlueprintPure, Category="Campaign")
    TArray<FHunterChapter> GetChapters() const { return Chapters; }

    UFUNCTION(BlueprintPure, Category="Campaign")
    FHunterChapter GetCurrentChapter() const;

    UPROPERTY(BlueprintAssignable, Category="Campaign")
    FHunterChapterChanged OnChapterChanged;

    UPROPERTY(BlueprintAssignable, Category="Campaign")
    FHunterChapterCompleted OnChapterCompleted;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Campaign")
    TArray<FHunterChapter> Chapters;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Campaign")
    int32 CurrentChapterNumber = 1;

private:
    void BuildDefaultCampaign();
};
