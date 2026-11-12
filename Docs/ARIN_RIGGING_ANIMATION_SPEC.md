# Arin Vale — Rigging and Animation Implementation Specification

This document turns the blockout into a production character. The current Blender generator creates separate static mesh pieces only. It does not create a skeleton, skin weights, facial rig, animation clips, or Unreal assets.

## 1. Final character asset layout
- ARIN_BODY: head, torso, hands, legs and feet
- ARIN_HAIR: scalp cap and separate hair cards/clumps
- ARIN_STAGE_01: starter jacket, trousers, boots and guards
- ARIN_WEAPON_SWORD: grip, guard and blade
- ARIN_STAGE_02 through ARIN_STAGE_05: modular armor upgrades
- ARIN_RIG: one shared humanoid armature and control rig
- ARIN_FX_ATTACHMENTS: named sockets/locators for energy effects

The Blender generator currently creates separate primitive meshes, not this finished asset layout.

## 2. Skeleton requirements
Use a humanoid hierarchy compatible with Unreal's Manny/Quinn conventions where practical. Keep a stable retargeting base:
- root, pelvis
- spine_01, spine_02, spine_03, neck_01, head
- clavicle_l / upperarm_l / lowerarm_l / hand_l
- clavicle_r / upperarm_r / lowerarm_r / hand_r
- thigh_l / calf_l / foot_l / ball_l
- thigh_r / calf_r / foot_r / ball_r

Add optional twist bones and finger chains for deformation and weapon grip. Name bones consistently and document deviations before animation production.

## 3. Skinning and modular armor
- Retopologize the hero body; do not use the blockout spheres as final geometry.
- Apply transforms and maintain consistent scale/orientation before binding.
- Use clean deformation topology around shoulders, elbows, hips, knees and neck.
- Bind body and deforming clothing to the shared skeleton with normalized weights.
- Parent rigid armor plates to appropriate bones or use constrained accessory meshes.
- Test extreme poses; correct clipping and weight collapse.
- Keep stage armor modular so face, hair, body proportions and base skeleton never change.

## 4. Facial and hair requirements
- Preserve approved face shape, eye spacing, nose, jaw and hairstyle silhouette.
- Create facial blend shapes or a facial bone rig for brows, eyelids, jaw and mouth.
- Separate hair clumps and use restrained secondary motion; avoid hair obscuring the eyes in gameplay.
- Emissive eye glow is a temporary ability effect, not a permanent identity change.

## 5. Unreal animation blueprint
Create ABP_ArinVale with:
- Locomotion state machine: idle, walk, jog, sprint, jump, fall, land.
- Directional start/stop and turn-in-place support.
- Upper-body slot for weapon actions while locomotion continues.
- Combat montages for sword combos, heavy attacks, guard, hit reactions and recovery.
- Dodge/evade with invulnerability timing controlled by gameplay code, not guessed from animation duration.
- Aim/fire/reload layers if ranged weapons are retained.
- Animation notifies for footstep audio, weapon trails, hit windows and FX events.
- Motion warping for contextual finishers and boss interactions.

## 6. First animation deliverable
Create and test this minimum set before expanding:
1. Combat idle loop
2. Walk and run cycles
3. Jump start, fall loop and landing
4. Directional dodge
5. Three-hit sword combo
6. Heavy sword attack
7. Hit reaction and knockdown recovery
8. Awakening pose/cinematic

Animations must be original. Do not extract or trace copyrighted anime footage or another game's proprietary animation.

## 7. Export and validation
- Export skeletal mesh and animations as FBX using a documented Blender-to-Unreal preset.
- Export the shared skeleton once; use it for every stage outfit and animation.
- Verify forward axis, scale, root orientation, bone hierarchy and animation range in Unreal.
- Create LODs and test material count, draw calls and animation performance.
- Validate on the actual minimum target hardware before setting final quality settings.

## Definition of done
Arin is not game-ready until the final mesh is rigged, weighted, textured, equipped with modular stage armor, imported into Unreal, and tested with the locomotion and combat animation set. A static blockout or generated concept image does not satisfy this milestone.