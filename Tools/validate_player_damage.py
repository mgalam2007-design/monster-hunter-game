#!/usr/bin/env python3
"""Validate player damage intake and equipment mitigation safeguards."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "Source/HunterGame/Characters/ArinValeCharacter.cpp").read_text(encoding="utf-8")
start = SOURCE.find("float AArinValeCharacter::TakeDamage")
end = SOURCE.find("void AArinValeCharacter::SetupPlayerInputComponent", start)
assert start >= 0 and end > start, "Could not locate Arin TakeDamage implementation"
body = SOURCE[start:end]
for token in (
    "!FMath::IsFinite(DamageAmount)",
    "DamageAmount <= 0.0f",
    "HealthComponent->IsDead()",
    "FMath::IsFinite(RawDamageReduction)",
    "FMath::Clamp(RawDamageReduction, 0.0f, 0.95f)",
    "!FMath::IsFinite(MitigatedDamage)",
    "!FMath::IsFinite(AppliedDamage)",
    "HealthComponent->ApplyDamage(AppliedDamage)",
):
    assert token in body, f"Missing player damage safeguard: {token}"
print("Validated Arin damage input, death-state and equipment mitigation safeguards.")
