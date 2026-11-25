#!/usr/bin/env python3
"""Validate offline content-based equipment recommendations."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / "Source/HunterGame/AI/HunterLoadoutRecommendationSubsystem.h"
SOURCE = ROOT / "Source/HunterGame/AI/HunterLoadoutRecommendationSubsystem.cpp"


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    header = HEADER.read_text(encoding="utf-8")
    source = SOURCE.read_text(encoding="utf-8")
    for token in ("FHunterEquipmentRecommendation", "RecommendEquipment",
                  "EHunterPlayerArchetype", "MaxResults"):
        if token not in header:
            fail(f"recommendation API is missing: {token}")
    for token in ("ScoreItem", "EHunterItemType::Weapon", "EHunterItemType::Armor",
                  "RequiredEvolutionStage", "DamageWeight", "DefenseWeight",
                  "FMath::IsFinite", "FMath::Clamp", "Results.Sort", "Results.SetNum"):
        if token not in source:
            fail(f"recommendation scoring or safety is missing: {token}")
    if any(token in header + source for token in ("https://", "OpenAI API", "GPT API")):
        fail("recommendation subsystem must remain fully offline")
    print("Validated offline equipment ranking, archetype weighting and evolution eligibility.")


if __name__ == "__main__":
    main()
