# HUNTER 2.0 — Implementation Status

This repository is an Unreal Engine 5 C++ project scaffold. This document distinguishes committed code foundations from assets, editor integration, and runtime validation that remain outstanding. A lightweight GitHub Actions workflow checks Blender script syntax and the Unreal project descriptor; it does not compile Unreal C++.

## Committed C++ foundations
- Arin Vale third-person character with camera, movement, jump, sprint, dodge, sword attack montage hook, progression, health and weapon-trace components. Defeat handling closes the inventory UI, restores game-only input/cursor state, blocks movement and camera input while dead, and safely handles a missing movement component during no-checkpoint respawn. Respawn now resets sprint speed to the progression-adjusted walk speed, clears stale attack/dodge/weapon-trace timers, and ends any active melee trace; MoveRight also rejects non-finite axis input.
- Arin progression component with five evolution stages, XP thresholds, stage profiles, exact save restoration and ability-unlock queries.
- Player health component with damage, healing, health percentage and death events; Arin routes Unreal damage intake into it.
- Socket-based melee trace component with per-swing hit de-duplication and point-damage application.
- Reusable monster base with health, player detection, AI-controller pursuit, attack cooldown, damage, death event and Blueprint hooks; configurable per-monster loot tables can spawn inventory pickups on death with individual drop chances.
- Boss base extending the monster foundation with configurable health thresholds, phase-change events and escalating attack damage; phase transitions stop once the boss has been defeated.
- Reusable boss encounter actor spawns a configured boss at a relative transform when Arin enters its trigger, prevents duplicate starts, registers a configurable BossDefeated quest event, and exposes Blueprint events for start, spawn failure and boss defeat.
- Quest subsystem with data-driven objectives (defeat, interact, reach, collect, survive), tagged event progress, completion and quest-state lifecycle delegates (including start, clear and restore) and validated progress restoration; objective updates are applied before Blueprint notifications, with a quest-identity guard against listeners clearing or replacing the active quest during callbacks; objective type enum values, IDs, event tags and required counts are validated on both quest start and save restoration. Starting a new quest is rejected while another quest is active so unfinished progress cannot be silently overwritten.
- Reusable world-space quest trigger actor that reports configured tagged events when Arin enters a level volume, with one-shot, repeatable and destroy-after-trigger options.
- Stackable inventory component attached to Arin, with item types, consumable health/energy restore data, add/remove operations, quantity queries, change events and save/restore support. Incoming item data is validated for finite numeric metadata, bounded armor/stage values and aggregate quantity overflow; stacks sharing an item ID must have matching normalized metadata, checked before any quantity is mutated; incoming values are normalized before comparison so valid clamped items can merge with existing stacks. AddItem builds a complete candidate inventory and commits it before broadcasting one change event. RestoreItems now independently rebuilds and validates the complete saved inventory, skips malformed/conflicting/overflowing entries, commits once, and broadcasts only after restoration is complete.
- Reusable world pickup actor with overlap collection, inventory insertion, configurable tagged quest-event reporting and Blueprint pickup presentation hook.
- Reusable breach encounter director starts a configured enemy wave when Arin enters its trigger, spawns enemies at a configurable interval, tracks enemy death delegates and spawns a configured quest pickup only after every scheduled spawn and enemy defeat; spawn points, fallback count, spawn interval, reward item and quest event tag are configurable, with Blueprint encounter presentation events.
- Reusable checkpoint actor automatically saves campaign, progression, inventory, equipment, currency, active quest state and Arin's world transform when Arin enters its activation radius; save slot, user index, checkpoint ID and one-time activation are configurable, with a Blueprint success/failure presentation event. Checkpoint and chapter-exit trigger radii are normalized to finite minimum sizes at runtime, and both save interactions reject empty slot names. Campaign checkpoint saves now also reject invalid current-chapter values and non-finite Arin transforms before creating a save object, and persist the exact transform that passed validation.
- Reusable chapter-exit actor attempts to complete the active chapter when Arin enters its exit volume, respects campaign completion rules (including Chapter 1's required quest), grants configurable XP and currency rewards, then autosaves the updated campaign; if autosave fails, a retry saves the already-committed completion without advancing another chapter or granting rewards twice. Exposes configurable save slot and Blueprint completion/save-result event.
- Weapon/armor equipment component with inventory-backed equip/unequip, evolution-stage requirements (with fail-closed behavior if progression is unavailable), weapon damage bonuses and armor damage reduction; restored equipment is checked for the expected slot, finite stats and Arin's restored evolution-stage requirement, then clamped to safe combat ranges; campaign loading restores progression before equipment validation. EquipItem now treats the matching inventory entry as authoritative for item type, damage stats, defense and evolution requirement, preventing Blueprint callers from fabricating stronger equipment metadata for a valid item ID. A transaction guard also rejects nested equip/unequip calls triggered by inventory or equipment delegates; failed rollback insertion is logged explicitly for recovery. Equipment now also rejects non-finite or out-of-range combat metadata in the authoritative inventory entry, and damage/defense getters fail safely to zero if corrupted runtime values are encountered.
- Arin's melee damage now scales from his evolution stage and equipped weapon; incoming damage is reduced by equipped armor.
- Progression-gated ability component with regenerating energy, costs and cooldowns; Energy Strike applies area damage only to HunterMonsterBase enemies (including bosses), Shadow Step launches Arin forward, and Shadow Echo spawns a configurable Blueprint actor.
- Evolution stage changes apply health-capacity, movement-speed and melee-damage multipliers.
- Consumable inventory items can restore health and/or energy; item data stores the restore amounts and Arin consumes one item per use.
- Version 6 save data persists campaign progress, Arin progression, inventory, equipped weapon/armor, active quest objective counters, Arin's currency balance and Arin's world transform; older version 1–5 saves remain accepted. Load now ignores an invalid/non-finite saved transform and safely handles a missing game-instance quest subsystem instead of dereferencing it.
- Currency wallet component attached to Arin with overflow-safe earning, balance setting, spending checks and Blueprint change events; wallet mutations are guarded during notifications so Blueprint listeners cannot recursively alter the balance in the middle of a transaction; monster defeats can award configurable currency.
- Shared item catalog Data Asset supports item IDs, inventory data, descriptions, rarity, sell values, UI icons and world meshes; the same definitions can feed pickup, shop and inventory presentation.
- Inventory item definitions now carry weapon damage bonus, armor damage reduction and required evolution stage. Equipment can be equipped directly by item ID from inventory, with the full original item metadata preserved if an equip operation rolls back.
- Blueprint-ready HUD widget data bridge now exposes health, energy, stage-relative XP progress, evolution stage and currency snapshots, and refreshes from gameplay component events.
- Blueprint-ready quest tracker widget reads the active quest, objective descriptions/counts/completion, quest completion state and campaign chapter title/briefing/completion count. It refreshes from objective, completion, quest-state lifecycle and campaign delegates, including when a quest is cleared during load; Arin can create a configured tracker Widget Blueprint alongside the HUD.
- Blueprint-ready chapter selection widget exposes all 20 chapter entries, unlock/completion state, current chapter, completed/unlocked counts and campaign completion state. It binds to chapter-change/completion delegates and provides a guarded StartChapterByNumber action; the actual chapter menu visuals and level travel remain to be authored.
- Blueprint-ready save/load widget exposes save-slot existence, chapter progress, XP and currency; it supports explicit save and load actions with Blueprint success/failure events. The visual save menu and multiple-slot management remain to be authored.
- Blueprint-ready inventory widget exposes item stacks, catalog descriptions/rarity/icons, equipped weapon/armor and item totals; it supports equipping by item ID, unequipping either slot and using a selected consumable by item ID when health or energy can be restored. Its snapshot marks consumables that are currently usable. Arin can create the configured inventory Widget Blueprint independently and toggle it with I. Unequipped gear retains its damage, defense and evolution requirements. Arin can create a configured HUD Widget Blueprint on the locally controlled player and add it to the viewport; a Widget Blueprint child still needs to provide the actual visual layout.
- Data-driven vendor component with configurable item catalogs, prices and finite/unlimited stock; supports multi-quantity purchases and sales, prevents buying or selling quest-critical items, rejects duplicate catalog IDs rather than choosing ambiguous prices/stock, validates item enum and finite gameplay metadata before charging a buyer, checks insufficient funds/stock and guards currency arithmetic overflow. Transactions explicitly receive the interacting hunter, so the vendor actor can remain separate from the player. A transaction reentrancy guard now blocks nested buy/sell calls triggered by currency, inventory or shop event listeners while a transaction is still being applied. Buy transactions now check whether a refund succeeds when inventory insertion fails and log an explicit error if the wallet cannot be restored; sell transactions likewise restore the sold item if payout fails and log a failed item rollback. A dedicated `Tools/validate_shop.py` static validator now checks the buy/sell transaction reentrancy guards, refund and item-rollback failure paths, error logging and stock-update ordering; GitHub Actions runs it alongside campaign and project-descriptor checks. These are compensating rollback attempts, not a guarantee of cross-component atomicity under arbitrary Blueprint callbacks.
- Blueprint-ready vendor shop widget bridge exposes listing data, catalog descriptions/rarity/icons, buy/sell prices, stock, affordability and player balance. It exposes both shop listings and the player's owned inventory for a vendor sell tab, supports purchase/sale calls by item ID and refreshes when shop stock or player currency changes. Affordability state excludes sold-out listings and invalid negative prices. A reusable vendor actor now provides a configurable proximity sphere, E-key interaction, shop-widget initialization, Escape-to-close behavior and a Blueprint proximity event; vendor Blueprints still need a configured shop widget class and catalog.
- Monster defeat rewards now award Arin XP and report configurable tagged events to the active quest subsystem.
- 20-chapter original campaign subsystem with chapter briefings, recommended evolution stages, unlock/completion state, an explicit final-campaign completion query/event and save/load functions. Save restoration filters invalid completed chapter IDs, enforces strictly sequential chapter completion (so gaps cannot unlock later chapters), and resumes at a playable incomplete chapter rather than a chapter already marked complete. Chapter 1 start and completion fail cleanly if the quest subsystem is unavailable or another active quest prevents initialization; save/load also safely handles an unavailable quest subsystem. Chapter 1 now starts an authored quest: defeat three breach creatures and recover one tagged breach shard; its chapter completion is blocked until both objectives are complete. Re-entering Chapter 1 no longer resets partial/completed quest progress, and chapter completion fails closed if the required quest is missing or incomplete. Chapter 2 now starts `CH02_MissingSurvivors`: reach a survivor-district trigger (`SurvivorDistrictLocated`) and rescue three civilians through trigger events (`SurvivorRescued`); its exit is gated on that exact quest being complete. Starting it clears a completed prior chapter quest but refuses to overwrite unfinished quest progress. Chapter 3 now starts `CH03_GuildEvaluation`: defeat five trial opponents (`TrialOpponentDefeated`) and activate the arena seal (`TrialSealActivated`); its exit is gated on that exact quest being complete. Starting it clears a completed prior chapter quest but refuses to overwrite unfinished quest progress. Chapter 4 now starts `CH04_TransitGate`: reach the sealed gate (`TransitGateReached`) and defeat its guardian (`TransitGuardianDefeated`); its exit is gated on that exact quest being complete. Chapter 5 now starts `CH05_MiningContract`: defeat four settlement creatures (`MiningCreatureDefeated`) and collect three corrupted ore samples (`CorruptedOreCollected`); its exit is gated on that exact quest being complete. Chapter 6 now starts `CH06_TrialOfSteel`: defeat the elite trial opponent (`SteelTrialOpponentDefeated`) and activate the guild approval seal (`SteelTrialApproved`); its exit is gated on that exact quest being complete. Chapter 7 now starts `CH07_BlueMark`: locate the source of Arin's mark (`BlueMarkSourceLocated`) and collect three resonance fragments (`ResonanceFragmentCollected`); its exit is gated on that exact quest being complete. Chapter 8 now starts `CH08_HunterReborn`: defeat six awakening expedition creatures (`AwakeningCreatureDefeated`) and activate the awakening altar (`AwakeningAltarActivated`); its exit is gated on that exact quest being complete. Chapter 9 now starts `CH09_VanishedHunters`: locate the missing hunters' signal (`MissingHunterSignalLocated`), defeat four gate stalkers (`GateStalkerDefeated`) and recover the expedition recorder (`ExpeditionRecorderRecovered`); its exit is gated on that exact quest being complete. Chapter 10 now starts `CH10_HollowDistrict`: reach the district core (`HollowDistrictCoreReached`), destroy three shadow anchors (`ShadowAnchorDestroyed`) and defeat the Hollow Warden (`HollowWardenDefeated`); its exit is gated on that exact quest being complete. Chapter 11 now starts `CH11_EchoesOfTheFallen`: recover three expedition memory echoes (`ExpeditionMemoryRecovered`) and defeat the fallen captain (`FallenCaptainDefeated`); its exit is gated on that exact quest being complete. Chapter 12 now starts `CH12_ShadowAccord`: meet the faction representative (`ShadowContactReached`), defeat three shadow sentinels (`ShadowSentinelDefeated`) and resolve the pact at its altar (`ShadowPactResolved`); its exit is gated on that exact quest being complete. Chapter 13 now starts `CH13_SiegeOfTheNorth`: defeat five siege creatures (`NorthernSiegeCreatureDefeated`) and reactivate two fortress beacons (`FortressBeaconActivated`); its exit is gated on that exact quest being complete. Chapter 14 now starts `CH14_BrokenCitadel`: destroy three command pylons (`CitadelPylonDestroyed`) and defeat the siege commander (`CitadelCommanderDefeated`); its exit is gated on that exact quest being complete. Chapter 15 now starts `CH15_WorldsUnsealed`: stabilize four gate anchors (`BreachAnchorStabilized`) and defeat two breach guardians (`BreachGuardianDefeated`); its exit is gated on that exact quest being complete. Chapter 16 now starts `CH16_SevenBeacons`: activate seven boundary beacons (`BoundaryBeaconActivated`) and defeat three attackers (`BeaconAttackerDefeated`); its exit is gated on that exact quest being complete. Chapter 17 now starts `CH17_LastSanctuary`: evacuate five civilian groups (`SanctuaryGroupEvacuated`) and defeat six invaders (`SanctuaryInvaderDefeated`); its exit is gated on that exact quest being complete. Chapters 1-20 now have authored quest definitions and exact-quest completion gates. Chapter 18 `CH18_CrownOfRuin` requires four `ThroneWardenDefeated` events and three `RuinCommandCoreDestroyed` events; Chapter 19 `CH19_BeyondTheGate` requires three `OriginGateSealStabilized` events and two `OriginSentinelDefeated` events; Chapter 20 `CH20_TheFinalHunter` requires one `OriginHeartExposed` event and one `FinalHunterDefeated` event. Completing Chapter 20 uses the existing final-campaign completion path.
- Blender procedural static blockout generator for Arin Vale and a character rigging/animation production specification.
- Chapter 1 first-playable Unreal Editor integration guide covering the breach level, Arin setup, enemy Blueprint, paced encounter, shard pickup, chapter exit, checkpoint and acceptance checklist. A companion Chapters 02–20 level assembly guide now gives exact map-name suggestions, actor configuration recipes, objective counts, event tags, mission-order cautions and per-map acceptance tests. These remain editor instructions, not completed Unreal assets.

- Save-slot snapshot validation now rejects unsupported versions, chapter numbers outside 1–20, negative XP/currency, oversized completion lists, invalid chapter IDs and duplicate completed chapters before exposing metadata to the Blueprint menu. `Tools/validate_save_snapshot.py` checks these source safeguards and runs in GitHub Actions; this is a static source check, not a runtime save-file test.

- Arin now removes a consumables stack before applying its health/energy effect and aborts if inventory removal fails, preventing a failed removal from granting a free effect. The inventory menu also refuses to open while Arin is dead or awaiting respawn. `Tools/validate_character_safety.py` checks these source safeguards in CI; runtime behavior still requires Unreal testing.

## Current gameplay limitations
- A reusable boss encounter trigger is now implemented, but the boss system still needs authored boss Blueprints, encounter maps, phase-specific attacks and presentation. The Chapter 1 breach director now spaces enemy spawns over time; its encounter and reward still need to be configured in a playable level. Chapter 1 has two authored objective definitions (defeat three creatures and collect one breach shard), but still needs a level, enemy Blueprints and a pickup Blueprint configured with the BreachShardCollected event tag. The new quest trigger actor is a reusable code class; level designers must place/configure instances in Unreal and set matching event tags. Chapter 18-20 mission triggers, throne wardens, ruin command cores, origin-gate seals, origin sentinels, origin-heart seal and final enemy still need to be placed/configured in their respective Unreal maps; Chapter 2-17 objective triggers, pickups, trial seals, resonance fragments, awakening altar, missing-hunter signal, gate stalkers, expedition recorder, hollow-district core, shadow anchors, Hollow Warden, expedition memory echoes, fallen captain, shadow-faction contact, shadow sentinels, pact altar, northern siege creatures, fortress defense beacons, citadel command pylons, siege commander, unstable gate anchors, breach guardians, boundary beacons, beacon attackers, sanctuary civilian groups, sanctuary invaders and tagged enemies must be placed/configured in their respective Unreal maps.
- The monster base needs Blueprint enemy children with meshes, animation blueprints, configured AI controllers, navigation mesh and attack/death presentation. Loot drops require the pickup actor class and per-enemy item/drop chance entries to be configured in those Blueprint children.
- Sword traces require a skeletal mesh with correctly configured weapon_r and weapon_tip sockets; the attack trace window is currently a code-timed fallback, not a polished animation-notify combat pipeline.
- Campaign chapters contain titles, briefings, unlock state and authored C++ quest objectives, but do not yet have finished level maps, configured objective actors, dialogue, cutscenes or complete in-level mission scripting. The chapter-selection widget is a data/action bridge only; it does not yet load chapter maps or present a finished menu. The save/load widget operates on one configurable slot and has no finished visual menu or multiple-slot browser.
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


## Final campaign quest integration (Chapters 18–20)

- Chapter 18 quest ID: `CH18_CrownOfRuin`; required tags: `ThroneWardenDefeated` ×4 and `RuinCommandCoreDestroyed` ×3.
- Chapter 19 quest ID: `CH19_BeyondTheGate`; required tags: `OriginGateSealStabilized` ×3 and `OriginSentinelDefeated` ×2.
- Chapter 20 quest ID: `CH20_TheFinalHunter`; required tags: `OriginHeartExposed` ×1 and `FinalHunterDefeated` ×1.
- Every chapter exit validates its exact active quest ID and complete state. Chapter 20 completion uses the existing final campaign completion event path.
- Unreal Editor work remains: create maps, objective triggers, enemies/boss, visuals, audio and level travel; compile and test the C++ in the target Unreal version. No Unreal build or gameplay test is claimed.

## HUNTER 2.0 browser vertical-slice demo

- `Demo/index.html` is a standalone browser-playable 2D prototype, separate from the Unreal Engine game. It contains three combat sectors: an opening scout encounter, a stronger hunter/brute wave, and a Breach Guardian boss.
- Implemented demo controls: WASD/arrow movement, SPACE sword slash, Q energy projectile, SHIFT dash, E healing potion and ESC pause; touch-screen action buttons are included.
- The demo includes enemy contact damage, boss projectile patterns, health/energy, XP level-ups, rank labels, potion pickups between sectors and local best-score storage.
- `Tools/validate_browser_demo.py` checks the required demo features and runs Node.js syntax validation on its inline JavaScript. This validator is part of GitHub Actions.
- This is a playable browser prototype using temporary canvas artwork. It does not represent a completed Unreal Engine build, final 3D assets, the 20-chapter campaign or verified cross-device gameplay. Manual browser playtesting is still required.

## Quest notification re-entry hardening

- The quest subsystem now increments a state revision whenever a quest is started, cleared, restored or updated by an event.
- Objective notification loops stop when a Blueprint/delegate listener changes quest state, including a clear-and-restart using the same quest ID. This prevents stale notifications from a previous state from being sent into a replacement quest.
- Quest restoration now publishes the restored snapshot first and sends objective updates from a stable copy, checking the revision after each callback.
- `Tools/validate_quest_notifications.py` checks these safeguards and runs in GitHub Actions.
- This is a source-level safety improvement. It has not yet been compiled or exercised in Unreal Engine.

## Visible Arin prototype foundation

- Arin now receives a temporary engine-native static-mesh blockout assembled from Unreal's built-in cube and sphere meshes. It includes torso, shoulders, arms, legs, boots, head, hair and a sword, so the default C++ pawn is no longer intentionally invisible when those engine primitive assets are available.
- The blockout parts are attached to the character capsule and have collision disabled, leaving movement/collision to the existing character capsule.
- This is temporary greybox geometry, not the final rigged Arin model. It has no skinning, animation, material palette, facial detail or weapon socket integration.
- `Tools/validate_playable_foundation.py` checks the blockout components and verifies every legacy input binding in the character code has a matching `DefaultInput.ini` mapping. It runs in GitHub Actions.
- The C++ has not yet been compiled or viewed in Unreal Editor; engine asset path resolution and visual alignment still require an editor test.

## Unreal breach arena greybox

- Added `AHunterPrototypeArenaActor`, a placeable C++ actor that builds a basic enclosed combat arena from Unreal's built-in cube mesh.
- The arena includes a floor, perimeter walls, split rear wall with a central gate opening, gate frame, four cover pillars and a blue point light.
- Arena width, depth and wall height are editable properties. Geometry blocks movement and is configured to contribute to navigation.
- `Tools/validate_arena_greybox.py` checks the required geometry, collision, navigation and gate lighting; it runs in GitHub Actions.
- This provides a first environment blockout for encounter assembly, not a finished art level. It still needs to be placed in an Unreal map, checked for collision/navmesh and lit/materialed in-editor.

## Visible monster prototype foundation

- The reusable monster base now assembles a temporary humanoid/creature silhouette from Unreal's built-in cube and sphere meshes: torso, shoulders, arms, legs, head and horns.
- AI chase path requests are now throttled by an editable repath interval instead of being submitted on every Tick while pursuing a target. Editable Blueprint combat and movement stats are normalized, with non-finite values replaced by safe defaults before use.
- The visual pieces have collision and navigation influence disabled. The inherited Character capsule remains responsible for movement and collision, so the blockout does not add extra hitboxes.
- `Tools/validate_monster_blockout.py` checks the required source components and is included in GitHub Actions.
- This is a shared placeholder silhouette for early encounter assembly, not a final monster roster. Distinct enemy species, materials, skeletal meshes, rigs, animations, attack telegraphs and boss-specific geometry remain to be authored.
- The C++ still requires an Unreal Editor compile and visual test. Engine primitive asset loading, scale/alignment and navigation-based pursuit must be verified in the target project version.


## Boss prototype visual foundation

- The shared boss base now creates a larger crowned silhouette from Unreal engine primitive meshes, distinct from the standard monster blockout.
- Default prototype boss tuning is 1,200 health, 35 base attack damage, 350 chase speed, 300 XP and 100 currency; Blueprint children can override these defaults.
- Boss phase thresholds are clamped, sorted and applied after initialization; invalid or negative phase damage multipliers are normalized before scaling.
- Boss visual pieces have collision and navigation influence disabled; the inherited character capsule remains responsible for collision.
- `Tools/validate_boss_foundation.py` checks the boss silhouette, default tuning and phase-scaling safeguards in GitHub Actions.
- This is still a primitive blockout, not a designed boss model, rig, animation set or complete phase-specific attack system. Unreal compilation, visual alignment and combat testing remain required.


## Encounter integration validation

- Added `Tools/validate_encounters.py` to statically verify breach-wave pacing, successful-spawn tracking, death-event removal, completion gating, and one-time reward protection.
- The same validator checks boss spawn-failure handling, boss identity matching, one-time defeat handling, and quest-event wiring.
- GitHub Actions now runs this encounter validator alongside the monster and boss foundation checks.
- These checks inspect source structure only; they do not compile Unreal C++ or prove runtime behavior in an Unreal level.


## Swept melee weapon-trace improvement

- Weapon tracing now sweeps a configurable sphere volume along both blade sockets and across the blade edge between frames, reducing missed hits during fast swings compared with four line-only traces.
- Each segment collects its own hit results; a per-swing weak-actor set still ensures an actor receives damage only once per attack.
- Trace activation and ticking verify both configured mesh sockets still exist; a missing mesh/socket safely ends the trace.
- Trace radius and base damage are normalized against non-finite or out-of-range values. The trace remains configurable through the Blueprint component defaults.
- `Tools/validate_weapon_trace.py` checks the swept-volume implementation, socket guards, hit de-duplication and finite damage settings in GitHub Actions.
- This is static source validation only. The trace radius, collision channel, socket placement, and damage feel must still be tuned and tested against real skeletal weapon sockets in Unreal Editor.


## Health component finite-value hardening

- Damage and healing reject NaN and infinite input amounts before changing health.
- Initial maximum health is normalized to a finite positive value; attempts to set maximum health to a non-finite value are ignored.
- Health percentage and preserved-health calculations are clamped to safe finite ranges.
- `Tools/validate_health_component.py` checks the health mutation safeguards in GitHub Actions.
- These are static source checks. Runtime behavior still needs Unreal compilation and gameplay testing.


## Monster damage finite-value hardening

- Monster `TakeDamage` rejects non-finite, zero/negative and post-death damage before applying it.
- Returned damage is checked before it reaches monster health mutation.
- Direct hunter damage also rejects invalid values and clamps against a finite maximum-health fallback.
- Monster health percentage is now bounded to the 0–1 range.
- `Tools/validate_monster_damage.py` checks these guards in GitHub Actions.

## Player damage and movement safety pass

- Arin rejects non-finite and non-positive incoming damage, ignores damage after death, validates the mitigated result and clamps equipment damage reduction to a safe range.
- Arin's editable movement/combat values (sprint, dodge, respawn delay, attack cooldown and weapon trace window) are normalized at BeginPlay; movement axis values and progression speed multipliers are checked before use.
- `Tools/validate_player_damage.py` and `Tools/validate_arin_tuning.py` exercise these source-level safeguards in GitHub Actions.
- This does not replace Unreal compilation or hands-on controller/gameplay testing.

## Monster AI safety pass

- Monster AI ignores a defeated Arin when acquiring a target and drops a target that dies while being pursued.
- Monster Tick rejects non-finite or non-positive frame deltas, and attack damage is checked before applying it.
- `Tools/validate_monster_damage.py` now also checks these AI safeguards in GitHub Actions.
- The existing AI still requires configured AI controllers, navigation bounds and in-editor testing; this source check does not prove pathfinding or combat feel.


## Campaign save-load integrity

- Campaign loading now validates the complete save snapshot before changing campaign chapter state or restoring player data.
- Loads reject unsupported chapter numbers, negative XP/currency, evolution stages outside 0–4, oversized completion lists, invalid chapter IDs and duplicate completed chapters.
- This prevents malformed or edited save data from partially mutating the live campaign before being rejected.
- `Tools/validate_campaign_load.py` checks the ordering and presence of these source safeguards in GitHub Actions.
- This is a static source validation only; actual save-file compatibility and restoration still require Unreal compilation and runtime testing.
