# Chapter 08 — A Hunter Reborn: Unreal Editor Integration Guide

Generate `/Game/Maps/Chapter08/L_CH08_HunterReborn` by compiling HunterGame and running `Tools/unreal/create_chapter08_level.py` from **Tools → Execute Python Script**. Build NavMesh before Play.

## Objectives

- Defeat six awakening ambushers. Each reports `AwakeningCreatureDefeated`.
- Press E at the awakening altar to report `AwakeningAltarActivated`.
- Complete both objectives and reach the chapter exit to save Chapter 8.

This greybox uses the native monster blockout and quest trigger. The awakening cinematic, Arin's evolution presentation, enemy art, animations, VFX/audio and final environment remain to be authored. The generator has static CI validation only; it has not been run in Unreal Editor.
