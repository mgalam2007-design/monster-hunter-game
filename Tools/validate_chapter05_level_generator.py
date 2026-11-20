from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter05_level.py"
source = SCRIPT.read_text(encoding="utf-8")

required = [
    'LEVEL_PATH = "/Game/Maps/Chapter05/L_CH05_MiningSettlement"',
    "HunterPrototypeArenaActor",
    "HunterMonsterBase",
    "HunterQuestTriggerActor",
    "HunterChapterExitActor",
    "MiningCreatureDefeated",
    "CorruptedOreCollected",
    "spawn_points =",
    "enumerate(spawn_points, start=1)",
    "defeat_quest_event_tag",
    "sample_points =",
    "requires_interaction",
    "E  RECOVER ORE SAMPLE",
    "CORRUPTED ORE RECOVERED",
    "NavMeshBoundsVolume",
    "save_current_level",
]
missing = [token for token in required if token not in source]
if missing:
    raise SystemExit("Chapter 5 level generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
assert source.count("MiningCreatureDefeated") == 1
assert source.count("CorruptedOreCollected") == 1
print("Chapter 5 Unreal Editor level generator contract passed.")
