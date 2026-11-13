#!/usr/bin/env python3
"""Validate swept melee tracing and damage safety in the Unreal source."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / "Source/HunterGame/Components/HunterWeaponTraceComponent.h"
SOURCE = ROOT / "Source/HunterGame/Components/HunterWeaponTraceComponent.cpp"


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    header = HEADER.read_text(encoding="utf-8")
    source = SOURCE.read_text(encoding="utf-8")

    required_header = (
        "float TraceRadius = 12.0f",
        "FName BaseSocket",
        "FName TipSocket",
        "TSet<TWeakObjectPtr<AActor>> HitActors",
    )
    for token in required_header:
        if token not in header:
            fail(f"Weapon trace header is missing {token}")

    required_source = (
        "FMath::IsFinite(BaseDamage)",
        "FMath::IsFinite(TraceRadius)",
        "DoesSocketExist(BaseSocket)",
        "DoesSocketExist(TipSocket)",
        "FCollisionShape::MakeSphere(TraceRadius)",
        "SweepMultiByChannel(",
        "HitActors.Contains(HitActor)",
        "HitActors.Add(HitActor)",
        "ApplyPointDamage(",
        "EndWeaponTrace();",
    )
    for token in required_source:
        if token not in source:
            fail(f"Weapon trace implementation is missing {token}")

    if "LineTraceMultiByChannel" in source:
        fail("Legacy line-only tracing remains; swept blade volumes should be used")

    print("Validated swept melee volumes, socket guards, per-swing hit de-duplication and finite damage settings.")


if __name__ == "__main__":
    main()
