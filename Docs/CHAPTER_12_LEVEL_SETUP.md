# Chapter 12 — The Shadow Accord: Unreal Editor Integration Guide

Compile HunterGame and run `Tools/unreal/create_chapter12_level.py` from **Tools → Execute Python Script** to generate `/Game/Maps/Chapter12/L_CH12_ShadowAccord`. Build NavMesh before Play.

## Objectives
- Reach the unknown faction's contact point (`ShadowContactReached`).
- Defeat three shadow sentinels (`ShadowSentinelDefeated`).
- Choose one pact outcome: press E at **Bind the Shadows** (`BindShadows`) or **Free the Shadows** (`FreeShadows`). Either choice completes `ShadowPactResolved`; the other choice cannot then be selected.
- Complete all three objectives and reach the exit to save Chapter 12 and travel to Chapter 13.

The two pact stations are greybox interaction points. The selected choice is stored in the campaign subsystem, exposed to Blueprint through `GetChapter12StoryChoice`, and persisted in save version 7 (older version 1–6 saves load with no recorded choice). Valid values are `BindShadows` and `FreeShadows`; the first accepted choice is immutable for that campaign save. This establishes the branching story-state foundation, but later chapters do not yet change quests, dialogue or encounters based on the choice. Faction characters, choice consequences, enemy art, animation, VFX/audio, environment art and runtime testing remain outstanding.
