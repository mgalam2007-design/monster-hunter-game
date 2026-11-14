#!/usr/bin/env python3
"""Validate that overlap pickups cannot be granted more than once."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CPP = (ROOT / "Source/HunterGame/Items/HunterPickupActor.cpp").read_text(encoding="utf-8")
HEADER = (ROOT / "Source/HunterGame/Items/HunterPickupActor.h").read_text(encoding="utf-8")

assert "bool bPickupClaimed = false;" in HEADER
assert "if (!Arin || bPickupClaimed || PickupItem.ItemId.IsNone())" in CPP
assert CPP.index("bPickupClaimed = true;") < CPP.index("Inventory->AddItem(PickupItem)")
assert "bPickupClaimed = false;" in CPP
assert CPP.index("Inventory->AddItem(PickupItem)") < CPP.index("BP_OnPickedUp(Arin)")
assert "if (bDestroyAfterPickup)" in CPP

print("Pickup duplicate-claim source checks passed.")
