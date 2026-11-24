#!/usr/bin/env python3
"""Validate Arin's stage-based visual evolution hooks and component visibility."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HEADER = (ROOT / "Source/HunterGame/Characters/ArinValeCharacter.h").read_text(encoding="utf-8")
SOURCE = (ROOT / "Source/HunterGame/Characters/ArinValeCharacter.cpp").read_text(encoding="utf-8")

for token in (
    "EvolvedShoulderArmor", "EliteChestCore", "LegendaryCrown", "UltimateAuraCore",
    "void ApplyEvolutionVisuals();",
):
    assert token in HEADER, f"Missing evolution visual declaration: {token}"

for token in (
    "ArinEvolvedShoulderArmor", "ArinEliteChestCore",
    "ArinLegendaryCrown", "ArinUltimateAuraCore",
    "SetVisibility(false, true)", "ApplyEvolutionVisuals();",
    "Stage >= 1", "Stage >= 2", "Stage >= 3", "Stage >= 4",
):
    assert token in SOURCE, f"Missing evolution visual behavior: {token}"

handler_start = SOURCE.find("void AArinValeCharacter::HandleEvolutionStageChanged")
handler_end = SOURCE.find("void AArinValeCharacter::ApplyEvolutionStats", handler_start)
assert handler_start >= 0 and handler_end > handler_start
assert "ApplyEvolutionVisuals();" in SOURCE[handler_start:handler_end]
print("Arin's five-stage visual evolution visibility and stage-change refresh passed.")
