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
move_right_start = SOURCE.find("void AArinValeCharacter::MoveRight")
move_right_end = SOURCE.find("void AArinValeCharacter::Turn", move_right_start)
assert move_right_start >= 0 and move_right_end > move_right_start, "Could not locate MoveRight"
assert "FMath::IsFinite(Value)" in SOURCE[move_right_start:move_right_end], "MoveRight must reject non-finite axis input"

respawn_start = SOURCE.find("void AArinValeCharacter::RespawnAtLastCheckpoint")
respawn_end = SOURCE.find("void AArinValeCharacter::", respawn_start + 10)
assert respawn_start >= 0 and respawn_end > respawn_start, "Could not locate respawn implementation"
respawn = SOURCE[respawn_start:respawn_end]
for token in (
    "HealthComponent->ReviveFullHealth()",
    "StopSprint();",
    "GetWorldTimerManager().ClearTimer(AttackCooldownHandle)",
    "GetWorldTimerManager().ClearTimer(DodgeCooldownHandle)",
    "GetWorldTimerManager().ClearTimer(WeaponTraceHandle)",
    "EndWeaponTrace();",
):
    assert token in respawn, f"Respawn must restore clean combat/movement state: {token}"

print("Validated Arin movement, dodge, respawn, attack tuning and input axis safeguards.")
