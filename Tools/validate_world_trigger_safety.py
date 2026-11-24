#!/usr/bin/env python3
"""Validate safe runtime setup for checkpoint and chapter-exit trigger volumes."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CHECKPOINT = (ROOT / "Source/HunterGame/Items/HunterCheckpointActor.cpp").read_text(encoding="utf-8")
EXIT = (ROOT / "Source/HunterGame/Items/HunterChapterExitActor.cpp").read_text(encoding="utf-8")

assert "FMath::IsFinite(ActivationRadius)" in CHECKPOINT
assert "FMath::Max(50.0f, ActivationRadius)" in CHECKPOINT
assert ": 180.0f;" in CHECKPOINT
assert "SaveSlotName.IsEmpty()" in CHECKPOINT

assert "FMath::IsFinite(TriggerRadius)" in EXIT
assert "FMath::Max(50.0f, TriggerRadius)" in EXIT
assert ": 220.0f;" in EXIT
assert "SaveSlotName.IsEmpty()" in EXIT

print("Validated finite, minimum-sized checkpoint and chapter-exit trigger setup.")

# Chapter exits only travel after a successful save and an explicitly configured map.
EXIT_HEADER = (ROOT / "Source/HunterGame/Items/HunterChapterExitActor.h").read_text(encoding="utf-8")
assert "FName NextChapterMapName = NAME_None" in EXIT_HEADER
assert "if (bSaved && !NextChapterMapName.IsNone() && GetWorld())" in EXIT
assert "UGameplayStatics::OpenLevel(this, NextChapterMapName)" in EXIT

# Core Chapter 1 navigation markers remain visible without Blueprint art.
CHECKPOINT_HEADER = (ROOT / "Source/HunterGame/Items/HunterCheckpointActor.h").read_text(encoding="utf-8")
EXIT_HEADER = (ROOT / "Source/HunterGame/Items/HunterChapterExitActor.h").read_text(encoding="utf-8")
assert "CheckpointVisual" in CHECKPOINT_HEADER and "FObjectFinder<UStaticMesh> CheckpointMesh" in CHECKPOINT
assert "ExitVisuals" in EXIT_HEADER and "ExitGateLeftPillar" in EXIT and "ExitGateHeader" in EXIT
assert "SetCollisionEnabled(ECollisionEnabled::NoCollision)" in CHECKPOINT
assert "SetCollisionEnabled(ECollisionEnabled::NoCollision)" in EXIT
