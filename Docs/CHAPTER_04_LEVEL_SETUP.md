# Chapter 04 — The Transit Gate: Unreal Editor Integration Guide

## Generate the greybox level

1. Compile the `HunterGame` C++ module in Unreal Editor.
2. Run **Tools → Execute Python Script** and select `Tools/unreal/create_chapter04_level.py`.
3. The script creates `/Game/Maps/Chapter04/L_CH04_TransitGate` with a native arena, PlayerStart, reach marker, boss encounter actor, lighting and NavMesh bounds.
4. Build navigation, press **P** to inspect the NavMesh, then Play.

## Objective flow

The campaign subsystem starts `CH04_TransitGate` on chapter entry.

- Approach the sealed transit-gate marker to report `TransitGateReached`.
- Enter the guardian's activation radius to spawn the native `HunterBossBase`.
- Defeat the guardian. The encounter actor reports `TransitGuardianDefeated` once on the boss death event.
- Reach the chapter exit after both objectives are complete. The exit completes and saves Chapter 4.

The guardian uses the native boss foundation, including health-threshold phase transitions and escalating attack damage. This is still a blockout boss: its final skeletal mesh, bespoke phase attacks, animation, telegraphs, sound, VFX and balance are not authored. A NavMesh must cover the arena for AI pursuit.

## Chapter 3 to Chapter 4 travel

The Chapter 3 generator now configures its exit to open `/Game/Maps/Chapter04/L_CH04_TransitGate` after Chapter 3 completion and successful save. Regenerate Chapter 3 after compiling the updated module so the placed exit receives the destination.

## Acceptance checklist

- [ ] Generate Chapter 3 and Chapter 4 maps in Unreal Editor.
- [ ] Complete Chapter 3 and confirm it transitions to Chapter 4 after saving.
- [ ] Confirm the quest tracker shows the transit-gate and guardian objectives.
- [ ] Approach the gate marker and verify `TransitGateReached` completes.
- [ ] Enter the guardian area and confirm exactly one boss spawns.
- [ ] Damage the boss through its phase thresholds and verify phase changes.
- [ ] Defeat the boss and verify `TransitGuardianDefeated` completes once.
- [ ] Confirm the exit is gated until both objectives complete and saves progress.

## Current boundary

The generator is committed and statically validated in CI, but it has not been executed in Unreal Editor here. Unreal compilation, boss spawn behavior, AI navigation, phase changes, objective event delivery and campaign travel still require editor/runtime testing.
