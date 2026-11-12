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
- Starter sword attack hook, ready to connect to a montage and hit detection
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
5. In the editor, create an input mapping context (or use the legacy input bindings in `Config/DefaultInput.ini`) and assign the default game mode. Confirm the legacy input mappings in `Config/DefaultInput.ini`. Confirm the legacy input mappings in `Config/DefaultInput.ini`.

## Controls (legacy input bindings)
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
The repository does **not** yet contain the finished Arin Vale 3D mesh, textures, skeleton, or animation assets. Those must be authored in Blender or another 3D tool, then imported into Unreal. The code scaffold is the foundation, not a claim that the final character art is already implemented.
