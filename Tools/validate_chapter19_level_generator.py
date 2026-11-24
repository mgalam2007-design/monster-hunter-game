from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter19_level.py"
source = SCRIPT.read_text(encoding="utf-8")
required = [
    'LEVEL_PATH = "/Game/Maps/Chapter19/L_CH19_BeyondTheGate"',
    "HunterPrototypeArenaActor",
    "HunterQuestTriggerActor",
    "HunterMonsterBase",
    "HunterChapterExitActor",
    "OriginGateSealStabilized",
    "OriginSentinelDefeated",
    "seal_points =",
    "enumerate(seal_points, start=1)",
    "sentinel_points =",
    "enumerate(sentinel_points, start=1)",
    "defeat_quest_event_tag",
    "requires_interaction",
    "E  STABILIZE ORIGIN SEAL",
    "next_chapter_map_name",
    "CH20_TheFinalHunter",
    "NavMeshBoundsVolume",
    "save_current_level",
]
missing = [token for token in required if token not in source]
if missing: raise SystemExit("Chapter 19 generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
assert source.count("OriginGateSealStabilized") == 1
print("Chapter 19 Unreal Editor level generator contract passed.")
