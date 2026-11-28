# Chapter 13 — Siege of the North: Unreal Editor Integration Guide

Compile HunterGame and run `Tools/unreal/create_chapter13_level.py` from **Tools → Execute Python Script** to generate `/Game/Maps/Chapter13/L_CH13_SiegeOfTheNorth`. Build NavMesh before Play.

## Objectives
- Defeat five creatures in the northern fortress siege (`NorthernSiegeCreatureDefeated`).
- Press E at both fortress beacons (`FortressBeaconActivated`).
- Reach the exit after both objectives to save and travel to Chapter 14.

This is a greybox fortress layout using the current monster blockout and interaction markers. Siege set pieces, fortification art, enemy waves, animations, audio, VFX and runtime testing remain outstanding.

## Chapter 12 pact consequences
The saved Chapter 12 choice changes one Chapter 13 objective and enables only its matching interaction:
- **BindShadows:** command the bound shadow vanguard (ShadowVanguardCommanded). The freed-refugee interaction is disabled.
- **FreeShadows:** protect the freed shadow refugees (ShadowRefugeesProtected). The vanguard interaction is disabled.

The branch trigger checks the saved campaign choice at BeginPlay and disables its collision when it does not match. A save with no recognized Chapter 12 choice receives the two shared siege objectives only. This is a source-level implementation; confirm both branches in Unreal Editor after generating the level.
