#!/usr/bin/env python3
"""Static consistency checks for the authored HUNTER 2.0 campaign quests.

This does not compile Unreal C++ or replace an in-editor playthrough.
"""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
CAMPAIGN_CPP = ROOT / "Source/HunterGame/Campaign/HunterCampaignSubsystem.cpp"
GUIDE = ROOT / "Docs/CAMPAIGN_LEVEL_INTEGRATION.md"

MISSIONS = {
    1: ("CH01_FirstBreach", ("MonsterDefeated", "BreachShardCollected")),
    2: ("CH02_MissingSurvivors", ("SurvivorDistrictLocated", "SurvivorRescued")),
    3: ("CH03_GuildEvaluation", ("TrialOpponentDefeated", "TrialSealActivated")),
    4: ("CH04_TransitGate", ("TransitGateReached", "TransitGuardianDefeated")),
    5: ("CH05_MiningContract", ("MiningCreatureDefeated", "CorruptedOreCollected")),
    6: ("CH06_TrialOfSteel", ("SteelTrialOpponentDefeated", "SteelTrialApproved")),
    7: ("CH07_BlueMark", ("BlueMarkSourceLocated", "ResonanceFragmentCollected")),
    8: ("CH08_HunterReborn", ("AwakeningCreatureDefeated", "AwakeningAltarActivated")),
    9: ("CH09_VanishedHunters", ("MissingHunterSignalLocated", "GateStalkerDefeated", "ExpeditionRecorderRecovered")),
    10: ("CH10_HollowDistrict", ("HollowDistrictCoreReached", "ShadowAnchorDestroyed", "HollowWardenDefeated")),
    11: ("CH11_EchoesOfTheFallen", ("ExpeditionMemoryRecovered", "FallenCaptainDefeated")),
    12: ("CH12_ShadowAccord", ("ShadowContactReached", "ShadowSentinelDefeated", "ShadowPactResolved")),
    13: ("CH13_SiegeOfTheNorth", ("NorthernSiegeCreatureDefeated", "FortressBeaconActivated")),
    14: ("CH14_BrokenCitadel", ("CitadelPylonDestroyed", "CitadelCommanderDefeated")),
    15: ("CH15_WorldsUnsealed", ("BreachAnchorStabilized", "BreachGuardianDefeated")),
    16: ("CH16_SevenBeacons", ("BoundaryBeaconActivated", "BeaconAttackerDefeated")),
    17: ("CH17_LastSanctuary", ("SanctuaryGroupEvacuated", "SanctuaryInvaderDefeated")),
    18: ("CH18_CrownOfRuin", ("ThroneWardenDefeated", "RuinCommandCoreDestroyed")),
    19: ("CH19_BeyondTheGate", ("OriginGateSealStabilized", "OriginSentinelDefeated")),
    20: ("CH20_TheFinalHunter", ("OriginHeartExposed", "FinalHunterDefeated")),
}


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    source = CAMPAIGN_CPP.read_text(encoding="utf-8")
    guide = GUIDE.read_text(encoding="utf-8")

    for chapter, (quest_id, event_tags) in MISSIONS.items():
        if source.count(quest_id) < 2:
            fail(f"Chapter {chapter}: quest ID {quest_id} must appear in both start and completion logic")
        for tag in event_tags:
            if f'TEXT("{tag}")' not in source:
                fail(f"Chapter {chapter}: event tag {tag} is not authored in campaign C++")
            if f"`{tag}`" not in guide:
                fail(f"Chapter {chapter}: event tag {tag} is missing from the integration guide")
        if f"`{quest_id}`" not in guide:
            fail(f"Chapter {chapter}: quest ID {quest_id} is missing from the integration guide")

    if "CurrentChapterNumber == 20" not in source:
        fail("Final chapter completion gate is missing")
    if not re.search(r"CampaignCompleted|OnCampaignCompleted", source):
        fail("Campaign completion event path is not referenced in campaign subsystem")

    print(f"Validated {len(MISSIONS)} chapter quest IDs, {sum(len(v[1]) for v in MISSIONS.values())} event tags, and guide coverage.")


if __name__ == "__main__":
    main()
