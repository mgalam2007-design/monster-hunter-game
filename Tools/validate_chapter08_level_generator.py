from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter08_level.py"
source = SCRIPT.read_text(encoding="utf-8")
required = [
    'LEVEL_PATH = "/Game/Maps/Chapter08/L_CH08_HunterReborn"',
    "HunterPrototypeArenaActor", "HunterMonsterBase", "HunterQuestTriggerActor",
    "HunterChapterExitActor", "AwakeningCreatureDefeated", "AwakeningAltarActivated",
    "spawn_points =", "enumerate(spawn_points, start=1)", "defeat_quest_event_tag",
    "E  ACTIVATE AWAKENING ALTAR", "AWAKENING COMPLETE", "NavMeshBoundsVolume",
    "save_current_level",
]
missing = [token for token in required if token not in source]
if missing: raise SystemExit("Chapter 8 generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
assert source.count("AwakeningCreatureDefeated") == 1
assert source.count("AwakeningAltarActivated") == 1
print("Chapter 8 Unreal Editor level generator contract passed.")
