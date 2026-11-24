from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter09_level.py"
source = SCRIPT.read_text(encoding="utf-8")
required = [
    'LEVEL_PATH = "/Game/Maps/Chapter09/L_CH09_VanishedHunters"',
    "HunterPrototypeArenaActor", "HunterMonsterBase", "HunterQuestTriggerActor",
    "HunterChapterExitActor", "MissingHunterSignalLocated", "GateStalkerDefeated",
    "ExpeditionRecorderRecovered", "spawn_points =", "enumerate(spawn_points, start=1)",
    "defeat_quest_event_tag", "requires_interaction", "E  RECOVER EXPEDITION RECORDER",
    "next_chapter_map_name", "CH10_HollowDistrict", "NavMeshBoundsVolume", "save_current_level",
]
missing = [token for token in required if token not in source]
if missing: raise SystemExit("Chapter 9 generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
assert source.count("GateStalkerDefeated") == 1
print("Chapter 9 Unreal Editor level generator contract passed.")
