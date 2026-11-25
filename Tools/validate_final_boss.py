#!/usr/bin/env python3
"""Static checks for the dedicated Chapter 20 Final Hunter boss."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HEADER = (ROOT / "Source/HunterGame/Characters/HunterFinalBossBase.h").read_text(encoding="utf-8")
SOURCE = (ROOT / "Source/HunterGame/Characters/HunterFinalBossBase.cpp").read_text(encoding="utf-8")
GENERATOR = (ROOT / "Tools/unreal/create_chapter20_level.py").read_text(encoding="utf-8")
GUIDE = (ROOT / "Docs/CHAPTER_20_LEVEL_SETUP.md").read_text(encoding="utf-8")

for token in (
    "AHunterFinalBossBase", "BeginOriginPulse", "ResolveOriginPulse",
    "ScheduleNextOriginPulse", "BP_OnOriginPulseTelegraph",
    "BP_OnOriginPulseImpact", "OriginPulseCooldownHandle",
    "OriginPulseImpactHandle", "OriginAftershockHandle", "ResolveOriginAftershock",
    "ApplyPulseDamage", "BP_OnOriginAftershockTelegraph",
    "BP_OnOriginAftershockImpact", "EndPlay",
):
    assert token in HEADER or token in SOURCE, f"Missing final boss behavior: {token}"

for token in (
    "OriginPulseRadius = 650.0f", "OriginPulseDamage = 48.0f",
    "OriginPulseWindup = 1.35f", "OriginPulseCooldown = 8.0f",
    "FMath::IsFinite(OriginPulseRadius)", "FMath::IsFinite(OriginPulseDamage)",
    "FMath::IsFinite(OriginPulseWindup)", "FMath::IsFinite(OriginPulseCooldown)",
    "BP_OnOriginPulseTelegraph(OriginPulseRadius, SafeWindup)",
    "BP_OnOriginPulseImpact(OriginPulseRadius)",
    "UGameplayStatics::ApplyDamage(Arin, Damage",
    "GetCurrentBossPhase()", "ClearTimer(OriginPulseCooldownHandle)",
    "ClearTimer(OriginPulseImpactHandle)",
    "ClearTimer(OriginAftershockHandle)",
    "if (GetCurrentBossPhase() >= 2)",
    "const float AftershockRadius = OriginPulseRadius * 0.60f",
    "const float AftershockWindup = 0.55f",
    "ApplyPulseDamage(Radius, OriginPulseDamage * 0.55f)",
    "FMath::IsFinite(BaseDamage)",
):
    assert token in SOURCE, f"Missing final boss implementation contract: {token}"

assert 'native_class("HunterFinalBossBase")' in GENERATOR
assert "HunterFinalBossBase" in GUIDE
assert "Origin Pulse" in GUIDE
print("Dedicated Final Hunter class, telegraph, phase scaling, damage and Chapter 20 wiring passed.")
