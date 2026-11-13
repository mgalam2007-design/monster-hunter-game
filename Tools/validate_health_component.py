#!/usr/bin/env python3
"""Validate finite-value protection in the reusable health component."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "Source/HunterGame/Components/HunterHealthComponent.cpp"


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    required = (
        "FMath::IsFinite(MaxHealth)",
        "!FMath::IsFinite(Amount)",
        "!FMath::IsFinite(NewMaxHealth)",
        "FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f)",
        "FMath::Clamp(CurrentHealth / PreviousMax, 0.0f, 1.0f)",
    )
    for token in required:
        if token not in source:
            fail(f"health component is missing finite-value safeguard: {token}")

    damage_start = source.find("float UHunterHealthComponent::ApplyDamage")
    heal_start = source.find("float UHunterHealthComponent::Heal")
    set_max_start = source.find("void UHunterHealthComponent::SetMaxHealth")
    if min(damage_start, heal_start, set_max_start) < 0:
        fail("could not locate health mutation functions")
    damage = source[damage_start:heal_start]
    heal = source[heal_start:source.find("void UHunterHealthComponent::ReviveFullHealth", heal_start)]
    if "!FMath::IsFinite(Amount)" not in damage or "!FMath::IsFinite(Amount)" not in heal:
        fail("damage and healing must reject NaN/infinite amounts")
    if "!FMath::IsFinite(NewMaxHealth)" not in source[set_max_start:]:
        fail("max-health mutation must reject NaN/infinite values")

    print("Validated finite-value guards for damage, healing, max health and health percentage.")
    

if __name__ == "__main__":
    main()
