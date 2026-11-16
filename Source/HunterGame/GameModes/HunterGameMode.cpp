#include "HunterGameMode.h"
#include "Characters/ArinValeCharacter.h"
#include "Campaign/HunterCampaignSubsystem.h"
#include "Engine/GameInstance.h"

AHunterGameMode::AHunterGameMode()
{
    DefaultPawnClass = AArinValeCharacter::StaticClass();
}

void AHunterGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Starting the configured map also initializes its campaign objective set.
    // StartChapter is safe for an already active/restored chapter and avoids
    // requiring a separate menu action when testing a map directly.
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UHunterCampaignSubsystem* Campaign =
            GameInstance->GetSubsystem<UHunterCampaignSubsystem>())
        {
            const int32 ChapterNumber = Campaign->GetCurrentChapterNumber();
            if (!Campaign->StartChapter(ChapterNumber))
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("HUNTER: Could not initialize campaign chapter %d."), ChapterNumber);
            }
        }
    }
}
