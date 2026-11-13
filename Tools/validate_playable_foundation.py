#!/usr/bin/env python3
"""Validate that the Unreal character has a visible blockout and configured legacy inputs."""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
CHARACTER = ROOT / "Source/HunterGame/Characters/ArinValeCharacter.cpp"
INPUTS = ROOT / "Config/DefaultInput.ini"


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    source = CHARACTER.read_text(encoding="utf-8")
    config = INPUTS.read_text(encoding="utf-8")

    required_parts = (
        "ArinBlockoutTorso", "ArinBlockoutShoulderL", "ArinBlockoutShoulderR",
        "ArinBlockoutArmL", "ArinBlockoutArmR", "ArinBlockoutLegL",
        "ArinBlockoutLegR", "ArinBlockoutHead", "ArinBlockoutHair",
        "ArinBlockoutSword",
    )
    for part in required_parts:
        if part not in source:
            fail(f"Arin's temporary visual blockout is missing {part}")

    for asset in ("/Engine/BasicShapes/Cube.Cube", "/Engine/BasicShapes/Sphere.Sphere"):
        if asset not in source:
            fail(f"Arin blockout does not load expected engine primitive {asset}")
    if "SetCollisionEnabled(ECollisionEnabled::NoCollision)" not in source:
        fail("Visual blockout parts must not interfere with character collision")

    bound_axes = set(re.findall(r'BindAxis\("([^"]+)"', source))
    bound_actions = set(re.findall(r'BindAction\("([^"]+)"', source))
    configured_axes = set(re.findall(r'AxisName="([^"]+)"', config))
    configured_actions = set(re.findall(r'ActionName="([^"]+)"', config))

    missing_axes = sorted(bound_axes - configured_axes)
    missing_actions = sorted(bound_actions - configured_actions)
    if missing_axes:
        fail("Input axes have no DefaultInput.ini mapping: " + ", ".join(missing_axes))
    if missing_actions:
        fail("Input actions have no DefaultInput.ini mapping: " + ", ".join(missing_actions))

    print(
        f"Validated {len(required_parts)} visible Arin blockout parts and "
        f"{len(bound_axes)} axis / {len(bound_actions)} action input bindings."
    )


if __name__ == "__main__":
    main()
