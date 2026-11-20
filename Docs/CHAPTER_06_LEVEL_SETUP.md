# Chapter 06 — Trial of Steel: Unreal Editor Integration Guide

## Generate the greybox level

1. Compile the `HunterGame` C++ module in Unreal Editor.
2. Run **Tools → Execute Python Script** and select `Tools/unreal/create_chapter06_level.py`.
3. The script creates `/Game/Maps/Chapter06/L_CH06_TrialOfSteel` with a native arena, PlayerStart, boss encounter, approval seal, lighting and NavMesh bounds.
4. Build navigation, press **P** to inspect the NavMesh, then Play.

## Objective flow

The campaign subsystem starts `CH06_TrialOfSteel` on chapter entry.

- Enter the elite encounter radius to spawn the native multi-phase `HunterBossBase`.
- Defeat the elite to report `SteelTrialOpponentDefeated`.
- Approach the guild approval seal and press **E** to report `SteelTrialApproved`.
- Reach the chapter exit after both objectives complete. The exit grants 450 XP and 220 currency, then saves Chapter 6.

This remains a greybox trial. The elite currently uses the native boss blockout and phase foundation; bespoke weapon style, skeletal mesh, phase attacks, animation, VFX/audio and arena art remain outstanding. A NavMesh must cover the arena for AI pursuit.

## Chapter 5 to Chapter 6 travel

The Chapter 5 generator now configures its exit to open `/Game/Maps/Chapter06/L_CH06_TrialOfSteel` after Chapter 5 completion and successful save. Regenerate Chapter 5 after compiling the updated module so the placed exit receives the destination.

## Acceptance checklist

- [ ] Generate Chapter 5 and Chapter 6 maps in Unreal Editor.
- [ ] Complete Chapter 5 and confirm it transitions to Chapter 6 after saving.
- [ ] Defeat the elite and verify `SteelTrialOpponentDefeated` completes.
- [ ] Press E at the approval seal and verify `SteelTrialApproved` completes once.
- [ ] Confirm the exit is gated until both objectives complete and saves progress.

## Current boundary

The generator is statically validated in CI, but it has not been executed in Unreal Editor here. Unreal compilation, map generation, AI navigation, boss phase behavior, objective delivery and campaign travel still require editor/runtime testing.
