#include "HunterCampaignSubsystem.h"

#include "ArinValeCharacter.h"
#include "ArinProgressionComponent.h"
#include "HunterSaveGame.h"
#include "Kismet/GameplayStatics.h"

UHunterCampaignSubsystem::UHunterCampaignSubsystem()
{
}

void UHunterCampaignSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    BuildDefaultCampaign();
}

void UHunterCampaignSubsystem::BuildDefaultCampaign()
{
    static const TCHAR* Titles[20] = {
        TEXT("The First Breach"), TEXT("Ashes at Dawn"), TEXT("The Lowest Rank"), TEXT("A Door Below"),
        TEXT("The Silent Contract"), TEXT("Trial of Steel"), TEXT("The Blue Mark"), TEXT("A Hunter Reborn"),
        TEXT("Whispers in the Dark"), TEXT("The Hollow District"), TEXT("Echoes of the Fallen"), TEXT("The Shadow Accord"),
        TEXT("Siege of the North"), TEXT("The Broken Citadel"), TEXT("A World Unsealed"), TEXT("The Seven Beacons"),
        TEXT("The Last Sanctuary"), TEXT("Crown of Ruin"), TEXT("Beyond the Gate"), TEXT("The Final Hunter")
    };

    static const TCHAR* Briefings[20] = {
        TEXT("A minor breach opens beneath the city. Arin enters as an unranked recruit and discovers that the gate is not empty."),
        TEXT("Survivors are missing in the district around the first breach. Track the signs and bring them home."),
        TEXT("The guild evaluates Arin after the incident. Survive a controlled trial against stronger opponents."),
        TEXT("A sealed underground gate appears beneath an abandoned transit station. Find its source."),
        TEXT("A guild contract leads to a remote mining settlement where the creatures behave with unnatural discipline."),
        TEXT("Enter the guild's combat trial and prove control over the power awakened inside you."),
        TEXT("A blue mark appears after the trial. Investigate its connection to the gates."),
        TEXT("A near-fatal expedition forces Arin to accept the first true evolution of his power."),
        TEXT("Reports describe hunters vanishing inside gates that should already be cleared."),
        TEXT("Explore a district consumed by a spreading shadow phenomenon and locate its anchor."),
        TEXT("Recover the memories left behind by a lost expedition and uncover the cost of shadow power."),
        TEXT("An unknown faction offers a pact. Decide whether its information is worth the risk."),
        TEXT("A northern fortress is under siege by creatures emerging from a chained gate."),
        TEXT("Break the siege and confront the commander controlling the invading monsters."),
        TEXT("Multiple gates open at once. Travel between unstable zones and close their anchors."),
        TEXT("Activate the beacons that hold the boundary between worlds together."),
        TEXT("Defend the final sanctuary while the guild evacuates the remaining civilians."),
        TEXT("Enter the ruined throne-city and defeat the power gathering the gate armies."),
        TEXT("Cross the origin gate and learn who created the breach network."),
        TEXT("Face the source of the invasion and decide the fate of both worlds.")
    };

    static const int32 StageRequirements[20] = {
        0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4
    };

    Chapters.Reset();
    Chapters.Reserve(20);

    for (int32 Index = 0; Index < 20; ++Index)
    {
        FHunterChapter Chapter;
        Chapter.ChapterNumber = Index + 1;
        Chapter.Title = FText::FromString(Titles[Index]);
        Chapter.Briefing = FText::FromString(Briefings[Index]);
        Chapter.RecommendedEvolutionStage = StageRequirements[Index];
        Chapter.bUnlocked = Index == 0;
        Chapters.Add(Chapter);
    }

    CurrentChapterNumber = 1;
}

bool UHunterCampaignSubsystem::StartChapter(int32 ChapterNumber)
{
    const int32 Index = ChapterNumber - 1;
    if (!Chapters.IsValidIndex(Index) || !Chapters[Index].bUnlocked || Chapters[Index].bCompleted)
    {
        return false;
    }

    CurrentChapterNumber = ChapterNumber;
    OnChapterChanged.Broadcast(CurrentChapterNumber);
    return true;
}

bool UHunterCampaignSubsystem::CompleteCurrentChapter()
{
    const int32 Index = CurrentChapterNumber - 1;
    if (!Chapters.IsValidIndex(Index) || Chapters[Index].bCompleted)
    {
        return false;
    }

    Chapters[Index].bCompleted = true;
    OnChapterCompleted.Broadcast(CurrentChapterNumber);

    const int32 NextIndex = Index + 1;
    if (Chapters.IsValidIndex(NextIndex))
    {
        Chapters[NextIndex].bUnlocked = true;
        CurrentChapterNumber = NextIndex + 1;
        OnChapterChanged.Broadcast(CurrentChapterNumber);
    }

    return true;
}

bool UHunterCampaignSubsystem::SaveCampaign(AArinValeCharacter* Arin, const FString& SlotName, int32 UserIndex)
{
    if (!Arin || SlotName.IsEmpty())
    {
        return false;
    }

    UHunterSaveGame* SaveObject = Cast<UHunterSaveGame>(UGameplayStatics::CreateSaveGameObject(UHunterSaveGame::StaticClass()));
    if (!SaveObject)
    {
        return false;
    }

    SaveObject->CurrentChapterNumber = CurrentChapterNumber;
    SaveObject->CompletedChapterNumbers.Reset();

    for (const FHunterChapter& Chapter : Chapters)
    {
        if (Chapter.bCompleted)
        {
            SaveObject->CompletedChapterNumbers.Add(Chapter.ChapterNumber);
        }
    }

    if (const UArinProgressionComponent* Progression = Arin->GetProgressionComponent())
    {
        SaveObject->ArinExperience = Progression->GetExperience();
        SaveObject->ArinEvolutionStage = static_cast<int32>(Progression->GetEvolutionStage());
    }

    return UGameplayStatics::SaveGameToSlot(SaveObject, SlotName, UserIndex);
}

bool UHunterCampaignSubsystem::LoadCampaign(AArinValeCharacter* Arin, const FString& SlotName, int32 UserIndex)
{
    if (!Arin || SlotName.IsEmpty() || !UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
    {
        return false;
    }

    UHunterSaveGame* SaveObject = Cast<UHunterSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
    if (!SaveObject || SaveObject->SaveVersion != 1)
    {
        return false;
    }

    for (FHunterChapter& Chapter : Chapters)
    {
        Chapter.bCompleted = SaveObject->CompletedChapterNumbers.Contains(Chapter.ChapterNumber);
        Chapter.bUnlocked = Chapter.ChapterNumber == 1 || Chapter.bCompleted ||
            SaveObject->CompletedChapterNumbers.Contains(Chapter.ChapterNumber - 1);
    }

    CurrentChapterNumber = FMath::Clamp(SaveObject->CurrentChapterNumber, 1, Chapters.Num());

    if (UArinProgressionComponent* Progression = Arin->GetProgressionComponent())
    {
        const int32 SafeStage = FMath::Clamp(SaveObject->ArinEvolutionStage, 0, 4);
        Progression->RestoreProgression(
            FMath::Max(0, SaveObject->ArinExperience),
            static_cast<EArinEvolutionStage>(SafeStage));
    }

    OnChapterChanged.Broadcast(CurrentChapterNumber);
    return true;
}

FHunterChapter UHunterCampaignSubsystem::GetCurrentChapter() const
{
    const int32 Index = CurrentChapterNumber - 1;
    return Chapters.IsValidIndex(Index) ? Chapters[Index] : FHunterChapter();
}
