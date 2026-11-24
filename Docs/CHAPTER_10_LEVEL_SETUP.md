# Chapter 10 — The Hollow District: Unreal Editor Integration Guide

Compile HunterGame and run `Tools/unreal/create_chapter10_level.py` from **Tools → Execute Python Script** to generate `/Game/Maps/Chapter10/L_CH10_HollowDistrict`. Build NavMesh before Play.

## Objectives
- Reach the district core (`HollowDistrictCoreReached`).
- Press E at each of three shadow anchors (`ShadowAnchorDestroyed`).
- Enter the Warden encounter radius and defeat the native multi-phase boss (`HollowWardenDefeated`).
- Reach the exit after all three objectives are complete to save Chapter 10.

The anchors are interactive markers and the Warden uses the existing boss blockout, not production assets. Custom boss attacks, animation, VFX/audio, district art and runtime testing remain outstanding.
