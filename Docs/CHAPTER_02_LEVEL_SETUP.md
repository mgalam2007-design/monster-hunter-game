# Chapter 02 — Missing Survivors: Unreal Editor Integration Guide

## Generate the greybox level

1. Compile the `HunterGame` C++ module in Unreal Editor.
2. Run **Tools → Execute Python Script** and select `Tools/unreal/create_chapter02_level.py`.
3. The script creates `/Game/Maps/Chapter02/L_CH02_MissingSurvivors`, including the arena, PlayerStart, lighting, NavMesh bounds, district-location trigger, three civilian-rescue triggers, simple visible civilian blockouts and chapter exit.
4. Build navigation, open the generated map, press **P** to inspect the NavMesh, then Play.

## Objective flow

The native campaign subsystem starts Chapter 2's `CH02_MissingSurvivors` quest when this map is entered. The quest requires:

- Enter the district marker to report `SurvivorDistrictLocated`.
- Enter each of the three marked rescue volumes. A native floating **E — RESCUE CIVILIAN** prompt appears; press **E** to rescue the civilian. Each successful interaction reports one `SurvivorRescued` event. Leaving the volume hides the prompt and cancels the interaction.
- Reach the chapter exit only after all four objective counts are complete. The exit then completes and saves Chapter 2.

Each rescue marker now has a simple cube-and-sphere civilian silhouette with a torso, head, arms and legs placed beside it. Their placeholder meshes have collision disabled so they do not obstruct movement. They are visible editor-generated placeholders, not animated NPCs. The generated rescue volumes are invisible greybox interaction zones. The native text prompt appears automatically, while `BP_OnInteractionAvailable` and `BP_OnQuestEventTriggered` remain available for additional UI and rescue presentation. Add visible civilian placeholder meshes, dialogue, animation and AI in Unreal Editor before treating the mission as player-ready. The current implementation uses a direct E-key interaction; it does not yet implement civilian NPC AI or bespoke rescue animations.

## Chapter 1 to Chapter 2 travel

The Chapter 1 generator now sets its chapter exit's `NextChapterMapName` to `/Game/Maps/Chapter02/L_CH02_MissingSurvivors`. Regenerate the Chapter 1 map after compiling the updated module so the placed exit receives this setting. The chapter exit travels only after the Chapter 1 quest is complete and its save succeeds.

## Acceptance checklist

- [ ] Generate and save both Chapter 1 and Chapter 2 maps in Unreal Editor.
- [ ] Complete Chapter 1 and confirm it transitions to Chapter 2 after saving.
- [ ] Confirm the Chapter 2 quest tracker shows the district-location objective and 0/3 rescues.
- [ ] Visit the district marker and confirm its objective completes.
- [ ] Visit all three rescue volumes and confirm the rescue count reaches 3/3.
- [ ] Confirm the Chapter 2 exit is blocked before all objectives complete.
- [ ] Complete the chapter and confirm the save restores Chapter 3 as the next playable chapter.

## Current boundary

The generator and static contract checks are committed, but the script has not been executed in Unreal Editor in this environment. This is a functional greybox objective flow, not a finished survivor mission: visible civilians, interaction prompts, dialogue, animation, audio, art and runtime testing remain outstanding.
