#!/usr/bin/env python3
"""Validate finite-value normalization for Arin's editable movement/combat tuning."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "Source/HunterGame/Characters/ArinValeCharacter.cpp").read_text(encoding="utf-8")
start = SOURCE.find("void AArinValeCharacter::BeginPlay")
end = SOURCE.find("float AArinValeCharacter::TakeDamage", start)
assert start >= 0 and end > start, "Could not locate Arin BeginPlay"
body = SOURCE[start:end]
for token in (
    "FMath::IsFinite(SprintSpeed)", "FMath::IsFinite(DodgeStrength)",
    "FMath::IsFinite(DodgeUpwardStrength)", "FMath::IsFinite(RespawnDelay)",
    "FMath::IsFinite(SwordAttackCooldown)", "FMath::IsFinite(DefaultWeaponTraceWindow)",
    "FMath::IsFinite(GetCharacterMovement()->MaxWalkSpeed)",
):
    assert token in body, f"Missing Arin tuning normalization: {token}"
for token in (
    "FMath::IsFinite(Value)",
    "FMath::IsFinite(RawMultiplier)",
    "if (!GetCharacterMovement()) return;",
):
    assert token in SOURCE, f"Missing player input safety guard: {token}"
print("Validated Arin movement, dodge, respawn, attack tuning and input axis safeguards.")
