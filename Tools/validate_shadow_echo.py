#!/usr/bin/env python3
"""Validate native Shadow Echo combat, ownership, default wiring and timer cleanup."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HEADER = (ROOT / "Source/HunterGame/Characters/HunterShadowEchoActor.h").read_text(encoding="utf-8")
SOURCE = (ROOT / "Source/HunterGame/Characters/HunterShadowEchoActor.cpp").read_text(encoding="utf-8")
ABILITY = (ROOT / "Source/HunterGame/Components/HunterAbilityComponent.cpp").read_text(encoding="utf-8")

for token in ("PulseRadius", "PulseDamage", "PulseInterval", "EchoLifetime",
              "PulseTimerHandle", "LifetimeTimerHandle", "EndPlay"):
    assert token in HEADER, f"Missing Shadow Echo declaration: {token}"
for token in ("SetTimer(PulseTimerHandle", "SetTimer(LifetimeTimerHandle",
              "ClearTimer(PulseTimerHandle)", "ClearTimer(LifetimeTimerHandle)",
              "OverlapMultiByObjectType", "AHunterMonsterBase", "IsMonsterDead()",
              "HitThisPulse", "ApplyDamage(Monster, Damage, Arin->GetController()",
              "Profile.DamageMultiplier", "Destroy()"):
    assert token in SOURCE, f"Missing Shadow Echo behavior or cleanup: {token}"
assert '#include "Characters/HunterShadowEchoActor.h"' in ABILITY
assert "ShadowEchoActorClass = AHunterShadowEchoActor::StaticClass();" in ABILITY
assert "SpawnParams.Owner = Arin;" in ABILITY
assert "SpawnActor<AActor>(ShadowEchoActorClass" in ABILITY

print("Validated Shadow Echo pulse combat, owner attribution, native default class and timer cleanup.")
