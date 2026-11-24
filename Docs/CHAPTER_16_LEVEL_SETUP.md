# Chapter 16 — Seven Beacons: Unreal Editor Integration Guide

Compile HunterGame and run `Tools/unreal/create_chapter16_level.py` from **Tools → Execute Python Script** to generate `/Game/Maps/Chapter16/L_CH16_SevenBeacons`. Build NavMesh before Play.

## Objectives
- Press E at all seven boundary beacons (`BoundaryBeaconActivated`).
- Defeat three beacon attackers (`BeaconAttackerDefeated`).
- Reach the exit after both objectives to save and travel to Chapter 17.

Beacon markers and attackers use greybox/native blockout actors. Beacon activation VFX, defense waves, environment art, animation/audio and runtime testing remain outstanding.
