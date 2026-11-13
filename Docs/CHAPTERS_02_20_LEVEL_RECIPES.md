# Chapters 02–20 — Unreal Level Assembly Recipes

These recipes convert the committed campaign quest definitions into editor tasks. They are not prebuilt maps or Blueprint assets. The C++ trigger, pickup, monster, boss-encounter and chapter-exit classes must be compiled in the target Unreal Engine project before they can be placed.

## Reusable actor configuration

### Tagged world trigger

Create a Blueprint child of `HunterQuestTriggerActor` for each reach, interact, rescue, seal, beacon, anchor or other non-pickup objective. Configure:

- **Quest Event Tag**: exact tag from the chapter matrix below (case-sensitive).
- **Event Amount**: normally `1`; use a single trigger per required event to make counts easy to audit.
- **Trigger Once**: enabled.
- **Destroy After Trigger**: optional; enable for disposable one-shot objects, disable for persistent landmarks.
- Resize the inherited box collision to the intended interaction/arrival area. Test that only Arin overlaps it.

For objectives with a required count greater than one, place that many independently usable trigger actors. Do not set Event Amount to the total on every actor or one interaction could over-count.

### Enemy defeat events

Create Blueprint children of `HunterMonsterBase` and configure visible mesh, Animation Blueprint, AI controller, collision, movement, health and attack presentation. Set **Defeat Quest Event Tag** to the exact defeat tag for that chapter. Place the required number of mission enemies, or use a controlled spawn director that creates exactly that number. Keep unrelated enemies on a different event tag so they cannot advance the mission objective.

Each enemy should report its defeat event only when it actually dies. Validate death, XP/currency reward and objective increment in PIE.

### Pickups and collectible objectives

For a physical collectible, create a Blueprint child of `HunterPickupActor`, assign a visible mesh and a valid inventory item, keep **Destroy After Pickup** enabled, and set its **Quest Event Tag** to the matching collection event. Place the required number of pickups. Confirm the item enters inventory and the objective advances once per pickup.

If an objective is a non-inventory interaction (for example, activating a beacon), use a tagged world trigger instead of a pickup.

### Boss encounters and exits

For a boss fight, create a Blueprint child of `HunterBossEncounterActor`, set **Boss Class**, verify the activation radius and spawn offset, and set **Boss Defeat Quest Event Tag** to the chapter's exact boss defeat tag. The boss Blueprint must derive from `HunterBossBase`. The encounter actor supplies a spawn/defeat framework; phase-specific attacks, visuals and audio remain Blueprint work.

Place a Blueprint child of `HunterChapterExitActor` after all objectives. Keep the shared save slot consistent across the campaign. The exit's C++ completion gate checks the exact chapter quest, so do not bypass it from a Blueprint.

## Recommended editor workflow for each chapter

1. Duplicate a clean chapter level template; rename it using the map name below and set a unique player start.
2. Block out the route and combat arena with simple geometry. Add a `NavMeshBoundsVolume` over all enemy walkable areas and verify navigation with **P**.
3. Add one configured trigger/pickup/enemy per required event count. Label actors clearly in the Outliner, e.g. `Q_CH02_Rescue_01`.
4. Add a checkpoint before the most significant combat section and an exit beyond the final objective.
5. Start from a fresh save and test each event. Then test partial save/load, re-entry and duplicate overlap prevention.
6. Replace blockout geometry and placeholder meshes only after the objective loop works.

## Chapter-by-chapter assembly

