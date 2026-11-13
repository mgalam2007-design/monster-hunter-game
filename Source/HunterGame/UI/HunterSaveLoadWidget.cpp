#include "HunterSaveLoadWidget.h"

#include "ArinValeCharacter.h"
#include "Campaign/HunterCampaignSubsystem.h"
#include "Campaign/HunterSaveGame.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void UHunterSaveLoadWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    RefreshSaveSlot();
}

AArinValeCharacter* UHunterSaveLoadWidget::GetControlledArin() const
{
    const APlayerController* PC = GetOwningPlayer();
    return PC ? Cast<AArinValeCharacter>(PC->GetPawn()) : nullptr;
}

bool UHunterSaveLoadWidget::SaveCurrentGame()
{
    AArinValeCharacter* Arin = GetControlledArin();
    UHunterCampaignSubsystem* Campaign = GetGameInstance()
        ? GetGameInstance()->GetSubsystem<UHunterCampaignSubsystem>()
        : nullptr;

    const bool bSucceeded = Arin && Campaign &&
        Campaign->SaveCampaign(Arin, SaveSlotName, UserIndex);

    RefreshSaveSlot();
    BP_OnSaveLoadResult(true, bSucceeded);
    return bSucceeded;
}

bool UHunterSaveLoadWidget::LoadSavedGame()
{
    AArinValeCharacter* Arin = GetControlledArin();
    UHunterCampaignSubsystem* Campaign = GetGameInstance()
        ? GetGameInstance()->GetSubsystem<UHunterCampaignSubsystem>()
        : nullptr;

    const bool bSucceeded = Arin && Campaign &&
        Campaign->LoadCampaign(Arin, SaveSlotName, UserIndex);

    RefreshSaveSlot();
    BP_OnSaveLoadResult(false, bSucceeded);
    return bSucceeded;
}

void UHunterSaveLoadWidget::RefreshSaveSlot()
{
    Snapshot = FHunterSaveSlotSnapshot();
    Snapshot.SlotName = SaveSlotName;
    Snapshot.bSaveExists = !SaveSlotName.IsEmpty() &&
        UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex);

    if (Snapshot.bSaveExists)
    {
        const UHunterSaveGame* SaveObject = Cast<UHunterSaveGame>(
            UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));

        bool bSnapshotValid = SaveObject &&
            SaveObject->SaveVersion >= 1 && SaveObject->SaveVersion <= 6;

        if (bSnapshotValid)
        {
            bSnapshotValid =
                SaveObject->CurrentChapterNumber >= 1 &&
                SaveObject->CurrentChapterNumber <= 20 &&
                SaveObject->ArinExperience >= 0 &&
                SaveObject->ArinCurrency >= 0 &&
                SaveObject->CompletedChapterNumbers.Num() <= 20;

            TSet<int32> UniqueCompletedChapters;
            for (const int32 CompletedChapter : SaveObject->CompletedChapterNumbers)
            {
                if (CompletedChapter < 1 || CompletedChapter > 20 ||
                    UniqueCompletedChapters.Contains(CompletedChapter))
                {
                    bSnapshotValid = false;
                    break;
                }

                UniqueCompletedChapters.Add(CompletedChapter);
            }
        }

        if (bSnapshotValid)
        {
            Snapshot.CurrentChapterNumber = SaveObject->CurrentChapterNumber;
            Snapshot.CompletedChapterCount = SaveObject->CompletedChapterNumbers.Num();
            Snapshot.ArinExperience = SaveObject->ArinExperience;
            Snapshot.ArinCurrency = SaveObject->ArinCurrency;
        }
        else
        {
            // Never expose malformed save metadata as a valid menu entry.
            Snapshot.bSaveExists = false;
        }
    }

    BP_OnSaveSlotSnapshotUpdated(Snapshot);
}
