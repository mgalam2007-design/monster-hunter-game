#include "HunterCampaignSubsystem.h"

#include "ArinValeCharacter.h"
#include "ArinProgressionComponent.h"
#include "Components/HunterInventoryComponent.h"
#include "Components/HunterEquipmentComponent.h"
#include "Components/HunterCurrencyComponent.h"
#include "Quests/HunterQuestSubsystem.h"
#include "HunterSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

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

    // Chapter 1 is the first fully authored quest loop in the campaign.
    // Do not report a successful chapter start if its required quest cannot start.
    if (ChapterNumber == 1)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Preserve partial or completed progress when the player re-enters Chapter 1.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH01_FirstBreach")))
        {
            TArray<FHunterObjective> Objectives;

            FHunterObjective DefeatObjective;
            DefeatObjective.ObjectiveId = TEXT("ClearBreachCreatures");
            DefeatObjective.Description = FText::FromString(TEXT("Defeat 3 creatures inside the breach"));
            DefeatObjective.Type = EHunterObjectiveType::Defeat;
            DefeatObjective.EventTag = TEXT("MonsterDefeated");
            DefeatObjective.RequiredCount = 3;
            Objectives.Add(DefeatObjective);

            FHunterObjective RecoverObjective;
            RecoverObjective.ObjectiveId = TEXT("RecoverBreachShard");
            RecoverObjective.Description = FText::FromString(TEXT("Recover the breach shard"));
            RecoverObjective.Type = EHunterObjectiveType::Collect;
            RecoverObjective.EventTag = TEXT("BreachShardCollected");
            RecoverObjective.RequiredCount = 1;
            Objectives.Add(RecoverObjective);

            if (!Quest->StartQuest(TEXT("CH01_FirstBreach"), Objectives))
            {
                return false;
            }
        }
    }
    else if (ChapterNumber == 2)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Chapter 2: locate the survivor district, then rescue the missing civilians.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH02_MissingSurvivors")))
        {
            // A completed prior chapter quest must be cleared before the
            // next chapter can initialize its own objective set.
            if (Quest->IsQuestActive())
            {
                if (!Quest->IsQuestComplete())
                {
                    return false;
                }
                Quest->ClearActiveQuest();
            }

            TArray<FHunterObjective> Objectives;

            FHunterObjective LocateObjective;
            LocateObjective.ObjectiveId = TEXT("LocateSurvivorDistrict");
            LocateObjective.Description = FText::FromString(TEXT("Find the missing survivors' district"));
            LocateObjective.Type = EHunterObjectiveType::Reach;
            LocateObjective.EventTag = TEXT("SurvivorDistrictLocated");
            LocateObjective.RequiredCount = 1;
            Objectives.Add(LocateObjective);

            FHunterObjective RescueObjective;
            RescueObjective.ObjectiveId = TEXT("RescueMissingCivilians");
            RescueObjective.Description = FText::FromString(TEXT("Rescue 3 missing civilians"));
            RescueObjective.Type = EHunterObjectiveType::Interact;
            RescueObjective.EventTag = TEXT("SurvivorRescued");
            RescueObjective.RequiredCount = 3;
            Objectives.Add(RescueObjective);

            if (!Quest->StartQuest(TEXT("CH02_MissingSurvivors"), Objectives))
            {
                return false;
            }
        }
    }

    else if (ChapterNumber == 3)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Chapter 3: pass the guild's controlled combat evaluation.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH03_GuildEvaluation")))
        {
            if (Quest->IsQuestActive())
            {
                if (!Quest->IsQuestComplete())
                {
                    return false;
                }
                Quest->ClearActiveQuest();
            }

            TArray<FHunterObjective> Objectives;

            FHunterObjective DefeatObjective;
            DefeatObjective.ObjectiveId = TEXT("DefeatTrialOpponents");
            DefeatObjective.Description = FText::FromString(TEXT("Defeat 5 trial opponents"));
            DefeatObjective.Type = EHunterObjectiveType::Defeat;
            DefeatObjective.EventTag = TEXT("TrialOpponentDefeated");
            DefeatObjective.RequiredCount = 5;
            Objectives.Add(DefeatObjective);

            FHunterObjective SecureObjective;
            SecureObjective.ObjectiveId = TEXT("SecureTrialArena");
            SecureObjective.Description = FText::FromString(TEXT("Activate the trial arena's completion seal"));
            SecureObjective.Type = EHunterObjectiveType::Interact;
            SecureObjective.EventTag = TEXT("TrialSealActivated");
            SecureObjective.RequiredCount = 1;
            Objectives.Add(SecureObjective);

            if (!Quest->StartQuest(TEXT("CH03_GuildEvaluation"), Objectives))
            {
                return false;
            }
        }
    }

    else if (ChapterNumber == 4)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Chapter 4: investigate the abandoned transit gate and defeat its guardian.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH04_TransitGate")))
        {
            if (Quest->IsQuestActive())
            {
                if (!Quest->IsQuestComplete())
                {
                    return false;
                }
                Quest->ClearActiveQuest();
            }

            TArray<FHunterObjective> Objectives;

            FHunterObjective ReachObjective;
            ReachObjective.ObjectiveId = TEXT("ReachTransitGate");
            ReachObjective.Description = FText::FromString(TEXT("Reach the sealed underground gate"));
            ReachObjective.Type = EHunterObjectiveType::Reach;
            ReachObjective.EventTag = TEXT("TransitGateReached");
            ReachObjective.RequiredCount = 1;
            Objectives.Add(ReachObjective);

            FHunterObjective GuardianObjective;
            GuardianObjective.ObjectiveId = TEXT("DefeatTransitGuardian");
            GuardianObjective.Description = FText::FromString(TEXT("Defeat the transit gate guardian"));
            GuardianObjective.Type = EHunterObjectiveType::Defeat;
            GuardianObjective.EventTag = TEXT("TransitGuardianDefeated");
            GuardianObjective.RequiredCount = 1;
            Objectives.Add(GuardianObjective);

            if (!Quest->StartQuest(TEXT("CH04_TransitGate"), Objectives))
            {
                return false;
            }
        }
    }

    else if (ChapterNumber == 5)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Chapter 5: complete the mining-settlement guild contract.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH05_MiningContract")))
        {
            if (Quest->IsQuestActive())
            {
                if (!Quest->IsQuestComplete())
                {
                    return false;
                }
                Quest->ClearActiveQuest();
            }

            TArray<FHunterObjective> Objectives;

            FHunterObjective DefeatObjective;
            DefeatObjective.ObjectiveId = TEXT("ClearMiningSettlement");
            DefeatObjective.Description = FText::FromString(TEXT("Defeat 4 creatures in the mining settlement"));
            DefeatObjective.Type = EHunterObjectiveType::Defeat;
            DefeatObjective.EventTag = TEXT("MiningCreatureDefeated");
            DefeatObjective.RequiredCount = 4;
            Objectives.Add(DefeatObjective);

            FHunterObjective CollectObjective;
            CollectObjective.ObjectiveId = TEXT("RecoverOreSamples");
            CollectObjective.Description = FText::FromString(TEXT("Recover 3 corrupted ore samples"));
            CollectObjective.Type = EHunterObjectiveType::Collect;
            CollectObjective.EventTag = TEXT("CorruptedOreCollected");
            CollectObjective.RequiredCount = 3;
            Objectives.Add(CollectObjective);

            if (!Quest->StartQuest(TEXT("CH05_MiningContract"), Objectives))
            {
                return false;
            }
        }
    }

    else if (ChapterNumber == 6)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Chapter 6: complete the guild's advanced trial of strength and control.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH06_TrialOfSteel")))
        {
            if (Quest->IsQuestActive())
            {
                if (!Quest->IsQuestComplete())
                {
                    return false;
                }
                Quest->ClearActiveQuest();
            }

            TArray<FHunterObjective> Objectives;

            FHunterObjective DefeatObjective;
            DefeatObjective.ObjectiveId = TEXT("DefeatSteelTrialElite");
            DefeatObjective.Description = FText::FromString(TEXT("Defeat the elite opponent in the Trial of Steel"));
            DefeatObjective.Type = EHunterObjectiveType::Defeat;
            DefeatObjective.EventTag = TEXT("SteelTrialOpponentDefeated");
            DefeatObjective.RequiredCount = 1;
            Objectives.Add(DefeatObjective);

            FHunterObjective ApprovalObjective;
            ApprovalObjective.ObjectiveId = TEXT("EarnSteelTrialApproval");
            ApprovalObjective.Description = FText::FromString(TEXT("Activate the guild trial approval seal"));
            ApprovalObjective.Type = EHunterObjectiveType::Interact;
            ApprovalObjective.EventTag = TEXT("SteelTrialApproved");
            ApprovalObjective.RequiredCount = 1;
            Objectives.Add(ApprovalObjective);

            if (!Quest->StartQuest(TEXT("CH06_TrialOfSteel"), Objectives))
            {
                return false;
            }
        }
    }

    else if (ChapterNumber == 7)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Chapter 7: investigate the blue mark by locating and stabilizing its resonance.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH07_BlueMark")))
        {
            if (Quest->IsQuestActive())
            {
                if (!Quest->IsQuestComplete())
                {
                    return false;
                }
                Quest->ClearActiveQuest();
            }

            TArray<FHunterObjective> Objectives;

            FHunterObjective LocateObjective;
            LocateObjective.ObjectiveId = TEXT("LocateBlueMarkSource");
            LocateObjective.Description = FText::FromString(TEXT("Locate the source of Arin's blue mark"));
            LocateObjective.Type = EHunterObjectiveType::Reach;
            LocateObjective.EventTag = TEXT("BlueMarkSourceLocated");
            LocateObjective.RequiredCount = 1;
            Objectives.Add(LocateObjective);

            FHunterObjective StabilizeObjective;
            StabilizeObjective.ObjectiveId = TEXT("StabilizeResonanceFragments");
            StabilizeObjective.Description = FText::FromString(TEXT("Collect 3 resonance fragments to stabilize the mark"));
            StabilizeObjective.Type = EHunterObjectiveType::Collect;
            StabilizeObjective.EventTag = TEXT("ResonanceFragmentCollected");
            StabilizeObjective.RequiredCount = 3;
            Objectives.Add(StabilizeObjective);

            if (!Quest->StartQuest(TEXT("CH07_BlueMark"), Objectives))
            {
                return false;
            }
        }
    }

    else if (ChapterNumber == 8)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Chapter 8: survive the expedition ambush and complete Arin's first true awakening.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH08_HunterReborn")))
        {
            if (Quest->IsQuestActive())
            {
                if (!Quest->IsQuestComplete())
                {
                    return false;
                }
                Quest->ClearActiveQuest();
            }

            TArray<FHunterObjective> Objectives;

            FHunterObjective DefeatObjective;
            DefeatObjective.ObjectiveId = TEXT("DefeatAwakeningAmbushers");
            DefeatObjective.Description = FText::FromString(TEXT("Defeat 6 creatures in the awakening expedition"));
            DefeatObjective.Type = EHunterObjectiveType::Defeat;
            DefeatObjective.EventTag = TEXT("AwakeningCreatureDefeated");
            DefeatObjective.RequiredCount = 6;
            Objectives.Add(DefeatObjective);

            FHunterObjective ActivateObjective;
            ActivateObjective.ObjectiveId = TEXT("ActivateAwakeningAltar");
            ActivateObjective.Description = FText::FromString(TEXT("Activate the awakening altar"));
            ActivateObjective.Type = EHunterObjectiveType::Interact;
            ActivateObjective.EventTag = TEXT("AwakeningAltarActivated");
            ActivateObjective.RequiredCount = 1;
            Objectives.Add(ActivateObjective);

            if (!Quest->StartQuest(TEXT("CH08_HunterReborn"), Objectives))
            {
                return false;
            }
        }
    }

    else if (ChapterNumber == 9)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Chapter 9: trace the vanished hunters and recover the expedition's final signal record.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH09_VanishedHunters")))
        {
            if (Quest->IsQuestActive())
            {
                if (!Quest->IsQuestComplete())
                {
                    return false;
                }
                Quest->ClearActiveQuest();
            }

            TArray<FHunterObjective> Objectives;

            FHunterObjective LocateObjective;
            LocateObjective.ObjectiveId = TEXT("LocateMissingHunterSignal");
            LocateObjective.Description = FText::FromString(TEXT("Locate the last signal from the missing hunters"));
            LocateObjective.Type = EHunterObjectiveType::Reach;
            LocateObjective.EventTag = TEXT("MissingHunterSignalLocated");
            LocateObjective.RequiredCount = 1;
            Objectives.Add(LocateObjective);

            FHunterObjective DefeatObjective;
            DefeatObjective.ObjectiveId = TEXT("DefeatGateStalkers");
            DefeatObjective.Description = FText::FromString(TEXT("Defeat 4 gate stalkers guarding the signal site"));
            DefeatObjective.Type = EHunterObjectiveType::Defeat;
            DefeatObjective.EventTag = TEXT("GateStalkerDefeated");
            DefeatObjective.RequiredCount = 4;
            Objectives.Add(DefeatObjective);

            FHunterObjective RecoverObjective;
            RecoverObjective.ObjectiveId = TEXT("RecoverExpeditionRecorder");
            RecoverObjective.Description = FText::FromString(TEXT("Recover the missing expedition's recorder"));
            RecoverObjective.Type = EHunterObjectiveType::Collect;
            RecoverObjective.EventTag = TEXT("ExpeditionRecorderRecovered");
            RecoverObjective.RequiredCount = 1;
            Objectives.Add(RecoverObjective);

            if (!Quest->StartQuest(TEXT("CH09_VanishedHunters"), Objectives))
            {
                return false;
            }
        }
    }

    else if (ChapterNumber == 10)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Chapter 10: push into the hollow district, sever its anchors and defeat its guardian.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH10_HollowDistrict")))
        {
            if (Quest->IsQuestActive())
            {
                if (!Quest->IsQuestComplete())
                {
                    return false;
                }
                Quest->ClearActiveQuest();
            }

            TArray<FHunterObjective> Objectives;

            FHunterObjective ReachObjective;
            ReachObjective.ObjectiveId = TEXT("ReachHollowDistrictCore");
            ReachObjective.Description = FText::FromString(TEXT("Reach the center of the hollow district"));
            ReachObjective.Type = EHunterObjectiveType::Reach;
            ReachObjective.EventTag = TEXT("HollowDistrictCoreReached");
            ReachObjective.RequiredCount = 1;
            Objectives.Add(ReachObjective);

            FHunterObjective AnchorObjective;
            AnchorObjective.ObjectiveId = TEXT("DestroyShadowAnchors");
            AnchorObjective.Description = FText::FromString(TEXT("Destroy 3 shadow anchors"));
            AnchorObjective.Type = EHunterObjectiveType::Interact;
            AnchorObjective.EventTag = TEXT("ShadowAnchorDestroyed");
            AnchorObjective.RequiredCount = 3;
            Objectives.Add(AnchorObjective);

            FHunterObjective GuardianObjective;
            GuardianObjective.ObjectiveId = TEXT("DefeatHollowWarden");
            GuardianObjective.Description = FText::FromString(TEXT("Defeat the Hollow Warden"));
            GuardianObjective.Type = EHunterObjectiveType::Defeat;
            GuardianObjective.EventTag = TEXT("HollowWardenDefeated");
            GuardianObjective.RequiredCount = 1;
            Objectives.Add(GuardianObjective);

            if (!Quest->StartQuest(TEXT("CH10_HollowDistrict"), Objectives))
            {
                return false;
            }
        }
    }

    else if (ChapterNumber == 11)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Chapter 11: recover the memories of the lost expedition and confront its fallen captain.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH11_EchoesOfTheFallen")))
        {
            if (Quest->IsQuestActive())
            {
                if (!Quest->IsQuestComplete())
                {
                    return false;
                }
                Quest->ClearActiveQuest();
            }

            TArray<FHunterObjective> Objectives;

            FHunterObjective MemoryObjective;
            MemoryObjective.ObjectiveId = TEXT("RecoverExpeditionMemories");
            MemoryObjective.Description = FText::FromString(TEXT("Recover 3 memory echoes from the lost expedition"));
            MemoryObjective.Type = EHunterObjectiveType::Collect;
            MemoryObjective.EventTag = TEXT("ExpeditionMemoryRecovered");
            MemoryObjective.RequiredCount = 3;
            Objectives.Add(MemoryObjective);

            FHunterObjective CaptainObjective;
            CaptainObjective.ObjectiveId = TEXT("DefeatFallenCaptain");
            CaptainObjective.Description = FText::FromString(TEXT("Defeat the fallen expedition captain"));
            CaptainObjective.Type = EHunterObjectiveType::Defeat;
            CaptainObjective.EventTag = TEXT("FallenCaptainDefeated");
            CaptainObjective.RequiredCount = 1;
            Objectives.Add(CaptainObjective);

            if (!Quest->StartQuest(TEXT("CH11_EchoesOfTheFallen"), Objectives))
            {
                return false;
            }
        }
    }

    else if (ChapterNumber == 12)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Chapter 12: investigate the unknown faction and make a deliberate choice about its pact.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH12_ShadowAccord")))
        {
            if (Quest->IsQuestActive())
            {
                if (!Quest->IsQuestComplete())
                {
                    return false;
                }
                Quest->ClearActiveQuest();
            }

            TArray<FHunterObjective> Objectives;

            FHunterObjective ContactObjective;
            ContactObjective.ObjectiveId = TEXT("ReachShadowContact");
            ContactObjective.Description = FText::FromString(TEXT("Meet the unknown faction's representative"));
            ContactObjective.Type = EHunterObjectiveType::Reach;
            ContactObjective.EventTag = TEXT("ShadowContactReached");
            ContactObjective.RequiredCount = 1;
            Objectives.Add(ContactObjective);

            FHunterObjective DefeatObjective;
            DefeatObjective.ObjectiveId = TEXT("DefeatShadowSentinels");
            DefeatObjective.Description = FText::FromString(TEXT("Defeat 3 shadow sentinels guarding the meeting"));
            DefeatObjective.Type = EHunterObjectiveType::Defeat;
            DefeatObjective.EventTag = TEXT("ShadowSentinelDefeated");
            DefeatObjective.RequiredCount = 3;
            Objectives.Add(DefeatObjective);

            FHunterObjective PactObjective;
            PactObjective.ObjectiveId = TEXT("ResolveShadowPact");
            PactObjective.Description = FText::FromString(TEXT("Make a decision at the shadow pact altar"));
            PactObjective.Type = EHunterObjectiveType::Interact;
            PactObjective.EventTag = TEXT("ShadowPactResolved");
            PactObjective.RequiredCount = 1;
            Objectives.Add(PactObjective);

            if (!Quest->StartQuest(TEXT("CH12_ShadowAccord"), Objectives))
            {
                return false;
            }
        }
    }

    else if (ChapterNumber == 13)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Chapter 13: defend the northern fortress and restore its defensive beacons.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH13_SiegeOfTheNorth")))
        {
            if (Quest->IsQuestActive())
            {
                if (!Quest->IsQuestComplete())
                {
                    return false;
                }
                Quest->ClearActiveQuest();
            }

            TArray<FHunterObjective> Objectives;

            FHunterObjective DefeatObjective;
            DefeatObjective.ObjectiveId = TEXT("RepelNorthernSiege");
            DefeatObjective.Description = FText::FromString(TEXT("Defeat 5 creatures attacking the northern fortress"));
            DefeatObjective.Type = EHunterObjectiveType::Defeat;
            DefeatObjective.EventTag = TEXT("NorthernSiegeCreatureDefeated");
            DefeatObjective.RequiredCount = 5;
            Objectives.Add(DefeatObjective);

            FHunterObjective BeaconObjective;
            BeaconObjective.ObjectiveId = TEXT("RestoreFortressBeacons");
            BeaconObjective.Description = FText::FromString(TEXT("Reactivate 2 fortress defense beacons"));
            BeaconObjective.Type = EHunterObjectiveType::Interact;
            BeaconObjective.EventTag = TEXT("FortressBeaconActivated");
            BeaconObjective.RequiredCount = 2;
            Objectives.Add(BeaconObjective);

            if (!Quest->StartQuest(TEXT("CH13_SiegeOfTheNorth"), Objectives))
            {
                return false;
            }
        }
    }

    else if (ChapterNumber == 14)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Chapter 14: break the citadel siege by destroying its command pylons and defeating its commander.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH14_BrokenCitadel")))
        {
            if (Quest->IsQuestActive())
            {
                if (!Quest->IsQuestComplete())
                {
                    return false;
                }
                Quest->ClearActiveQuest();
            }

            TArray<FHunterObjective> Objectives;

            FHunterObjective PylonObjective;
            PylonObjective.ObjectiveId = TEXT("DestroyCitadelPylons");
            PylonObjective.Description = FText::FromString(TEXT("Destroy 3 citadel command pylons"));
            PylonObjective.Type = EHunterObjectiveType::Interact;
            PylonObjective.EventTag = TEXT("CitadelPylonDestroyed");
            PylonObjective.RequiredCount = 3;
            Objectives.Add(PylonObjective);

            FHunterObjective CommanderObjective;
            CommanderObjective.ObjectiveId = TEXT("DefeatCitadelCommander");
            CommanderObjective.Description = FText::FromString(TEXT("Defeat the citadel siege commander"));
            CommanderObjective.Type = EHunterObjectiveType::Defeat;
            CommanderObjective.EventTag = TEXT("CitadelCommanderDefeated");
            CommanderObjective.RequiredCount = 1;
            Objectives.Add(CommanderObjective);

            if (!Quest->StartQuest(TEXT("CH14_BrokenCitadel"), Objectives))
            {
                return false;
            }
        }
    }

    else if (ChapterNumber == 15)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Chapter 15: close simultaneous breaches by stabilizing their anchors.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH15_WorldsUnsealed")))
        {
            if (Quest->IsQuestActive())
            {
                if (!Quest->IsQuestComplete())
                {
                    return false;
                }
                Quest->ClearActiveQuest();
            }

            TArray<FHunterObjective> Objectives;

            FHunterObjective AnchorObjective;
            AnchorObjective.ObjectiveId = TEXT("StabilizeBreachAnchors");
            AnchorObjective.Description = FText::FromString(TEXT("Stabilize 4 unstable gate anchors"));
            AnchorObjective.Type = EHunterObjectiveType::Interact;
            AnchorObjective.EventTag = TEXT("BreachAnchorStabilized");
            AnchorObjective.RequiredCount = 4;
            Objectives.Add(AnchorObjective);

            FHunterObjective GuardianObjective;
            GuardianObjective.ObjectiveId = TEXT("DefeatBreachGuardians");
            GuardianObjective.Description = FText::FromString(TEXT("Defeat 2 guardians protecting the unstable gates"));
            GuardianObjective.Type = EHunterObjectiveType::Defeat;
            GuardianObjective.EventTag = TEXT("BreachGuardianDefeated");
            GuardianObjective.RequiredCount = 2;
            Objectives.Add(GuardianObjective);

            if (!Quest->StartQuest(TEXT("CH15_WorldsUnsealed"), Objectives))
            {
                return false;
            }
        }
    }

    else if (ChapterNumber == 16)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Chapter 16: reactivate the world-boundary beacons and defend them from attackers.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH16_SevenBeacons")))
        {
            if (Quest->IsQuestActive())
            {
                if (!Quest->IsQuestComplete())
                {
                    return false;
                }
                Quest->ClearActiveQuest();
            }

            TArray<FHunterObjective> Objectives;

            FHunterObjective BeaconObjective;
            BeaconObjective.ObjectiveId = TEXT("ActivateBoundaryBeacons");
            BeaconObjective.Description = FText::FromString(TEXT("Activate 7 boundary beacons"));
            BeaconObjective.Type = EHunterObjectiveType::Interact;
            BeaconObjective.EventTag = TEXT("BoundaryBeaconActivated");
            BeaconObjective.RequiredCount = 7;
            Objectives.Add(BeaconObjective);

            FHunterObjective DefenseObjective;
            DefenseObjective.ObjectiveId = TEXT("DefendBeaconKeepers");
            DefenseObjective.Description = FText::FromString(TEXT("Defeat 3 creatures attacking the beacon network"));
            DefenseObjective.Type = EHunterObjectiveType::Defeat;
            DefenseObjective.EventTag = TEXT("BeaconAttackerDefeated");
            DefenseObjective.RequiredCount = 3;
            Objectives.Add(DefenseObjective);

            if (!Quest->StartQuest(TEXT("CH16_SevenBeacons"), Objectives))
            {
                return false;
            }
        }
    }

    else if (ChapterNumber == 17)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Chapter 17: hold the last sanctuary while the remaining civilians evacuate.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH17_LastSanctuary")))
        {
            if (Quest->IsQuestActive())
            {
                if (!Quest->IsQuestComplete())
                {
                    return false;
                }
                Quest->ClearActiveQuest();
            }

            TArray<FHunterObjective> Objectives;

            FHunterObjective RescueObjective;
            RescueObjective.ObjectiveId = TEXT("EvacuateSanctuaryCivilians");
            RescueObjective.Description = FText::FromString(TEXT("Escort 5 civilian groups to safety"));
            RescueObjective.Type = EHunterObjectiveType::Interact;
            RescueObjective.EventTag = TEXT("SanctuaryGroupEvacuated");
            RescueObjective.RequiredCount = 5;
            Objectives.Add(RescueObjective);

            FHunterObjective DefendObjective;
            DefendObjective.ObjectiveId = TEXT("DefeatSanctuaryInvaders");
            DefendObjective.Description = FText::FromString(TEXT("Defeat 6 creatures attacking the sanctuary"));
            DefendObjective.Type = EHunterObjectiveType::Defeat;
            DefendObjective.EventTag = TEXT("SanctuaryInvaderDefeated");
            DefendObjective.RequiredCount = 6;
            Objectives.Add(DefendObjective);

            if (!Quest->StartQuest(TEXT("CH17_LastSanctuary"), Objectives))
            {
                return false;
            }
        }
    }


    else if (ChapterNumber == 18)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Chapter 18: break the ruined throne-city's command network.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH18_CrownOfRuin")))
        {
            if (Quest->IsQuestActive())
            {
                if (!Quest->IsQuestComplete())
                {
                    return false;
                }
                Quest->ClearActiveQuest();
            }

            TArray<FHunterObjective> Objectives;

            FHunterObjective WardenObjective;
            WardenObjective.ObjectiveId = TEXT("DefeatThroneWardens");
            WardenObjective.Description = FText::FromString(TEXT("Defeat 4 wardens guarding the ruined throne"));
            WardenObjective.Type = EHunterObjectiveType::Defeat;
            WardenObjective.EventTag = TEXT("ThroneWardenDefeated");
            WardenObjective.RequiredCount = 4;
            Objectives.Add(WardenObjective);

            FHunterObjective CoreObjective;
            CoreObjective.ObjectiveId = TEXT("DestroyRuinCores");
            CoreObjective.Description = FText::FromString(TEXT("Destroy 3 command cores beneath the throne-city"));
            CoreObjective.Type = EHunterObjectiveType::Interact;
            CoreObjective.EventTag = TEXT("RuinCommandCoreDestroyed");
            CoreObjective.RequiredCount = 3;
            Objectives.Add(CoreObjective);

            if (!Quest->StartQuest(TEXT("CH18_CrownOfRuin"), Objectives))
            {
                return false;
            }
        }
    }
    else if (ChapterNumber == 19)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Chapter 19: cross the origin gate and disable its guardian network.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH19_BeyondTheGate")))
        {
            if (Quest->IsQuestActive())
            {
                if (!Quest->IsQuestComplete())
                {
                    return false;
                }
                Quest->ClearActiveQuest();
            }

            TArray<FHunterObjective> Objectives;

            FHunterObjective SealObjective;
            SealObjective.ObjectiveId = TEXT("StabilizeOriginSeals");
            SealObjective.Description = FText::FromString(TEXT("Stabilize 3 seals along the origin gate"));
            SealObjective.Type = EHunterObjectiveType::Interact;
            SealObjective.EventTag = TEXT("OriginGateSealStabilized");
            SealObjective.RequiredCount = 3;
            Objectives.Add(SealObjective);

            FHunterObjective SentinelObjective;
            SentinelObjective.ObjectiveId = TEXT("DefeatOriginSentinels");
            SentinelObjective.Description = FText::FromString(TEXT("Defeat 2 sentinels guarding the origin gate"));
            SentinelObjective.Type = EHunterObjectiveType::Defeat;
            SentinelObjective.EventTag = TEXT("OriginSentinelDefeated");
            SentinelObjective.RequiredCount = 2;
            Objectives.Add(SentinelObjective);

            if (!Quest->StartQuest(TEXT("CH19_BeyondTheGate"), Objectives))
            {
                return false;
            }
        }
    }
    else if (ChapterNumber == 20)
    {
        UGameInstance* GameInstance = GetGameInstance();
        UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest)
        {
            return false;
        }

        // Chapter 20: expose and defeat the source of the breach network.
        if (Quest->GetActiveQuestId() != FName(TEXT("CH20_TheFinalHunter")))
        {
            if (Quest->IsQuestActive())
            {
                if (!Quest->IsQuestComplete())
                {
                    return false;
                }
                Quest->ClearActiveQuest();
            }

            TArray<FHunterObjective> Objectives;

            FHunterObjective HeartObjective;
            HeartObjective.ObjectiveId = TEXT("ExposeOriginHeart");
            HeartObjective.Description = FText::FromString(TEXT("Disable the final seal protecting the origin heart"));
            HeartObjective.Type = EHunterObjectiveType::Interact;
            HeartObjective.EventTag = TEXT("OriginHeartExposed");
            HeartObjective.RequiredCount = 1;
            Objectives.Add(HeartObjective);

            FHunterObjective FinalBossObjective;
            FinalBossObjective.ObjectiveId = TEXT("DefeatFinalHunter");
            FinalBossObjective.Description = FText::FromString(TEXT("Defeat the source of the invasion"));
            FinalBossObjective.Type = EHunterObjectiveType::Defeat;
            FinalBossObjective.EventTag = TEXT("FinalHunterDefeated");
            FinalBossObjective.RequiredCount = 1;
            Objectives.Add(FinalBossObjective);

            if (!Quest->StartQuest(TEXT("CH20_TheFinalHunter"), Objectives))
            {
                return false;
            }
        }
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

    // Chapter 1 cannot be skipped if its quest is missing, inactive or incomplete.
    if (CurrentChapterNumber == 1)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH01_FirstBreach")) || !Quest->IsQuestComplete())
        {
            return false;
        }
    }
    else if (CurrentChapterNumber == 2)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH02_MissingSurvivors")) || !Quest->IsQuestComplete())
        {
            return false;
        }
    }
    else if (CurrentChapterNumber == 3)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH03_GuildEvaluation")) || !Quest->IsQuestComplete())
        {
            return false;
        }
    }
    else if (CurrentChapterNumber == 4)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH04_TransitGate")) || !Quest->IsQuestComplete())
        {
            return false;
        }
    }
    else if (CurrentChapterNumber == 5)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH05_MiningContract")) || !Quest->IsQuestComplete())
        {
            return false;
        }
    }
    else if (CurrentChapterNumber == 6)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH06_TrialOfSteel")) || !Quest->IsQuestComplete())
        {
            return false;
        }
    }
    else if (CurrentChapterNumber == 7)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH07_BlueMark")) || !Quest->IsQuestComplete())
        {
            return false;
        }
    }
    else if (CurrentChapterNumber == 8)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH08_HunterReborn")) || !Quest->IsQuestComplete())
        {
            return false;
        }
    }
    else if (CurrentChapterNumber == 9)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH09_VanishedHunters")) || !Quest->IsQuestComplete())
        {
            return false;
        }
    }
    else if (CurrentChapterNumber == 10)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH10_HollowDistrict")) || !Quest->IsQuestComplete())
        {
            return false;
        }
    }
    else if (CurrentChapterNumber == 11)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH11_EchoesOfTheFallen")) || !Quest->IsQuestComplete())
        {
            return false;
        }
    }
    else if (CurrentChapterNumber == 12)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH12_ShadowAccord")) || !Quest->IsQuestComplete())
        {
            return false;
        }
    }
    else if (CurrentChapterNumber == 13)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH13_SiegeOfTheNorth")) || !Quest->IsQuestComplete())
        {
            return false;
        }
    }
    else if (CurrentChapterNumber == 14)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH14_BrokenCitadel")) || !Quest->IsQuestComplete())
        {
            return false;
        }
    }
    else if (CurrentChapterNumber == 15)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH15_WorldsUnsealed")) || !Quest->IsQuestComplete())
        {
            return false;
        }
    }
    else if (CurrentChapterNumber == 16)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH16_SevenBeacons")) || !Quest->IsQuestComplete())
        {
            return false;
        }
    }
    else if (CurrentChapterNumber == 17)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH17_LastSanctuary")) || !Quest->IsQuestComplete())
        {
            return false;
        }
    }
    else if (CurrentChapterNumber == 18)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH18_CrownOfRuin")) || !Quest->IsQuestComplete())
        {
            return false;
        }
    }
    else if (CurrentChapterNumber == 19)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH19_BeyondTheGate")) || !Quest->IsQuestComplete())
        {
            return false;
        }
    }
    else if (CurrentChapterNumber == 20)
    {
        UGameInstance* GameInstance = GetGameInstance();
        const UHunterQuestSubsystem* Quest = GameInstance
            ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
            : nullptr;
        if (!Quest || Quest->GetActiveQuestId() != FName(TEXT("CH20_TheFinalHunter")) || !Quest->IsQuestComplete())
        {
            return false;
        }
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
    else if (IsCampaignComplete())
    {
        OnCampaignCompleted.Broadcast();
    }

    return true;
}