| Chapter / map | Quest ID | Required world content |
|---|---|---|
| 02 — Ashes at Dawn / `L_CH02_AshesAtDawn` | `CH02_MissingSurvivors` | One district-arrival trigger: `SurvivorDistrictLocated`; three separate civilian rescue triggers: `SurvivorRescued`. Stage the civilians in a damaged street or shelter route. |
| 03 — The Lowest Rank / `L_CH03_LowestRank` | `CH03_GuildEvaluation` | Five trial enemies tagged `TrialOpponentDefeated`; one arena seal trigger: `TrialSealActivated`. Keep the arena enclosed and readable. |
| 04 — A Door Below / `L_CH04_ADoorBelow` | `CH04_TransitGate` | One gate approach trigger: `TransitGateReached`; one guardian enemy tagged `TransitGuardianDefeated`. Place the exit beyond the sealed-gate encounter. |
| 05 — The Silent Contract / `L_CH05_SilentContract` | `CH05_MiningContract` | Four mining creatures tagged `MiningCreatureDefeated`; three ore pickups tagged `CorruptedOreCollected`. Keep ore visible and reachable after or during combat. |
| 06 — Trial of Steel / `L_CH06_TrialOfSteel` | `CH06_TrialOfSteel` | One elite trial enemy tagged `SteelTrialOpponentDefeated`; one guild approval trigger: `SteelTrialApproved`. Gate approval access until the duel is complete using Blueprint logic. |
| 07 — The Blue Mark / `L_CH07_BlueMark` | `CH07_BlueMark` | One source-location trigger: `BlueMarkSourceLocated`; three resonance pickups tagged `ResonanceFragmentCollected`. Place fragments along a short exploration route. |
| 08 — A Hunter Reborn / `L_CH08_HunterReborn` | `CH08_HunterReborn` | Six awakening creatures tagged `AwakeningCreatureDefeated`; one altar activation trigger: `AwakeningAltarActivated`. Put the altar at the arena's far end. |
| 09 — Whispers in the Dark / `L_CH09_WhispersInTheDark` | `CH09_VanishedHunters` | One signal trigger: `MissingHunterSignalLocated`; four stalkers tagged `GateStalkerDefeated`; one recorder pickup tagged `ExpeditionRecorderRecovered`. |
| 10 — The Hollow District / `L_CH10_HollowDistrict` | `CH10_HollowDistrict` | One core-area trigger: `HollowDistrictCoreReached`; three anchor interactables tagged `ShadowAnchorDestroyed`; one Warden tagged `HollowWardenDefeated`. |
| 11 — Echoes of the Fallen / `L_CH11_EchoesOfTheFallen` | `CH11_EchoesOfTheFallen` | Three memory pickups tagged `ExpeditionMemoryRecovered`; one captain enemy tagged `FallenCaptainDefeated`. |
| 12 — The Shadow Accord / `L_CH12_ShadowAccord` | `CH12_ShadowAccord` | One faction-contact trigger: `ShadowContactReached`; three sentinels tagged `ShadowSentinelDefeated`; one pact-resolution trigger: `ShadowPactResolved`. |
| 13 — Siege of the North / `L_CH13_SiegeOfTheNorth` | `CH13_SiegeOfTheNorth` | Five siege creatures tagged `NorthernSiegeCreatureDefeated`; two beacon triggers: `FortressBeaconActivated`. |
| 14 — The Broken Citadel / `L_CH14_BrokenCitadel` | `CH14_BrokenCitadel` | Three pylon interactables tagged `CitadelPylonDestroyed`; one commander enemy tagged `CitadelCommanderDefeated`. |
| 15 — A World Unsealed / `L_CH15_AWorldUnsealed` | `CH15_WorldsUnsealed` | Four gate-anchor triggers: `BreachAnchorStabilized`; two guardian enemies tagged `BreachGuardianDefeated`. |
| 16 — The Seven Beacons / `L_CH16_TheSevenBeacons` | `CH16_SevenBeacons` | Seven beacon triggers tagged `BoundaryBeaconActivated`; three attackers tagged `BeaconAttackerDefeated`. |
| 17 — The Last Sanctuary / `L_CH17_LastSanctuary` | `CH17_LastSanctuary` | Five civilian-group evacuation triggers tagged `SanctuaryGroupEvacuated`; six invaders tagged `SanctuaryInvaderDefeated`. |
| 18 — Crown of Ruin / `L_CH18_CrownOfRuin` | `CH18_CrownOfRuin` | Four throne wardens tagged `ThroneWardenDefeated`; three command-core interactables tagged `RuinCommandCoreDestroyed`. |
| 19 — Beyond the Gate / `L_CH19_BeyondTheGate` | `CH19_BeyondTheGate` | Three origin-seal triggers tagged `OriginGateSealStabilized`; two sentinels tagged `OriginSentinelDefeated`. |
| 20 — The Final Hunter / `L_CH20_TheFinalHunter` | `CH20_TheFinalHunter` | One origin-heart exposure trigger: `OriginHeartExposed`; one final boss/enemy tagged `FinalHunterDefeated`. Put the final exit after the boss and heart objective. |

## Mission gating and event-order notes

The quest subsystem tracks event totals, not story prerequisites between individual objectives. If the intended sequence matters (for example, a seal should only activate after enemies are defeated), implement that local prerequisite in the level Blueprint: disable the relevant interaction until the required encounter has completed. Do not rely on the order of rows in the quest tracker to enforce world interaction order.

For multi-count interactables, make each actor one-shot. For enemy waves, ensure the spawn system cannot spawn more quest-tagged enemies than the required total. Avoid reusing a mission defeat tag on ambient enemies.

## Acceptance test per map

- [ ] Correct chapter level and player start load.
- [ ] Every objective actor has the exact tag from the matrix.
- [ ] Every required event can be triggered the required number of times, no more.
- [ ] Ambient enemies and unrelated collectibles do not advance mission counters.
- [ ] Boss encounter starts once and reports its configured defeat tag once.
- [ ] Quest tracker reaches the required counts.
- [ ] Exit is blocked until the exact quest is complete.
- [ ] Save/load restores partial objective counts and chapter progress.
- [ ] Re-entering the map does not reset an active quest.
- [ ] No collision, AI, Blueprint or save errors appear in the Output Log.

## Current boundary

These recipes specify the editor work; they do not create Unreal maps, actors, meshes, animation Blueprints, dialogue, level travel or UMG layouts. The C++ must be compiled and tested in the project's actual Unreal Engine version. No Unreal Editor build or playthrough is claimed by this document.
