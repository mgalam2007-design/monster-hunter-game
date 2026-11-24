from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter14_level.py"
source = SCRIPT.read_text(encoding="utf-8")
required = [
    'LEVEL_PATH = "/Game/Maps/Chapter14/L_CH14_BrokenCitadel"',
    "HunterPrototypeArenaActor",
    "HunterQuestTriggerActor",
    "HunterBossEncounterActor",
    "HunterBossBase",
    "HunterChapterExitActor",
    "CitadelPylonDestroyed",
    "CitadelCommanderDefeated",
    "pylon_points =",
    "enumerate(pylon_points, start=1)",
    "requires_interaction",
    "E  DESTROY COMMAND PYLON",
    "boss_defeat_quest_event_tag",
    "boss_spawn_offset",
    "next_chapter_map_name",
    "CH15_WorldsUnsealed",
    "NavMeshBoundsVolume",
    "save_current_level",
]
missing = [token for token in required if token not in source]
if missing: raise SystemExit("Chapter 14 generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
assert source.count("CitadelPylonDestroyed") == 1
print("Chapter 14 Unreal Editor level generator contract passed.")
