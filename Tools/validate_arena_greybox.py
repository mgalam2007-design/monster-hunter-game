#!/usr/bin/env python3
"""Static checks for the Unreal breach arena greybox actor."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / "Source/HunterGame/World/HunterPrototypeArenaActor.h"
SOURCE = ROOT / "Source/HunterGame/World/HunterPrototypeArenaActor.cpp"


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    header = HEADER.read_text(encoding="utf-8")
    source = SOURCE.read_text(encoding="utf-8")

    required_parts = (
        "ArenaFloor", "ArenaWestWall", "ArenaEastWall", "ArenaFrontWall",
        "ArenaRearWest", "ArenaRearEast", "BreachGateLeft",
        "BreachGateRight", "BreachGateLintel", "ArenaPillarNW",
        "ArenaPillarNE", "ArenaPillarSW", "ArenaPillarSE",
    )
    for part in required_parts:
        if f'TEXT("{part}")' not in source:
            fail(f"Arena is missing required greybox piece: {part}")

    if "SetRootComponent(SceneRoot)" not in source:
        fail("Arena has no scene root")
    if "SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics)" not in source:
        fail("Arena geometry must block player and enemy movement")
    if "SetCanEverAffectNavigation(true)" not in source:
        fail("Arena geometry is not configured to contribute to navigation")
    if "GateLight->SetLightColor(FLinearColor(0.08f, 0.38f, 1.0f))" not in source:
        fail("Breach gate blue light is missing")
    if "ArenaHalfWidth" not in header or "ArenaHalfDepth" not in header or "WallHeight" not in header:
        fail("Arena dimensions must be editable properties")
    if "SetMobility(EComponentMobility::Static)" not in source:
        fail("Static arena geometry should be marked static")

    print(f"Validated {len(required_parts)} arena pieces, blocking collision, navigation and gate lighting.")


if __name__ == "__main__":
    main()
