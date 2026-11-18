# Chapter 03 — Guild Evaluation: Unreal Editor Integration Guide

## Generate the greybox level

1. Compile the `HunterGame` C++ module in Unreal Editor.
2. Run **Tools → Execute Python Script** and select `Tools/unreal/create_chapter03_level.py`.
3. The script creates `/Game/Maps/Chapter03/L_CH03_GuildEvaluation` with a native arena, PlayerStart, five native trial opponents, an E-key completion seal, lighting and NavMesh bounds.
4. Build navigation, press **P** to inspect the NavMesh, then Play.

## Objective flow

The campaign subsystem starts `CH03_GuildEvaluation` on chapter entry. Defeat all five placed opponents; each enemy instance is configured to report `TrialOpponentDefeated`. Then approach the seal and press **E** to report `TrialSealActivated`. The chapter exit remains gated until both objective counts are complete, then completes and saves the chapter.

The opponents use the native monster blockout and pursuit foundation. They are not final enemy art: skeletal meshes, animation, authored attacks, audio, VFX and difficulty tuning remain to be added. Confirm the level's NavMesh covers all five spawn positions.

## Chapter 2 to Chapter 3 travel

The Chapter 2 generator configures its chapter exit to open `/Game/Maps/Chapter03/L_CH03_GuildEvaluation` after Chapter 2 completion and successful save. Regenerate Chapter 2 after compiling the updated module so the placed exit receives the destination.

## Acceptance checklist

- [ ] Generate Chapter 2 and Chapter 3 maps in Unreal Editor.
- [ ] Complete Chapter 2 and confirm it transitions to Chapter 3 after saving.
- [ ] Confirm the quest tracker shows 0/5 trial opponents and the seal objective.
- [ ] Defeat five opponents and verify the count reaches 5/5.
- [ ] Activate the seal with E and verify the second objective completes.
- [ ] Confirm the exit is gated before both objectives are complete.
- [ ] Complete the chapter and verify campaign progress saves.

## Current boundary

The generator is committed and statically validated in CI, but it has not been executed in Unreal Editor here. Unreal compilation, NavMesh verification, enemy behavior, objective event delivery and the complete chapter transition still require editor/runtime testing.
