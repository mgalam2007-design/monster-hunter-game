from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter13_level.py"
source = SCRIPT.read_text(encoding="utf-8")
required = [
    'LEVEL_PATH = "/Game/Maps/Chapter13/L_CH13_SiegeOfTheNorth"',
    "HunterPrototypeArenaActor",
    "HunterQuestTriggerActor",
    "HunterMonsterBase",
    "HunterChapterExitActor",
    "NorthernSiegeCreatureDefeated",
    "FortressBeaconActivated",
    "siege_points =",
    "enumerate(siege_points, start=1)",
    "beacon_points =",
    "enumerate(beacon_points, start=1)",
    "defeat_quest_event_tag",
    "requires_interaction",
    "E  RESTORE FORTRESS BEACON",
    "next_chapter_map_name",
    "CH14_BrokenCitadel",
    "NavMeshBoundsVolume",
    "save_current_level",
]
missing = [token for token in required if token not in source]
if missing: raise SystemExit("Chapter 13 generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
assert source.count("NorthernSiegeCreatureDefeated") == 1
print("Chapter 13 Unreal Editor level generator contract passed.")
