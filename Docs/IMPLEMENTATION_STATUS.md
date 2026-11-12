# HUNTER 2.0 — Implementation Status

This repository is an Unreal Engine 5 C++ project scaffold. This document separates committed code from systems that still require assets, editor setup, integration, and runtime validation.

## Committed C++ foundations
- Arin Vale third-person character: camera boom, follow camera, movement, jump, sprint, dodge and sword attack montage hook.
- Arin progression component: five evolution stages, experience thresholds, stage profiles and ability-unlock queries.
- Progression component attached to Arin as a default subobject and exposed to Blueprints.
- Reusable monster character base: health, damage handling, player detection, AI-controller pursuit, attack cooldown, death event and Blueprint hooks.
- 20-chapter original campaign subsystem: chapter briefings, recommended evolution stage, chapter unlock/completion state and Blueprint delegates.
- Save-game data object and campaign save/load functions for chapter completion/current chapter and Arin experience/evolution stage.
- Blender procedural static blockout generator for Arin Vale.
- Character rigging/animation production specification.

## Current gameplay limitations
- The sword attack is still a montage hook. There is no production sword mesh, socket setup, hit trace, combo chain, hit-stop, damage reaction or finished combat animation.
- The monster base requires Blueprint enemy children with meshes, animation blueprints, configured AI controllers, a navigation mesh and attack/death presentation.
- The campaign subsystem tracks chapter progression and briefings. It does not yet implement level maps, objectives, dialogue, cutscenes, rewards, boss encounters or mission scripting.
- Save/load is implemented as callable subsystem functions. The game UI and automatic save checkpoints still need wiring.
- Evolution stage profiles expose multipliers, but gameplay stats and modular armor/FX are not yet automatically driven by those profiles.
- Arin's current Blender blockout is static primitive geometry, not a finished rigged hero asset.

## Required Unreal Editor integration
1. Generate project files and compile the HunterGame module in the target Unreal Engine version.
2. Create a playable test map and assign Arin's character class as the player pawn.
3. Configure the legacy input mappings used by the current character code: MoveForward, MoveRight, Turn, LookUp, Jump, Sprint, Dodge and SwordAttack.
4. Create a NavMeshBoundsVolume and test a Blueprint child of AHunterMonsterBase with an AIController.
5. Add a player health/damage system before treating enemy attacks as complete combat.
6. Wire chapter UI and mission completion events to UHunterCampaignSubsystem.
7. Add save/load UI or checkpoint calls to the campaign subsystem.
8. Import final skeletal mesh, materials, animations, weapon and modular armor assets; configure sockets and animation notifies.

## Validation checklist
- Clean C++ build in the exact Unreal Engine version used by the project.
- Launch the map and verify player possession, camera and input mappings.
- Verify all five progression thresholds, stage-change events and ability queries.
- Verify monster pursuit, attack range, damage, death and despawn with navigation enabled.
- Complete chapters in order and verify unlocking, replay rules and final-chapter behavior.
- Save, close, reopen and load a slot; verify chapter state and Arin progression.
- Profile performance on the actual minimum target PC.

No Unreal Editor build or gameplay test is claimed by this status file. The code must be compiled and exercised in Unreal before being called production-ready.