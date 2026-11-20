# Chapter 05 — The Silent Contract: Unreal Editor Integration Guide

## Generate the greybox level

1. Compile the `HunterGame` C++ module in Unreal Editor.
2. Run **Tools → Execute Python Script** and select `Tools/unreal/create_chapter05_level.py`.
3. The script creates `/Game/Maps/Chapter05/L_CH05_MiningSettlement` with a native arena, PlayerStart, four tagged mining creatures, three interactive ore-sample points, lighting and NavMesh bounds.
4. Build navigation, press **P** to inspect the NavMesh, then Play.

## Objective flow

The campaign subsystem starts `CH05_MiningContract` on chapter entry.

- Defeat all four settlement creatures. Each reports `MiningCreatureDefeated`.
- Approach each corrupted ore sample and press **E**. Each one-shot interaction reports `CorruptedOreCollected`.
- Reach the chapter exit only after all seven objective counts are complete. The exit grants 375 XP and 185 currency, then saves Chapter 5.

The sample points use the reusable quest interaction actor and currently represent recoverable ore as greybox interaction markers. A production ore mesh, pickup animation, mining VFX/audio and environmental art remain to be authored. Enemy visuals are the existing native blockout; AI requires a built NavMesh.

## Chapter 4 to Chapter 5 travel

The Chapter 4 generator now configures its exit to open `/Game/Maps/Chapter05/L_CH05_MiningSettlement` after Chapter 4 completion and successful save. Regenerate Chapter 4 after compiling the updated module so the placed exit receives the destination.

## Acceptance checklist

- [ ] Generate Chapter 4 and Chapter 5 maps in Unreal Editor.
- [ ] Complete Chapter 4 and confirm it transitions to Chapter 5 after saving.
- [ ] Confirm the quest tracker shows four creature defeats and three ore samples.
- [ ] Defeat four enemies and verify the defeat objective reaches 4/4.
- [ ] Press E at each of the three ore sample points and verify each counts once.
- [ ] Confirm the exit is gated until both objectives complete and saves progress.

## Current boundary

The generator is committed and statically validated in CI, but it has not been executed in Unreal Editor here. Unreal compilation, map generation, AI navigation, interaction behavior, objective delivery and campaign travel still require editor/runtime testing.
