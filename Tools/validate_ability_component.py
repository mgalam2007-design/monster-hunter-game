#!/usr/bin/env python3
"""Validate finite-value and resource guards in the hunter ability component."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "Source/HunterGame/Components/HunterAbilityComponent.cpp").read_text(encoding="utf-8")

required = [
    "FMath::IsFinite(MaxEnergy)",
    "FMath::IsFinite(EnergyRegenerationPerSecond)",
    "FMath::IsFinite(CurrentEnergy)",
    "!FMath::IsFinite(Amount)",
    "!FMath::IsFinite(DeltaTime)",
    "FMath::IsFinite(EnergyStrikeDamage)",
    "FMath::IsFinite(EnergyStrikeRadius)",
    "FMath::IsFinite(ShadowStepStrength)",
    "FMath::IsFinite(ShadowEchoSpawnDistance)",
    "!FMath::IsFinite(Cost)",
    "!FMath::IsFinite(GetAbilityCooldown(Ability))",
]
for token in required:
    assert token in SOURCE, f"Missing ability safety guard: {token}"

print("Validated ability energy, timing, cooldown and tuning safeguards.")
