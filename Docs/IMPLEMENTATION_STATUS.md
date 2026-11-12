# HUNTER 2.0 — Implementation Status

This repository is an Unreal Engine 5 C++ project scaffold. This document distinguishes committed code foundations from assets, editor integration, and runtime validation that remain outstanding. A lightweight GitHub Actions workflow checks Blender script syntax and the Unreal project descriptor; it does not compile Unreal C++.

## Committed C++ foundations
- Arin Vale third-person character with camera, movement, jump, sprint, dodge, sword attack montage hook, progression, health and weapon-trace components.
- Arin progression component with five evolution stages, XP thresholds, stage profiles, exact save restoration and ability-unlock queries.
- Player health component with damage, healing, health percentage and death events; Arin routes Unreal damage intake into it.
- Socket-based melee trace component with per-swing hit de-duplication and point-damage application.
- Reusable monster base with health, player detection, AI-controller pursuit, attack cooldown, damage, death event and Blueprint hooks; configurable per-monster loot tables can spawn inventory pickups on death with individual drop chances.
- Boss base extending the monster foundation with configurable health thresholds, phase-change events and escalating attack damage.
- Quest subsystem with data-driven objectives (defeat, interact, reach, collect, survive), tagged event progress, completion delegates and validated progress restoration.
- Reusable world-space quest trigger actor that reports configured tagged events when Arin enters a level volume, with one-shot, repeatable and destroy-after-trigger options.
- Stackable inventory component attached to Arin, with item types, consumable health/energy restore data, add/remove operations, quantity queries, change events and save/restore support.
- Reusable world pickup actor with overlap collection, inventory insertion, optional tagged quest-event reporting and Blueprint pickup presentation hook.
- Weapon/armor equipment component with inventory-backed equip/unequip, evolution-stage requirements, weapon damage bonuses and armor damage reduction.
- Arin's melee damage now scales from his evolution stage and equipped weapon; incoming damage is reduced by equipped armor.
- Progression-gated ability component with regenerating energy, costs and cooldowns; Energy Strike applies area damage, Shadow Step launches Arin forward, and Shadow Echo spawns a configurable Blueprint actor.
- Evolution stage changes apply health-capacity, movement-speed and melee-damage multipliers.
- Consumable inventory items can restore health and/or energy; item data stores the restore amounts and Arin consumes one item per use.
- Version 5 save data persists campaign progress, Arin progression, inventory, equipped weapon/armor, active quest objective counters and Arin's currency balance; older version 1–4 saves remain accepted.
- Currency wallet component attached to Arin with overflow-safe earning, balance setting, spending checks and Blueprint change events; monster defeats can award configurable currency.
- Shared item catalog Data Asset supports item IDs, inventory data, descriptions, rarity, sell values, UI icons and world meshes; the same definitions can feed pickup, shop and inventory presentation.
- Inventory item definitions now carry weapon damage bonus, armor damage reduction and required evolution stage. Equipment can be equipped directly by item ID from inventory, with the full original item metadata preserved if an equip operation rolls back.
- Blueprint-ready HUD widget data bridge now exposes health, energy, stage-relative XP progress, evolution stage and currency snapshots, and refreshes from gameplay component events. A Widget Blueprint child still needs to provide the actual visual layout.
- Data-driven vendor component with configurable item catalogs, prices and finite/unlimited stock; supports multi-quantity purchases and sales, prevents selling quest items, checks insufficient funds/stock and guards currency arithmetic overflow. Transactions explicitly receive the interacting hunter, so the vendor actor can remain separate from the player.
- Monster defeat rewards now award Arin XP and report configurable tagged events to the active quest subsystem.
- 20-chapter original campaign subsystem with chapter briefings, recommended evolution stages, unlock/completion state and save/load functions. Chapter 1 now starts an authored quest: defeat three breach creatures and recover one tagged breach shard; its chapter completion is blocked until both objectives are complete. Re-entering Chapter 1 no longer resets partial/completed quest progress, and chapter completion fails closed if the required quest is missing or incomplete.
- Versioned save-game data object for campaign, progression, inventory, equipment and active quest progress; version 1–4 saves remain loadable.
- Blender procedural static blockout generator for Arin Vale and a character rigging/animation production specification.

