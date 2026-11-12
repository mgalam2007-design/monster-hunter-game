#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HunterSaveLoadWidget.generated.h"

class AArinValeCharacter;

/** Save/load state exposed to a Blueprint-created menu. */
USTRUCT(BlueprintType)
struct FHunterSaveSlotSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Save")
    FString SlotName;

    UPROPERTY(BlueprintReadOnly, Category="Save")
    bool bSaveExists = false;

    UPROPERTY(BlueprintReadOnly, Category="Save")
    int32 CurrentChapterNumber = 1;

    UPROPERTY(BlueprintReadOnly, Category="Save")
    int32 CompletedChapterCount = 0;

    UPROPERTY(BlueprintReadOnly, Category="Save")
    int32 ArinExperience = 0;

    UPROPERTY(BlueprintReadOnly, Category="Save")
    int64 ArinCurrency = 0;
};

/** Blueprint-ready single-player save slot controller. */
UCLASS()
class HUNTERGAME_API UHunterSaveLoadWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category="Save")
    FHunterSaveSlotSnapshot GetSaveSlotSnapshot() const { return Snapshot; }

    UFUNCTION(BlueprintCallable, Category="Save")
    bool SaveCurrentGame();

    UFUNCTION(BlueprintCallable, Category="Save")
    bool LoadSavedGame();

    UFUNCTION(BlueprintCallable, Category="Save")
    void RefreshSaveSlot();

protected:
    virtual void NativeOnInitialized() override;

    UFUNCTION(BlueprintImplementableEvent, Category="Save")
    void BP_OnSaveSlotSnapshotUpdated(const FHunterSaveSlotSnapshot& NewSnapshot);

    UFUNCTION(BlueprintImplementableEvent, Category="Save")
    void BP_OnSaveLoadResult(bool bWasSaveOperation, bool bSucceeded);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Save")
    FString SaveSlotName = TEXT("HunterCampaign");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Save")
    int32 UserIndex = 0;

private:
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Save", meta=(AllowPrivateAccess="true"))
    FHunterSaveSlotSnapshot Snapshot;

    AArinValeCharacter* GetControlledArin() const;
};
