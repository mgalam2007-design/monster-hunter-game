# HUNTER 2.0 — Campaign Level Integration Guide

This guide maps the 20 authored C++ campaign quests to the event tags that level actors and enemies must report. It is an Unreal Editor implementation checklist, not a claim that the maps or actors already exist.

## Shared setup for every chapter

1. Create a level for the chapter and place the configured Arin player start.
2. Place the reusable quest-trigger actor for each reach/interact/collect objective. Set its event tag to the exact spelling below and configure its one-shot behavior to prevent duplicate interactions.
3. For defeat objectives, configure each enemy Blueprint to report the matching quest event tag when it dies. Use the correct enemy class and count only intended mission enemies.
4. For pickups, configure the item ID and matching collection event tag. Confirm collection adds the item and advances the quest only once.
5. Place a chapter-exit actor at the end of the playable route. Its completion attempt is intentionally rejected unless the exact chapter quest is active and complete.
6. Add a checkpoint where appropriate and configure its save slot consistently with the project's save/load UI.
7. Test each objective from a fresh save, then test leaving/re-entering the level and saving/loading partial progress.

Event tag matching is exact and case-sensitive. Objective counts are required event totals, not numbers of actors automatically spawned by the subsystem.

## Chapter objective matrix

| Chapter | Quest ID | Required objective events |
|---|---|---|
| 1 — The First Breach | `CH01_FirstBreach` | Defeat 3: `MonsterDefeated`; collect 1 shard: `BreachShardCollected` |
| 2 — Ashes at Dawn | `CH02_MissingSurvivors` | Reach district: `SurvivorDistrictLocated` ×1; rescue civilians: `SurvivorRescued` ×3 |
| 3 — The Lowest Rank | `CH03_GuildEvaluation` | Defeat trial opponents: `TrialOpponentDefeated` ×5; activate seal: `TrialSealActivated` ×1 |
| 4 — A Door Below | `CH04_TransitGate` | Reach gate: `TransitGateReached` ×1; defeat guardian: `TransitGuardianDefeated` ×1 |
| 5 — The Silent Contract | `CH05_MiningContract` | Defeat mining creatures: `MiningCreatureDefeated` ×4; collect ore: `CorruptedOreCollected` ×3 |
| 6 — Trial of Steel | `CH06_TrialOfSteel` | Defeat elite opponent: `SteelTrialOpponentDefeated` ×1; guild approval: `SteelTrialApproved` ×1 |
| 7 — The Blue Mark | `CH07_BlueMark` | Locate source: `BlueMarkSourceLocated` ×1; collect fragments: `ResonanceFragmentCollected` ×3 |
| 8 — A Hunter Reborn | `CH08_HunterReborn` | Defeat ambushers: `AwakeningCreatureDefeated` ×6; activate altar: `AwakeningAltarActivated` ×1 |
| 9 — Whispers in the Dark | `CH09_VanishedHunters` | Locate signal: `MissingHunterSignalLocated` ×1; defeat stalkers: `GateStalkerDefeated` ×4; recover recorder: `ExpeditionRecorderRecovered` ×1 |
| 10 — The Hollow District | `CH10_HollowDistrict` | Reach core: `HollowDistrictCoreReached` ×1; destroy anchors: `ShadowAnchorDestroyed` ×3; defeat Warden: `HollowWardenDefeated` ×1 |
| 11 — Echoes of the Fallen | `CH11_EchoesOfTheFallen` | Recover memories: `ExpeditionMemoryRecovered` ×3; defeat captain: `FallenCaptainDefeated` ×1 |
| 12 — The Shadow Accord | `CH12_ShadowAccord` | Meet contact: `ShadowContactReached` ×1; defeat sentinels: `ShadowSentinelDefeated` ×3; resolve pact: `ShadowPactResolved` ×1 |
| 13 — Siege of the North | `CH13_SiegeOfTheNorth` | Defeat siege creatures: `NorthernSiegeCreatureDefeated` ×5; activate beacons: `FortressBeaconActivated` ×2 |
| 14 — The Broken Citadel | `CH14_BrokenCitadel` | Destroy pylons: `CitadelPylonDestroyed` ×3; defeat commander: `CitadelCommanderDefeated` ×1 |
| 15 — A World Unsealed | `CH15_WorldsUnsealed` | Stabilize anchors: `BreachAnchorStabilized` ×4; defeat guardians: `BreachGuardianDefeated` ×2 |
| 16 — The Seven Beacons | `CH16_SevenBeacons` | Activate boundary beacons: `BoundaryBeaconActivated` ×7; defeat attackers: `BeaconAttackerDefeated` ×3 |
| 17 — The Last Sanctuary | `CH17_LastSanctuary` | Evacuate groups: `SanctuaryGroupEvacuated` ×5; defeat invaders: `SanctuaryInvaderDefeated` ×6 |
| 18 — Crown of Ruin | `CH18_CrownOfRuin` | Defeat throne wardens: `ThroneWardenDefeated` ×4; destroy command cores: `RuinCommandCoreDestroyed` ×3 |
| 19 — Beyond the Gate | `CH19_BeyondTheGate` | Stabilize origin seals: `OriginGateSealStabilized` ×3; defeat sentinels: `OriginSentinelDefeated` ×2 |
| 20 — The Final Hunter | `CH20_TheFinalHunter` | Expose origin heart: `OriginHeartExposed` ×1; defeat final enemy: `FinalHunterDefeated` ×1 |

## Final campaign completion

Chapter 20 is the final chapter. When its exact quest is complete and the chapter-exit actor successfully completes it, the campaign subsystem marks the final chapter complete and emits its campaign-completed event. Do not broadcast this event directly from a boss Blueprint; let the chapter completion flow own it.

## Validation checklist

- Every trigger uses the exact event tag shown above.
- Each enemy reports its defeat event once, on actual death.
- Each interactable/collectible cannot accidentally report multiple completions from one use.
- Quest tracker displays every objective and its current/required count.
- A chapter exit does not complete the chapter while any objective is incomplete.
- Starting a chapter cannot overwrite another unfinished active quest.
- Re-entering a chapter preserves its partial quest progress.
- Save/load restores objective counters and completion state.
- Chapters unlock sequentially; later chapters cannot be completed out of order.
- Completing Chapter 20 produces the final campaign-completed event.

## Remaining work

The C++ quest definitions and completion gates are committed. The Unreal maps, trigger placements, enemy and boss Blueprints, pickups, visual presentation, dialogue, rewards, map travel and full playthrough tests still need to be created in Unreal Editor. Compile the project in the target Unreal Engine version and validate every chapter in-game before calling the campaign production-ready.
