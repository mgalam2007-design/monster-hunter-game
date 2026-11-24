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

# Evolution-stage scaling must affect combat output and resource pacing.
for token in (
    "GetEvolutionStage()", "BaseCost * FMath::Max(0.75f",
    "BaseCooldown * FMath::Max(0.80f", "Profile.DamageMultiplier",
    "ScaledRadius = EnergyStrikeRadius * (1.0f + 0.05f * Stage)",
    "ApplyDamage(Target, ScaledDamage", "Profile.MovementMultiplier",
    "ShadowStepStrength * MovementMultiplier",
):
    assert token in SOURCE, f"Missing evolution-scaled ability behavior: {token}"
