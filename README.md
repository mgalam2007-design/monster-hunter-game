# HUNTER 2.0

An original third-person 3D anime action RPG built with **Unreal Engine 5 and C++**.

## Project direction
- Original protagonist: **Arin Vale**
- Third-person PC game first
- Original world, story, monsters, weapons, and animations
- Arin's face, hairstyle, proportions, and silhouette stay recognizable across all 20 chapters. Armor, weapons, aura, and abilities evolve through five progression stages.
- Inspired by broad dark-fantasy action-RPG conventions; no copied anime/manga characters, scenes, dialogue, artwork, or animation.

## Current milestone: playable character foundation
This repository currently contains the C++ project scaffold:
- Third-person character class with camera boom and follow camera
- Basic movement, look, jump, sprint, and directional dodge
- Swept melee hit detection with a procedural blade fallback for the unrigged blockout character
- Progression-gated abilities: area-damage Energy Strike, forward Shadow Step and configurable Shadow Echo actor
- Inventory consumables that restore health and/or energy
- Stackable inventory, world pickups, weapon/armor stat equipment and campaign persistence
- Monster XP rewards and tagged quest objective progress
- Game mode and Unreal module setup

## Open in Unreal Engine
1. Install Unreal Engine 5 and Visual Studio 2022 with the **Game development with C++** workload.
2. Clone this repository.
3. Right-click `HunterGame.uproject` and generate Visual Studio project files.
4. Open `HunterGame.uproject`, allow Unreal to compile the C++ modules, then press Play.
5. Confirm the project compiles and opens in the editor. The default game mode and legacy input mappings are already configured in the repository.

## Generate and test Chapter 1\n1. Compile the `HunterGame` C++ module in Unreal Editor.\n2. Open **Tools → Execute Python Script** and run `Tools/unreal/create_chapter01_level.py`.\n3. The script creates `/Game/Maps/Chapter01/L_CH01_FirstBreach` with the arena, PlayerStart, three-enemy breach encounter, Breach Shard reward, checkpoint, chapter exit, lighting and NavMesh bounds.\n4. Build navigation in the editor, open the generated level, press **P** to inspect the NavMesh, then press **Play**.\n5. Move into the arena to trigger the wave. Defeat the three enemies, collect the shard, and reach the chapter exit.\n\nThe native game mode starts the current campaign quest automatically. Arin has a generated HUD and quest tracker; press **I** to open the inventory panel.\n\n## Controls (legacy input bindings)
- WASD: move
- Mouse: look
- Space: jump
- Left Shift: sprint
- Q: dodge
- Left Mouse: sword attack
- 1: Energy Strike (unlocks at Elite)
- 2: Shadow Step (unlocks at Legendary)
- 3: Shadow Echo (unlocks at Legendary; requires a configured actor Blueprint)
- F: use the first usable health/energy consumable in inventory
- 1: Energy Strike (unlocks at Elite)
- 2: Shadow Step (unlocks at Legendary)
- 3: Shadow Echo (unlocks at Legendary; requires a configured actor Blueprint)
- F: use the first usable health/energy consumable in inventory

## Important asset status
The repository does **not** yet contain finished Arin/monster skeletal meshes, textures, animation assets, final environments, or authored cinematic sequences. The Chapter 1 generator creates a greybox map, not a finished production level. Unreal compilation, map generation and runtime gameplay still need to be verified in the editor; GitHub static checks do not perform those tests.
