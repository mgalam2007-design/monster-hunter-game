#!/usr/bin/env python3
"""Static checks for the Unreal boss prototype foundation."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HEADER = (ROOT / "Source/HunterGame/Characters/HunterBossBase.h").read_text(encoding="utf-8")
SOURCE = (ROOT / "Source/HunterGame/Characters/HunterBossBase.cpp").read_text(encoding="utf-8")

assert "virtual void BeginPlay() override" in HEADER
for part in (
    "BossCrownCore", "BossChestPlate", "BossShoulderLeft",
    "BossShoulderRight", "BossCrownHornLeft", "BossCrownHornRight",
    "BossBackSpine",
):
    assert f'TEXT("{part}")' in SOURCE, f"Missing boss silhouette part: {part}"

assert "MaxHealth = 1200.0f" in SOURCE
assert "AttackDamage = 35.0f" in SOURCE
assert "ExperienceReward = 300" in SOURCE
assert "CurrencyReward = 100" in SOURCE
assert "SetCollisionEnabled(ECollisionEnabled::NoCollision)" in SOURCE
assert "SetCanEverAffectNavigation(false)" in SOURCE
assert "FMath::IsFinite(AttackDamageMultiplierPerPhase)" in SOURCE
assert "PhaseHealthThresholds.Sort" in SOURCE
assert "BaseAttackDamage = AttackDamage" in SOURCE
assert "if (!IsMonsterDead())" in SOURCE
print("Boss blockout, reward defaults and phase-scaling safeguards passed.")
