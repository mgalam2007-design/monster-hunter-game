#!/usr/bin/env python3
"""Validate offline content-based equipment recommendations."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / "Source/HunterGame/AI/HunterLoadoutRecommendationSubsystem.h"
SOURCE = ROOT / "Source/HunterGame/AI/HunterLoadoutRecommendationSubsystem.cpp"
WIDGET_H = ROOT / "Source/HunterGame/UI/HunterInventoryWidget.h"
WIDGET_CPP = ROOT / "Source/HunterGame/UI/HunterInventoryWidget.cpp"


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    header = HEADER.read_text(encoding="utf-8")
    source = SOURCE.read_text(encoding="utf-8")
    widget_h = WIDGET_H.read_text(encoding="utf-8")
    widget_cpp = WIDGET_CPP.read_text(encoding="utf-8")
    for token in ("FHunterEquipmentRecommendation", "RecommendEquipment",
                  "EHunterPlayerArchetype", "MaxResults"):
        if token not in header:
            fail(f"recommendation API is missing: {token}")
    for token in ("ScoreItem", "EHunterItemType::Weapon", "EHunterItemType::Armor",
                  "RequiredEvolutionStage", "DamageWeight", "DefenseWeight",
                  "FMath::IsFinite", "FMath::Clamp", "Results.Sort", "Results.SetNum"):
        if token not in source:
            fail(f"recommendation scoring or safety is missing: {token}")
    for token in ("BalancedResults", "AddBestOfType(EHunterItemType::Weapon)",
                  "AddBestOfType(EHunterItemType::Armor)", "ContainsByPredicate"):
        if token not in source:
            fail(f"recommendation shortlist must balance weapon and armor slots: {token}")
    for token in ("GetRecommendedEquipment", "FHunterEquipmentRecommendation"):
        if token not in widget_h or token not in widget_cpp:
            fail(f"inventory UI recommendation integration is missing: {token}")
    for token in ("AI LOADOUT RECOMMENDATIONS", "GetRecommendedEquipment(3)", "EquipItemById"):
        if token not in widget_cpp:
            fail(f"inventory UI recommendation presentation is missing: {token}")
    for token in ("EligibleCandidates.RemoveAll", "EquippedWeapon.ItemId", "EquippedArmor.ItemId"):
        if token not in widget_cpp:
            fail(f"recommendations must exclude already equipped gear: {token}")
    if any(token in header + source + widget_h + widget_cpp for token in ("https://", "OpenAI API", "GPT API")):
        fail("recommendation subsystem must remain fully offline")
    print("Validated offline equipment ranking, player-style weighting and inventory UI integration.")


if __name__ == "__main__":
    main()