bool UHunterCampaignSubsystem::SaveCampaign(AArinValeCharacter* Arin, const FString& SlotName, int32 UserIndex)
{
    if (!Arin || SlotName.IsEmpty() ||
        CurrentChapterNumber < 1 || CurrentChapterNumber > Chapters.Num())
    {
        return false;
    }

    // Never write a checkpoint that would later be rejected or teleport Arin
    // to an invalid transform during respawn/load.
    const FTransform ArinTransform = Arin->GetActorTransform();
    if (!ArinTransform.IsValid())
    {
        return false;
    }

    UHunterSaveGame* SaveObject = Cast<UHunterSaveGame>(UGameplayStatics::CreateSaveGameObject(UHunterSaveGame::StaticClass()));
    if (!SaveObject)
    {
        return false;
    }

    SaveObject->CurrentChapterNumber = CurrentChapterNumber;
    SaveObject->ArinWorldTransform = ArinTransform;
    SaveObject->bHasArinWorldTransform = true;
    SaveObject->CompletedChapterNumbers.Reset();

    for (const FHunterChapter& Chapter : Chapters)
    {
        if (Chapter.bCompleted)
        {
            SaveObject->CompletedChapterNumbers.Add(Chapter.ChapterNumber);
        }
    }

    if (const UHunterInventoryComponent* Inventory = Arin->GetInventoryComponent())
    {
        SaveObject->ArinInventoryItems = Inventory->GetItems();
    }

    if (const UHunterEquipmentComponent* Equipment = Arin->GetEquipmentComponent())
    {
        SaveObject->ArinEquippedWeapon = Equipment->GetWeapon();
        SaveObject->ArinEquippedArmor = Equipment->GetArmor();
    }

    if (const UHunterCurrencyComponent* Currency = Arin->GetCurrencyComponent())
    {
        SaveObject->ArinCurrency = Currency->GetBalance();
    }

    UGameInstance* GameInstance = GetGameInstance();
    if (const UHunterQuestSubsystem* Quest = GameInstance
        ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
        : nullptr)
    {
        SaveObject->ActiveQuestId = Quest->GetActiveQuestId();
        SaveObject->ActiveQuestObjectives = Quest->GetObjectives();
        SaveObject->bActiveQuestCompleted = Quest->IsQuestComplete();
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
    if (!SaveObject || SaveObject->SaveVersion < 1 || SaveObject->SaveVersion > 6)
    {
        return false;
    }

    // Reject malformed snapshots before changing campaign or player state.
    // The menu performs similar checks, but loads can also be requested directly.
    if (SaveObject->CurrentChapterNumber < 1 ||
        SaveObject->CurrentChapterNumber > Chapters.Num() ||
        SaveObject->ArinExperience < 0 ||
        SaveObject->ArinCurrency < 0 ||
        SaveObject->ArinEvolutionStage < 0 ||
        SaveObject->ArinEvolutionStage > 4 ||
        SaveObject->CompletedChapterNumbers.Num() > Chapters.Num())
    {
        return false;
    }

    TSet<int32> UniqueCompletedChapters;
    for (const int32 ChapterNumber : SaveObject->CompletedChapterNumbers)
    {
        if (ChapterNumber < 1 ||
            ChapterNumber > Chapters.Num() ||
            UniqueCompletedChapters.Contains(ChapterNumber))
        {
            return false;
        }
        UniqueCompletedChapters.Add(ChapterNumber);
    }

    TSet<int32> ValidCompletedChapters;
    for (const int32 ChapterNumber : SaveObject->CompletedChapterNumbers)
    {
        if (ChapterNumber >= 1 && ChapterNumber <= Chapters.Num())
        {
            ValidCompletedChapters.Add(ChapterNumber);
        }
    }

    bool bPreviousChapterCompleted = true;
    for (FHunterChapter& Chapter : Chapters)
    {
        // Campaign progress is strictly sequential: a later saved completion
        // cannot skip an unfinished chapter or unlock content out of order.
        Chapter.bCompleted = bPreviousChapterCompleted &&
            ValidCompletedChapters.Contains(Chapter.ChapterNumber);
        Chapter.bUnlocked = Chapter.ChapterNumber == 1 || bPreviousChapterCompleted;
        bPreviousChapterCompleted = Chapter.bCompleted;
    }

    int32 FirstIncompleteChapter = Chapters.Num();
    bool bFoundIncompleteChapter = false;
    for (const FHunterChapter& Chapter : Chapters)
    {
        if (!Chapter.bCompleted && Chapter.bUnlocked)
        {
            FirstIncompleteChapter = Chapter.ChapterNumber;
            bFoundIncompleteChapter = true;
            break;
        }
    }

    // Resume at a playable chapter, never on a chapter already marked complete.
    // A fully completed campaign remains parked on its final chapter for the UI.
    const int32 ResumeLimit = bFoundIncompleteChapter ? FirstIncompleteChapter : Chapters.Num();
    CurrentChapterNumber = FMath::Clamp(SaveObject->CurrentChapterNumber, 1, FMath::Max(1, ResumeLimit));
    if (bFoundIncompleteChapter && Chapters.IsValidIndex(CurrentChapterNumber - 1) && Chapters[CurrentChapterNumber - 1].bCompleted)
    {
        CurrentChapterNumber = FirstIncompleteChapter;
    }

    if (UHunterInventoryComponent* Inventory = Arin->GetInventoryComponent())
    {
        Inventory->RestoreItems(SaveObject->ArinInventoryItems);
    }

    if (UArinProgressionComponent* Progression = Arin->GetProgressionComponent())
    {
        const int32 SafeStage = FMath::Clamp(SaveObject->ArinEvolutionStage, 0, 4);
        Progression->RestoreProgression(
            FMath::Max(0, SaveObject->ArinExperience),
            static_cast<EArinEvolutionStage>(SafeStage));
    }

    if (UHunterEquipmentComponent* Equipment = Arin->GetEquipmentComponent())
    {
        Equipment->RestoreEquipment(SaveObject->ArinEquippedWeapon, SaveObject->ArinEquippedArmor);
    }

    if (UHunterCurrencyComponent* Currency = Arin->GetCurrencyComponent())
    {
        Currency->SetBalance(SaveObject->SaveVersion >= 5 ? SaveObject->ArinCurrency : 0);
    }

    UGameInstance* GameInstance = GetGameInstance();
    if (UHunterQuestSubsystem* Quest = GameInstance
        ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
        : nullptr)
    {
        if (!Quest->RestoreQuestState(SaveObject->ActiveQuestId, SaveObject->ActiveQuestObjectives, SaveObject->bActiveQuestCompleted))
        {
            Quest->ClearActiveQuest();
        }
    }

    // A corrupted save must never move Arin to a NaN/invalid world transform.
    if (SaveObject->SaveVersion >= 6 &&
        SaveObject->bHasArinWorldTransform &&
        SaveObject->ArinWorldTransform.IsValid())
    {
        Arin->SetActorTransform(SaveObject->ArinWorldTransform, false, nullptr, ETeleportType::TeleportPhysics);
    }

    OnChapterChanged.Broadcast(CurrentChapterNumber);
    return true;
}

bool UHunterCampaignSubsystem::IsCampaignComplete() const
{
    if (Chapters.IsEmpty())
    {
        return false;
    }

    for (const FHunterChapter& Chapter : Chapters)
    {
        if (!Chapter.bCompleted)
        {
            return false;
        }
    }
    return true;
}

FHunterChapter UHunterCampaignSubsystem::GetCurrentChapter() const
{
    const int32 Index = CurrentChapterNumber - 1;
    return Chapters.IsValidIndex(Index) ? Chapters[Index] : FHunterChapter();
}
