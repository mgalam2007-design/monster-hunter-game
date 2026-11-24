# Chapter 12 — The Shadow Accord: Unreal Editor Integration Guide

Compile HunterGame and run `Tools/unreal/create_chapter12_level.py` from **Tools → Execute Python Script** to generate `/Game/Maps/Chapter12/L_CH12_ShadowAccord`. Build NavMesh before Play.

## Objectives
- Reach the unknown faction's contact point (`ShadowContactReached`).
- Defeat three shadow sentinels (`ShadowSentinelDefeated`).
- Press E at the pact altar (`ShadowPactResolved`).
- Complete all three objectives and reach the exit to save Chapter 12 and travel to Chapter 13.

The contact and pact altar are greybox interaction points. The current pact interaction reports completion but does not yet branch story state or offer a player choice UI. Faction characters, choice consequences, enemy art, animation, VFX/audio, environment art and runtime testing remain outstanding.
