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
RECIPES = ROOT / "Docs/CHAPTERS_02_20_LEVEL_RECIPES.md"

# Each chapter lists its quest ID and (event tag, required count) pairs.
MISSIONS = {
    1: ("CH01_FirstBreach", (("MonsterDefeated", 3), ("BreachShardCollected", 1))),
    2: ("CH02_MissingSurvivors", (("SurvivorDistrictLocated", 1), ("SurvivorRescued", 3))),
    3: ("CH03_GuildEvaluation", (("TrialOpponentDefeated", 5), ("TrialSealActivated", 1))),
    4: ("CH04_TransitGate", (("TransitGateReached", 1), ("TransitGuardianDefeated", 1))),
    5: ("CH05_MiningContract", (("MiningCreatureDefeated", 4), ("CorruptedOreCollected", 3))),
    6: ("CH06_TrialOfSteel", (("SteelTrialOpponentDefeated", 1), ("SteelTrialApproved", 1))),
    7: ("CH07_BlueMark", (("BlueMarkSourceLocated", 1), ("ResonanceFragmentCollected", 3))),
    8: ("CH08_HunterReborn", (("AwakeningCreatureDefeated", 6), ("AwakeningAltarActivated", 1))),
    9: ("CH09_VanishedHunters", (("MissingHunterSignalLocated", 1), ("GateStalkerDefeated", 4), ("ExpeditionRecorderRecovered", 1))),
    10: ("CH10_HollowDistrict", (("HollowDistrictCoreReached", 1), ("ShadowAnchorDestroyed", 3), ("HollowWardenDefeated", 1))),
    11: ("CH11_EchoesOfTheFallen", (("ExpeditionMemoryRecovered", 3), ("FallenCaptainDefeated", 1))),
    12: ("CH12_ShadowAccord", (("ShadowContactReached", 1), ("ShadowSentinelDefeated", 3), ("ShadowPactResolved", 1))),
    13: ("CH13_SiegeOfTheNorth", (("NorthernSiegeCreatureDefeated", 5), ("FortressBeaconActivated", 2))),
    14: ("CH14_BrokenCitadel", (("CitadelPylonDestroyed", 3), ("CitadelCommanderDefeated", 1))),
    15: ("CH15_WorldsUnsealed", (("BreachAnchorStabilized", 4), ("BreachGuardianDefeated", 2))),
    16: ("CH16_SevenBeacons", (("BoundaryBeaconActivated", 7), ("BeaconAttackerDefeated", 3))),
    17: ("CH17_LastSanctuary", (("SanctuaryGroupEvacuated", 5), ("SanctuaryInvaderDefeated", 6))),
    18: ("CH18_CrownOfRuin", (("ThroneWardenDefeated", 4), ("RuinCommandCoreDestroyed", 3))),
    19: ("CH19_BeyondTheGate", (("OriginGateSealStabilized", 3), ("OriginSentinelDefeated", 2))),
    20: ("CH20_TheFinalHunter", (("OriginHeartExposed", 1), ("FinalHunterDefeated", 1))),
}


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def objective_counts_for_quest(source: str, quest_id: str) -> dict[str, int]:
    """Read objective event/count pairs from the quest's own StartQuest block."""
    start_marker = f'if (Quest->GetActiveQuestId() != FName(TEXT("{quest_id}")))'
    start = source.find(start_marker)
    if start < 0:
        fail(f"{quest_id}: cannot find its quest initialization branch")

    call_marker = f'Quest->StartQuest(TEXT("{quest_id}"), Objectives)'
    end = source.find(call_marker, start)
    if end < 0:
        fail(f"{quest_id}: cannot find its StartQuest call")

    branch = source[start:end]
    objective_pattern = re.compile(
        r"FHunterObjective\s+\w+;(?:(?!FHunterObjective).)*?"
        r'EventTag\s*=\s*TEXT\("([^"]+)"\);(?:(?!FHunterObjective).)*?'
        r"RequiredCount\s*=\s*(\d+)\s*;",
        re.DOTALL,
    )
    counts: dict[str, int] = {}
    for tag, count in objective_pattern.findall(branch):
        if tag in counts:
            fail(f"{quest_id}: duplicate objective event tag {tag}")
        counts[tag] = int(count)
    return counts



def function_body(source: str, signature: str) -> str:
    """Return one C++ function body using balanced braces."""
    start = source.find(signature)
    if start < 0:
        fail(f"cannot find function {signature}")
    opening = source.find("{", start)
    if opening < 0:
        fail(f"cannot find opening brace for {signature}")

    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:index]
    fail(f"unterminated function body for {signature}")


def main() -> None:
    source = CAMPAIGN_CPP.read_text(encoding="utf-8")
    guide = GUIDE.read_text(encoding="utf-8")
    recipes = RECIPES.read_text(encoding="utf-8")
    total_events = 0
    completion_body = function_body(source, "bool UHunterCampaignSubsystem::CompleteCurrentChapter()")

    for chapter, (quest_id, objectives) in MISSIONS.items():
        if source.count(quest_id) < 2:
            fail(f"Chapter {chapter}: quest ID {quest_id} must appear in both start and completion logic")
        if f"`{quest_id}`" not in guide:
            fail(f"Chapter {chapter}: quest ID {quest_id} is missing from the integration guide")

        exact_gate = f'GetActiveQuestId() != FName(TEXT("{quest_id}"))'
        if exact_gate not in completion_body:
            fail(f"Chapter {chapter}: completion does not require its exact active quest ID")
        if "!Quest->IsQuestComplete()" not in completion_body:
            fail(f"Chapter {chapter}: completion does not fail closed for an incomplete quest")

        actual = objective_counts_for_quest(source, quest_id)
        expected = dict(objectives)
        if actual != expected:
            fail(
                f"Chapter {chapter} ({quest_id}): objective event/count mismatch; "
                f"expected {expected}, found {actual}"
            )

        for tag, required_count in objectives:
            if f"`{tag}`" not in guide:
                fail(f"Chapter {chapter}: event tag {tag} is missing from the integration guide")
            if chapter > 1 and f"`{tag}`" not in recipes:
                fail(f"Chapter {chapter}: event tag {tag} is missing from the level assembly recipes")
            total_events += 1

        if chapter > 1 and f"`{quest_id}`" not in recipes:
            fail(f"Chapter {chapter}: quest ID {quest_id} is missing from the level assembly recipes")

    if "CurrentChapterNumber == 20" not in source:
        fail("Final chapter completion gate is missing")
    if not re.search(r"CampaignCompleted|OnCampaignCompleted", source):
        fail("Campaign completion event path is not referenced in campaign subsystem")

    print(
        f"Validated {len(MISSIONS)} chapter quest IDs, {total_events} objective event/count pairs, "
        "exact per-chapter completion gates, final campaign completion, integration-guide coverage, "
        "and Chapters 02–20 recipe coverage."
    )


if __name__ == "__main__":
    main()
