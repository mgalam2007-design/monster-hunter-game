# HUNTER 2.0 — Implementation Status

This repository is an Unreal Engine 5 C++ project scaffold. This document distinguishes committed code foundations from assets, editor integration, and runtime validation that remain outstanding. A lightweight GitHub Actions workflow checks Blender script syntax and the Unreal project descriptor; it does not compile Unreal C++.

## Committed C++ foundations
- Arin Vale third-person character with camera, movement, jump, sprint, dodge, sword attack montage hook, progression, health and weapon-trace components. Defeat handling now closes the inventory UI, restores game-only input/cursor state, blocks movement and camera input while dead, and safely handles a missing movement component during no-checkpoint respawn.
- Arin progression component with five evolution stages, XP thresholds, stage profiles, exact save restoration and ability-unlock queries.
- Player health component with damage, healing, health percentage and death events; Arin routes Unreal damage intake into it.
- Socket-based melee trace component with per-swing hit de-duplication and point-damage application.
- Reusable monster base with health, player detection, AI-controller pursuit, attack cooldown, damage, death event and Blueprint hooks; configurable per-monster loot tables can spawn inventory pickups on death with individual drop chances.
- Boss base extending the monster foundation with configurable health thresholds, phase-change events and escalating attack damage; phase transitions stop once the boss has been defeated.
- Reusable boss encounter actor spawns a configured boss at a relative transform when Arin enters its trigger, prevents duplicate starts, registers a configurable BossDefeated quest event, and exposes Blueprint events for start, spawn failure and boss defeat.
- Quest subsystem with data-driven objectives (defeat, interact, reach, collect, survive), tagged event progress, completion delegates and validated progress restoration.
- Reusable world-space quest trigger actor that reports configured tagged events when Arin enters a level volume, with one-shot, repeatable and destroy-after-trigger options.
- Stackable inventory component attached to Arin, with item types, consumable health/energy restore data, add/remove operations, quantity queries, change events and save/restore support.
- Reusable world pickup actor with overlap collection, inventory insertion, configurable tagged quest-event reporting and Blueprint pickup presentation hook.
- Reusable breach encounter director starts a configured enemy wave when Arin enters its trigger, tracks enemy death delegates and spawns a configured quest pickup after the wave is cleared; spawn points, fallback count, reward item and quest event tag are configurable, with Blueprint encounter presentation events.
- Reusable checkpoint actor automatically saves campaign, progression, inventory, equipment, currency, active quest state and Arin's world transform when Arin enters its activation radius; save slot, user index, checkpoint ID and one-time activation are configurable, with a Blueprint success/failure presentation event.
- Reusable chapter-exit actor attempts to complete the active chapter when Arin enters its exit volume, respects campaign completion rules (including Chapter 1's required quest), grants configurable XP and currency rewards, then autosaves the updated campaign; if autosave fails, a retry saves the already-committed completion without advancing another chapter or granting rewards twice. Exposes configurable save slot and Blueprint completion/save-result event.
- Weapon/armor equipment component with inventory-backed equip/unequip, evolution-stage requirements, weapon damage bonuses and armor damage reduction.
- Arin's melee damage now scales from his evolution stage and equipped weapon; incoming damage is reduced by equipped armor.
- Progression-gated ability component with regenerating energy, costs and cooldowns; Energy Strike applies area damage only to HunterMonsterBase enemies (including bosses), Shadow Step launches Arin forward, and Shadow Echo spawns a configurable Blueprint actor.
- Evolution stage changes apply health-capacity, movement-speed and melee-damage multipliers.
- Consumable inventory items can restore health and/or energy; item data stores the restore amounts and Arin consumes one item per use.
- Version 6 save data persists campaign progress, Arin progression, inventory, equipped weapon/armor, active quest objective counters, Arin's currency balance and Arin's world transform; older version 1–5 saves remain accepted.
- Currency wallet component attached to Arin with overflow-safe earning, balance setting, spending checks and Blueprint change events; monster defeats can award configurable currency.
- Shared item catalog Data Asset supports item IDs, inventory data, descriptions, rarity, sell values, UI icons and world meshes; the same definitions can feed pickup, shop and inventory presentation.
- Inventory item definitions now carry weapon damage bonus, armor damage reduction and required evolution stage. Equipment can be equipped directly by item ID from inventory, with the full original item metadata preserved if an equip operation rolls back.
- Blueprint-ready HUD widget data bridge now exposes health, energy, stage-relative XP progress, evolution stage and currency snapshots, and refreshes from gameplay component events.
- Blueprint-ready quest tracker widget reads the active quest, objective descriptions/counts/completion, quest completion state and campaign chapter title/briefing/completion count. It refreshes from quest and campaign delegates; Arin can create a configured tracker Widget Blueprint alongside the HUD.
- Blueprint-ready chapter selection widget exposes all 20 chapter entries, unlock/completion state, current chapter, completed/unlocked counts and campaign completion state. It binds to chapter-change/completion delegates and provides a guarded StartChapterByNumber action; the actual chapter menu visuals and level travel remain to be authored.
- Blueprint-ready save/load widget exposes save-slot existence, chapter progress, XP and currency; it supports explicit save and load actions with Blueprint success/failure events. The visual save menu and multiple-slot management remain to be authored.
- Blueprint-ready inventory widget exposes item stacks, catalog descriptions/rarity/icons, equipped weapon/armor and item totals; it supports equipping by item ID, unequipping either slot and using a selected consumable by item ID when health or energy can be restored. Its snapshot marks consumables that are currently usable. Arin can create the configured inventory Widget Blueprint independently and toggle it with I. Unequipped gear retains its damage, defense and evolution requirements. Arin can create a configured HUD Widget Blueprint on the locally controlled player and add it to the viewport; a Widget Blueprint child still needs to provide the actual visual layout.
- Data-driven vendor component with configurable item catalogs, prices and finite/unlimited stock; supports multi-quantity purchases and sales, prevents selling quest items, checks insufficient funds/stock and guards currency arithmetic overflow. Transactions explicitly receive the interacting hunter, so the vendor actor can remain separate from the player.
- Blueprint-ready vendor shop widget bridge exposes listing data, catalog descriptions/rarity/icons, buy/sell prices, stock, affordability and player balance. It exposes both shop listings and the player's owned inventory for a vendor sell tab, supports purchase/sale calls by item ID and refreshes when shop stock or player currency changes. Affordability state excludes sold-out listings and invalid negative prices. A reusable vendor actor now provides a configurable proximity sphere, E-key interaction, shop-widget initialization, Escape-to-close behavior and a Blueprint proximity event; vendor Blueprints still need a configured shop widget class and catalog.
- Monster defeat rewards now award Arin XP and report configurable tagged events to the active quest subsystem.
- 20-chapter original campaign subsystem with chapter briefings, recommended evolution stages, unlock/completion state, an explicit final-campaign completion query/event and save/load functions. Save restoration filters invalid completed chapter IDs, enforces strictly sequential chapter completion (so gaps cannot unlock later chapters), and clamps the current chapter to the highest unlocked chapter. Chapter 1 now starts an authored quest: defeat three breach creatures and recover one tagged breach shard; its chapter completion is blocked until both objectives are complete. Re-entering Chapter 1 no longer resets partial/completed quest progress, and chapter completion fails closed if the required quest is missing or incomplete.
- Versioned save-game data object for campaign, progression, inventory, equipment, active quest progress and Arin's checkpoint transform; version 1–5 saves remain loadable.
- Blender procedural static blockout generator for Arin Vale and a character rigging/animation production specification.

## Current gameplay limitations
- A reusable boss encounter trigger is now implemented, but the boss system still needs authored boss Blueprints, encounter maps, phase-specific attacks and presentation. Chapter 1 has two authored objective definitions (defeat three creatures and collect one breach shard), but still needs a level, enemy Blueprints and a pickup Blueprint configured with the BreachShardCollected event tag. The new quest trigger actor is a reusable code class; level designers must place/configure instances in Unreal and set matching event tags. Other chapters still need authored quests.
- The monster base needs Blueprint enemy children with meshes, animation blueprints, configured AI controllers, navigation mesh and attack/death presentation. Loot drops require the pickup actor class and per-enemy item/drop chance entries to be configured in those Blueprint children.
- Sword traces require a skeletal mesh with correctly configured weapon_r and weapon_tip sockets; the attack trace window is currently a code-timed fallback, not a polished animation-notify combat pipeline.
- Campaign chapters currently contain titles/briefings and unlock state, not finished level maps, objectives, dialogue, cutscenes or mission scripting. The chapter-selection widget is a data/action bridge only; it does not yet load chapter maps or present a finished menu. The save/load widget operates on one configurable slot and has no finished visual menu or multiple-slot browser.
- No production character/monster skeletal meshes, materials, animations, VFX, audio, authored UMG screens, equipment/loot content or finished environments are committed. The currency wallet is a gameplay foundation; the vendor transaction component, shop data bridge and reusable vendor interaction actor are committed, but vendor Blueprint instances/catalog Data Assets and visual shop UI are still outstanding. The inventory component has no authored item catalog or configured pickup Blueprints; the pickup actor must be configured with an item ID and mesh. Equipment currently changes combat stats but does not yet swap visible weapon/armor meshes. Ability gameplay effects are implemented as simple C++ foundations; Shadow Echo still requires a configured actor Blueprint, and polished animations, targeting, hit reactions, Niagara VFX and audio remain outstanding. Consumables require item entries configured with restoration values.
- Save/load functions persist chapter state, progression, inventory, equipment and active quest objective counters, and the checkpoint actor now saves Arin's world transform for position restoration on load; checkpoint and chapter-exit visuals/Blueprint setup, explicit respawn flow and menu integration still need to be authored in Unreal.
- Arin's Blender output is static primitive blockout geometry, not a finished rigged hero asset.

## Required Unreal Editor integration
1. Generate project files and compile the HunterGame module in the target Unreal Engine version.
2. Create a playable test map and assign Arin's character class as the player pawn.
3. Configure the input mappings used by the current character code: MoveForward, MoveRight, Turn, LookUp, Jump, Sprint, Dodge, SwordAttack, AbilityEnergyStrike, AbilityShadowStep, AbilityShadowEcho and UseConsumable.
4. Import/assign a skeletal mesh and configure weapon_r and weapon_tip sockets.
5. Create Blueprint children for regular monsters and bosses; assign meshes, animation blueprints, AI controllers and attack/death presentation.
6. Add a NavMeshBoundsVolume and verify enemy navigation.
7. Configure pickup Blueprints with item IDs and meshes; create and populate item catalog Data Assets, create vendor Blueprint instances with shop widget classes and populated catalogs, add visual equipment attachments; configure a Shadow Echo actor Blueprint and implement ability animation/VFX; add ability and consumable input bindings; create Widget Blueprint children for the HUD, quest tracker, chapter selection, save/load, inventory and vendor shop data bridges, chapter menu, inventory/equipment, vendor shop and save/load UI; create loot and shop content.
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
