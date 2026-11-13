#!/usr/bin/env python3
"""Validate finite-value guards in monster damage handling."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "Source/HunterGame/Characters/HunterMonsterBase.cpp"


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    take_start = source.find("float AHunterMonsterBase::TakeDamage")
    receive_start = source.find("void AHunterMonsterBase::ReceiveHunterDamage")
    percent_start = source.find("float AHunterMonsterBase::GetHealthPercent")
    die_start = source.find("void AHunterMonsterBase::Die")
    if min(take_start, receive_start, percent_start, die_start) < 0:
        fail("could not locate monster damage and health functions")

    take = source[take_start:receive_start]
    receive = source[receive_start:percent_start]
    percent = source[percent_start:die_start]
    for token in ("!FMath::IsFinite(DamageAmount)", "!FMath::IsFinite(AppliedDamage)", "bIsDead"):
        if token not in take:
            fail(f"TakeDamage is missing guard: {token}")
    for token in ("!FMath::IsFinite(DamageAmount)", "FMath::IsFinite(MaxHealth)", "FMath::Clamp("):
        if token not in receive:
            fail(f"ReceiveHunterDamage is missing guard: {token}")
    if "FMath::Clamp(SafeCurrentHealth / SafeMaxHealth, 0.0f, 1.0f)" not in percent:
        fail("monster health percentage must be finite and clamped")

    print("Validated monster damage input guards and bounded health percentage.")


if __name__ == "__main__":
    main()
