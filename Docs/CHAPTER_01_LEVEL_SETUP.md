# Chapter 01 — First Breach: Unreal Editor Integration Guide

This guide turns the committed C++ foundations into the first editor-authored playable slice. The C++ classes exist in the repository, but the level, Blueprint children, meshes, animations and UI must still be created in Unreal Editor. Do not treat this guide as evidence that those assets already exist.

## 1. Create the level

1. Open the project in the Unreal Engine version configured by the project descriptor.
2. Create a level named `L_CH01_FirstBreach` and save it under `Content/Maps/Chapter01/`.
3. Build a compact abandoned transit-station breach arena:
   - Safe entry corridor and a clear route into the arena.
   - Main combat floor with enough space for three enemies to approach Arin.
   - One visible reward platform or breach-crystal location.
   - Exit corridor beyond the reward.
4. Add lighting, fog and simple collision first. Optimize visual detail after the encounter works.
5. Add a `NavMeshBoundsVolume` covering the walkable arena and corridors. Press **P** in the editor viewport and confirm the walkable floor is green.

## 2. Configure Arin

1. Create a Blueprint child of `ArinValeCharacter`, named `BP_ArinVale`.
2. Assign a compatible skeletal mesh and Animation Blueprint when available. The procedural Arin blockout is only a static starting point and is not a finished skinned character.
3. Configure the project GameMode or level override to spawn `BP_ArinVale` as the default pawn.
4. Assign the HUD, quest tracker and inventory Widget Blueprint classes on the character if those properties are exposed in its Details panel.
5. Ensure the player starts in the safe entry corridor, facing toward the breach arena.

## 3. Create the first enemy

1. Create a Blueprint child of `HunterMonsterBase`, named `BP_BreachStalker`.
2. Assign a skeletal mesh and Animation Blueprint. If final art is not ready, use a temporary clearly visible placeholder mesh; do not leave the enemy invisible.
3. Set initial tuning values:
   - Max Health: 120
   - Detection Radius: 1400
   - Attack Range: 180
   - Attack Damage: 12
   - Attack Cooldown: 1.6
   - Chase Speed: 430
   - Experience Reward: 25
   - Currency Reward: 5
   - Defeat Quest Event Tag: `MonsterDefeated`
4. Set a suitable AI Controller class if required by the project setup and confirm the level has a working NavMesh.
5. Add a simple attack presentation using the Blueprint attack event and an attack montage when one exists.
6. Test one placed enemy before using it in the wave.

## 4. Configure the breach encounter

1. Create a Blueprint child of `HunterBreachEncounterActor`, named `BP_CH01_BreachEncounter`.
2. Set **Monster Class** to `BP_BreachStalker`.
3. Add three **Monster Spawn Points** as transforms placed safely on the arena floor. Keep them separated so enemies do not overlap on spawn.
4. Set **Spawn Interval Seconds** to 1.25.
5. Assign **Reward Pickup Class** to the pickup Blueprint created in the next section.
6. Set **Reward Item**:
   - Item ID: `BreachShard`
   - Item Type: Quest
   - Quantity: 1
   - Max Stack: 1
   - Display Name: Breach Shard
7. Set **Reward Quest Event Tag** to `BreachShardCollected`.
8. Place the encounter actor at the arena entrance and adjust **Trigger Radius** so it starts when Arin commits to entering the arena, not while standing in the safe spawn corridor.
9. Use the Blueprint encounter events for a start cue, completion cue and reward appearance if desired. At runtime, the encounter normalizes the trigger radius (50–5000), spawn interval (0.1–30 seconds) and fallback wave count (1–20); it also limits authored spawn arrays to 20 entries and replaces invalid authored spawn transforms with safe positions around Arin.

The director tracks only successfully spawned enemies. It refuses to start unless the monster class, reward pickup class, reward item ID and quest event tag are configured. The reward is spawned only after all scheduled spawn attempts have finished and all successfully spawned enemies have been defeated. The encounter is marked complete only after the pickup successfully finishes spawning; if pickup spawning fails, the Blueprint failure event fires and `RetryRewardSpawn` can be called after the cause is corrected.

## 5. Create the breach shard pickup

