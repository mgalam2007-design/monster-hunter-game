# Chapter 07 — The Blue Mark: Unreal Editor Integration Guide

Generate `/Game/Maps/Chapter07/L_CH07_BlueMark` by compiling HunterGame and running `Tools/unreal/create_chapter07_level.py` from **Tools → Execute Python Script**. Build NavMesh before Play.

## Objectives

- Reach the blue-mark source to report `BlueMarkSourceLocated`.
- Press E at each of three resonance-fragment markers. Each one-shot interaction reports `ResonanceFragmentCollected`.
- Complete both objectives and reach the exit. Chapter 7 then saves and travels to Chapter 8.

The fragments are interaction markers in this greybox, not finished collectible meshes. Blue-mark effects, environment art, animation, audio and VFX remain to be authored. The generator has static CI validation only; it has not been run in Unreal Editor.
