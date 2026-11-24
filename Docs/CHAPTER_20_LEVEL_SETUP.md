# Chapter 20 — The Final Hunter: Unreal Editor Integration Guide

Compile HunterGame and run `Tools/unreal/create_chapter20_level.py` from **Tools → Execute Python Script** to generate `/Game/Maps/Chapter20/L_CH20_TheFinalHunter`. Build NavMesh before Play.

## Objectives
- Press E at the Origin Heart seal (`OriginHeartExposed`).
- Enter the final encounter and defeat the Final Hunter (`FinalHunterDefeated`).
- Reach the finale exit to complete the final chapter and save the campaign. No next map is configured.

The final enemy now uses `AHunterFinalBossBase`, a dedicated subclass of the shared multi-phase boss foundation. It has a telegraphed Origin Pulse: the boss warns, waits, then damages Arin if he remains within the configured radius. Pulse damage rises by 20% per boss phase and pulse cooldown shortens by 15% per phase. Blueprint events `BP_OnOriginPulseTelegraph` and `BP_OnOriginPulseImpact` are the hooks for warning rings, audio, camera shake and impact VFX. The unique final-boss mesh, authored attack animation, ending/cutscene, Arin's final evolution presentation, final arena art and runtime testing remain outstanding.
