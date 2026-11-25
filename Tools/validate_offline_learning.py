#!/usr/bin/env python3
"""Validate the standalone offline Q-learning foundation and monster integration."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / "Source/HunterGame/AI/HunterLearningSubsystem.h"
SOURCE = ROOT / "Source/HunterGame/AI/HunterLearningSubsystem.cpp"
SAVE = ROOT / "Source/HunterGame/AI/HunterLearningSaveGame.h"
PROFILE_HEADER = ROOT / "Source/HunterGame/AI/HunterPlayerProfileSubsystem.h"
PROFILE_SOURCE = ROOT / "Source/HunterGame/AI/HunterPlayerProfileSubsystem.cpp"
PROFILE_SAVE = ROOT / "Source/HunterGame/AI/HunterPlayerProfileSaveGame.h"
MONSTER = ROOT / "Source/HunterGame/Characters/HunterMonsterBase.cpp"


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    header = HEADER.read_text(encoding="utf-8")
    source = SOURCE.read_text(encoding="utf-8")
    save = SAVE.read_text(encoding="utf-8")
    monster = MONSTER.read_text(encoding="utf-8")
    profile_header = PROFILE_HEADER.read_text(encoding="utf-8")
    profile_source = PROFILE_SOURCE.read_text(encoding="utf-8")
    profile_save = PROFILE_SAVE.read_text(encoding="utf-8")

    for token in ("PlayerFar", "PlayerApproaching", "PlayerInRange", "PlayerLowHealth",
                  "PlayerRetreating", "PlayerDodging", "Balanced", "Aggressive", "Cautious",
                  "ChooseCombatAction", "RecordCombatOutcome", "ResetLearning"):
        if token not in header:
            fail(f"learning subsystem header is missing: {token}")

    for token in ("ExplorationRate", "DiscountFactor", "LearningRate", "GetBestFutureValue",
                  "FMath::FRand()", "FMath::RandRange", "FMath::IsFinite(Reward)",
                  "FMath::Clamp(UpdatedValue, -25.0f, 25.0f)", "SaveGameToSlot",
                  "LoadGameFromSlot", "DoesSaveGameExist", "UpdatesSinceSave >= 20"):
        if token not in source:
            fail(f"Q-learning implementation is missing: {token}")

    for token in ("SaveVersion", "QValues", "ExplorationRate", "UpdateCount"):
        if token not in save:
            fail(f"local learning save data is missing: {token}")

    for token in ("ChooseCombatAction(LearningState)", "RecordCombatOutcome(LearningState, LearningAction",
                  "DamageScale", "CooldownScale"):
        if token not in monster:
            fail(f"monster combat is not using learned tactics: {token}")

    if any(token in source + header for token in ("https://", "OpenAI API", "GPT API")):
        fail("offline learning foundation must not contain hosted AI dependencies")

    for token in ("Aggressive", "Defensive", "Evasive", "Balanced", "ObservePlayer"):
        if token not in profile_header:
            fail(f"player profiling header is missing: {token}")
    for token in ("FindNearestCluster", "Centroids", "LearningRate", "FMath::Sqrt",
                  "SaveGameToSlot", "LoadGameFromSlot", "ObservationCount"):
        if token not in profile_source:
            fail(f"online k-means profiling is missing: {token}")
    for token in ("Centroids", "ClusterCounts", "ObservationCount", "SaveVersion"):
        if token not in profile_save:
            fail(f"player profile persistence is missing: {token}")
    for token in ("UHunterPlayerProfileSubsystem", "ObservePlayer",
                  "EHunterPlayerArchetype::Aggressive", "EHunterPlayerArchetype::Defensive"):
        if token not in monster:
            fail(f"monster AI is missing player-profile integration: {token}")

    print("Validated standalone offline Q-learning, local persistence and monster combat integration.")


if __name__ == "__main__":
    main()
