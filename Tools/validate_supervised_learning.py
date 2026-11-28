#!/usr/bin/env python3
"""Validate the standalone offline supervised behaviour prediction model."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / "Source/HunterGame/AI/HunterBehaviourPredictionSubsystem.h"
SOURCE = ROOT / "Source/HunterGame/AI/HunterBehaviourPredictionSubsystem.cpp"
MONSTER = ROOT / "Source/HunterGame/Characters/HunterMonsterBase.cpp"
SAVE = ROOT / "Source/HunterGame/AI/HunterBehaviourModelSaveGame.h"


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    header = HEADER.read_text(encoding="utf-8")
    source = SOURCE.read_text(encoding="utf-8")
    save = SAVE.read_text(encoding="utf-8")
    monster = MONSTER.read_text(encoding="utf-8")

    for token in ("PredictEvasionProbability", "RecordOutcome", "GetTrainingSampleCount",
                  "SaveModel", "LoadModel", "ResetModel"):
        if token not in header:
            fail(f"supervised model header is missing: {token}")
    for token in ("BuildFeatures", "1.0f / (1.0f + FMath::Exp(-Logit))",
                  "const float Gradient", "LearningRate * Gradient",
                  "LearningRate * Gradient", "Weights[Index] + LearningRate * Gradient, -8.0f, 8.0f)", "FMath::IsFinite",
                  "SaveGameToSlot", "LoadGameFromSlot", "DoesSaveGameExist",
                  "SamplesSinceSave >= 20"):
        if token not in source:
            fail(f"supervised model implementation is missing: {token}")
    for token in ("SaveVersion", "Weights", "TrainingSamples"):
        if token not in save:
            fail(f"supervised model save data is missing: {token}")
    initialize = source.split("void UHunterBehaviourPredictionSubsystem::Initialize", 1)[1].split(
        "void UHunterBehaviourPredictionSubsystem::Deinitialize", 1
    )[0]
    reset = source.split("void UHunterBehaviourPredictionSubsystem::ResetModel", 1)[1].split(
        "TArray<float> UHunterBehaviourPredictionSubsystem::BuildFeatures", 1
    )[0]
    if "LoadModel();" not in initialize or "ResetModel();" in initialize:
        fail("initialization must load saved weights without invoking the destructive reset")
    if "DeleteGameInSlot" not in reset:
        fail("explicit model reset must delete the persisted model slot")
    for token in ("PredictEvasionProbability", "RecordOutcome", "PendingEvasionDistanceRatio",
                  "PendingEvasionSpeedRatio", "PendingEvasionHealthDeficit"):
        if token not in monster:
            fail(f"monster combat integration is missing: {token}")
    if "GetTrainingSampleCount() >= 12" not in monster:
        fail("monster must wait for a minimum supervised training sample count")
    if "const_cast<AArinValeCharacter" in monster:
        fail("monster target should be mutable without const_cast")
    if any(token in header + source + monster for token in ("https://", "OpenAI API", "GPT API")):
        fail("supervised model must not contain hosted AI dependencies")

    print("Validated offline supervised learning, local persistence and monster combat integration.")


if __name__ == "__main__":
    main()