## Current gameplay limitations
- The boss system remains a code foundation without authored boss Blueprints or encounter maps. Chapter 1 has two authored objective definitions (defeat three creatures and collect one breach shard), but still needs a level, enemy Blueprints and a pickup Blueprint configured with the BreachShardCollected event tag. The new quest trigger actor is a reusable code class; level designers must place/configure instances in Unreal and set matching event tags. Other chapters still need authored quests.
- The monster base needs Blueprint enemy children with meshes, animation blueprints, configured AI controllers, navigation mesh and attack/death presentation. Loot drops require the pickup actor class and per-enemy item/drop chance entries to be configured in those Blueprint children.
- Sword traces require a skeletal mesh with correctly configured weapon_r and weapon_tip sockets; the attack trace window is currently a code-timed fallback, not a polished animation-notify combat pipeline.
- Campaign chapters currently contain titles/briefings and unlock state, not finished level maps, objectives, dialogue, cutscenes or mission scripting.
- No production character/monster skeletal meshes, materials, animations, VFX, audio, authored UMG screens, equipment/loot content or finished environments are committed. The currency wallet is a gameplay foundation; the vendor transaction component is committed, but vendor actors/catalog Data Assets, shop UI and interaction flow are still outstanding. The inventory component has no authored item catalog, configured pickup Blueprints, shop or UI yet; the pickup actor must be configured with an item ID and mesh. Equipment currently changes combat stats but does not yet swap visible weapon/armor meshes. Ability gameplay effects are implemented as simple C++ foundations; Shadow Echo still requires a configured actor Blueprint, and polished animations, targeting, hit reactions, Niagara VFX and audio remain outstanding. Consumables require item entries configured with restoration values.
- Save/load functions persist chapter state, progression, inventory, equipment and active quest objective counters, but checkpoint and menu integration still need to be authored in Unreal.
- Arin's Blender output is static primitive blockout geometry, not a finished rigged hero asset.

## Required Unreal Editor integration
1. Generate project files and compile the HunterGame module in the target Unreal Engine version.
2. Create a playable test map and assign Arin's character class as the player pawn.
3. Configure the input mappings used by the current character code: MoveForward, MoveRight, Turn, LookUp, Jump, Sprint, Dodge, SwordAttack, AbilityEnergyStrike, AbilityShadowStep, AbilityShadowEcho and UseConsumable.
4. Import/assign a skeletal mesh and configure weapon_r and weapon_tip sockets.
5. Create Blueprint children for regular monsters and bosses; assign meshes, animation blueprints, AI controllers and attack/death presentation.
6. Add a NavMeshBoundsVolume and verify enemy navigation.
7. Configure pickup Blueprints with item IDs and meshes; create and populate item catalog Data Assets, configure vendor Blueprint catalogs and interaction flow, add visual equipment attachments; configure a Shadow Echo actor Blueprint and implement ability animation/VFX; add ability and consumable input bindings; create a Widget Blueprint child for the HUD data bridge, build the quest tracker, chapter menu, inventory/equipment, vendor shop and save/load UI; create loot and shop content.
8. Create and populate the 20 campaign maps, objectives, encounters, dialogue, rewards and boss fights.
9. Import final Arin model, materials, animations, weapon and modular armor assets; configure animation notifies and VFX.

## Validation checklist
- Clean C++ build in the exact Unreal Engine version used by the project.
- Launch a test map and verify player possession, camera and input mappings.
- Verify all five progression thresholds, stage-change events, exact save restoration and ability queries.
- Verify health, healing, death and damage intake.
- Verify sword socket traces hit enemies once per swing and respect collision channels.
- Verify monster pursuit, attacks, damage, death and despawn with navigation enabled.
- Verify boss phase transitions and attack scaling at configured health thresholds.
- Verify quest objective event matching, counters, one-time quest completion, chapter-one completion gating and save/restore of partial quest progress.
- Complete chapters in order and verify unlocking, saving and loading.
- Profile performance on the actual minimum target PC.

No Unreal Editor build or gameplay test is claimed by this status file. The code must be compiled and exercised in Unreal before being called production-ready.
