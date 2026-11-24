from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter16_level.py"
source = SCRIPT.read_text(encoding="utf-8")
required = [
    'LEVEL_PATH = "/Game/Maps/Chapter16/L_CH16_SevenBeacons"',
    "HunterPrototypeArenaActor",
    "HunterQuestTriggerActor",
    "HunterMonsterBase",
    "HunterChapterExitActor",
    "BoundaryBeaconActivated",
    "BeaconAttackerDefeated",
    "beacon_points =",
    "enumerate(beacon_points, start=1)",
    "attacker_points =",
    "enumerate(attacker_points, start=1)",
    "defeat_quest_event_tag",
    "requires_interaction",
    "E  ACTIVATE BOUNDARY BEACON",
    "next_chapter_map_name",
    "CH17_LastSanctuary",
    "NavMeshBoundsVolume",
    "save_current_level",
]
missing = [token for token in required if token not in source]
if missing: raise SystemExit("Chapter 16 generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
assert source.count("BoundaryBeaconActivated") == 1
print("Chapter 16 Unreal Editor level generator contract passed.")