1. Create a Blueprint child of `HunterPickupActor`, named `BP_BreachShardPickup`.
2. Assign a visible static mesh to the inherited Pickup Mesh component. A temporary glowing crystal mesh is acceptable.
3. Set the pickup item to the same `BreachShard` quest item definition used by the encounter.
4. Keep **Destroy After Pickup** enabled.
5. The encounter director sets the pickup's quest event tag to `BreachShardCollected` when it spawns the reward. Do not use a different spelling or capitalization.
6. Add a simple pickup sound or Blueprint pickup event presentation when assets are ready.

## 6. Configure the chapter exit

1. Create a Blueprint child of `HunterChapterExitActor`, named `BP_CH01_Exit`.
2. Place it beyond the reward platform, so the player naturally reaches it after collecting the shard.
3. Keep **Save Slot Name** set to `HunterCampaign` and **User Index** set to 0 unless the project save setup is intentionally changed.
4. Set completion rewards to 100 XP and 50 currency for the first playable pass.
5. Use the exit result Blueprint event to show success or a clear blocked message if Chapter 1's quest is incomplete.
6. The exit only completes Chapter 1 after the active `CH01_FirstBreach` quest is complete. It autosaves after committing completion; if the save fails, retrying the exit saves the same completion without granting the rewards twice.

## 7. Add a checkpoint (recommended)

1. Place an instance of `HunterCheckpointActor` after the safe entry corridor and before the combat arena.
2. Use the same `HunterCampaign` save slot and user index 0.
3. Give the checkpoint a unique ID such as `CH01_Entry`.
4. Confirm the activation radius does not overlap the chapter exit or encounter trigger.

## 8. First-playable acceptance checklist

- [ ] The level opens with Arin controllable and visible.
- [ ] Arin can move, turn, jump, dodge and attack without getting stuck.
- [ ] The breach encounter starts only once when Arin enters its trigger.
- [ ] Three enemies spawn at the configured interval and can navigate toward Arin.
- [ ] Each enemy can damage Arin and can be defeated by Arin.
- [ ] Each defeat advances the `MonsterDefeated` objective.
- [ ] The breach shard appears only after the wave is finished and all spawned enemies are defeated.
- [ ] Collecting the shard adds the item and advances `BreachShardCollected`.
- [ ] The quest tracker shows both objectives complete.
- [ ] The chapter exit refuses completion before the quest is complete.
- [ ] After collecting the shard, the exit completes Chapter 1 and saves progress.
- [ ] Loading the save restores chapter progress, inventory, equipment, currency, quest state and Arin's saved transform.
- [ ] No critical Blueprint, AI, collision, animation or save errors appear in the Output Log.

## 9. Performance notes for the first pass

Keep the first level compact, use a small number of dynamic lights, limit expensive post-processing and start with simple materials. The current target PC configuration may struggle with a full UE5 editor session; use low scalability settings and avoid treating editor frame rate as final game performance.

## Current boundary

This is an implementation recipe, not a completed level. The repository currently contains the C++ actor/component foundations, but the Unreal Editor work above, final art assets and actual playtest are still outstanding.

## 1A. Use the code-generated greybox arena

The repository now includes `AHunterPrototypeArenaActor`, which creates a temporary enclosed arena from Unreal's built-in cube mesh.

1. In the Content Browser, enable **C++ Classes** in the Sources panel and locate `HunterPrototypeArenaActor` under the HunterGame module's World folder.
2. Drag `HunterPrototypeArenaActor` into `L_CH01_FirstBreach`.
3. In Details, start with **Arena Half Width = 1800**, **Arena Half Depth = 1800**, and **Wall Height = 600**.
4. Move the actor so its floor sits at the level's intended ground height. Its floor is slightly below the actor origin.
5. Keep the player start inside the arena, away from the walls and gate frame. Put the encounter trigger near the center and spawn points around the open floor.
6. Add or resize a `NavMeshBoundsVolume` so it covers the entire floor. Press **P** and verify green navigation coverage; if pillars or walls obstruct routes, adjust their placement or the NavMesh bounds.
7. Press Play and verify the arena blocks Arin and enemies correctly. The blue gate light is only a temporary visual marker.

This actor supplies collision geometry and a simple layout only. It does not create a level asset, player start, NavMesh volume, enemies, quest triggers, materials or final environment art automatically.

