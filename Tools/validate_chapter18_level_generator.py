from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter18_level.py"
source = SCRIPT.read_text(encoding="utf-8")
required = [
    'LEVEL_PATH = "/Game/Maps/Chapter18/L_CH18_CrownOfRuin"',
    "HunterPrototypeArenaActor",
    "HunterQuestTriggerActor",
    "HunterMonsterBase",
    "HunterChapterExitActor",
    "ThroneWardenDefeated",
    "RuinCommandCoreDestroyed",
    "warden_points =",
    "enumerate(warden_points, start=1)",
    "core_points =",
    "enumerate(core_points, start=1)",
    "defeat_quest_event_tag",
    "requires_interaction",
    "E  DESTROY COMMAND CORE",
    "next_chapter_map_name",
    "CH19_BeyondTheGate",
    "NavMeshBoundsVolume",
    "save_current_level",
]
missing = [token for token in required if token not in source]
if missing: raise SystemExit("Chapter 18 generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
assert source.count("ThroneWardenDefeated") == 1
print("Chapter 18 Unreal Editor level generator contract passed.")
