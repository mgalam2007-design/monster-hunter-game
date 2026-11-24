from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter20_level.py"
source = SCRIPT.read_text(encoding="utf-8")
required = [
    'LEVEL_PATH = "/Game/Maps/Chapter20/L_CH20_TheFinalHunter"',
    "HunterPrototypeArenaActor",
    "HunterQuestTriggerActor",
    "HunterBossEncounterActor",
    "HunterFinalBossBase",
    "HunterChapterExitActor",
    "OriginHeartExposed",
    "FinalHunterDefeated",
    "boss_defeat_quest_event_tag",
    "boss_spawn_offset",
    "requires_interaction",
    "E  DISABLE FINAL SEAL",
    "Campaign_Finale_Exit",
    "final chapter",
    "NavMeshBoundsVolume",
    "save_current_level",
]
missing = [token for token in required if token not in source]
if missing: raise SystemExit("Chapter 20 generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
assert source.count("OriginHeartExposed") == 1
print("Chapter 20 Unreal Editor level generator contract passed.")
