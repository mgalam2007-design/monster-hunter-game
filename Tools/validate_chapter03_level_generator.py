from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter03_level.py"
source = SCRIPT.read_text(encoding="utf-8")

required = [
    'LEVEL_PATH = "/Game/Maps/Chapter03/L_CH03_GuildEvaluation"',
    "HunterPrototypeArenaActor",
    "HunterMonsterBase",
    "HunterQuestTriggerActor",
    "HunterChapterExitActor",
    "TrialOpponentDefeated",
    "TrialSealActivated",
    "spawn_points =",
    "enumerate(spawn_points, start=1)",
    "defeat_quest_event_tag",
    "requires_interaction",
    "E  ACTIVATE TRIAL SEAL",
    "GUILD EVALUATION PASSED",
    "NavMeshBoundsVolume",
    "next_chapter_map_name",
    "CH04_TransitGate",
    "save_current_level",
]
missing = [token for token in required if token not in source]
if missing:
    raise SystemExit("Chapter 3 level generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
assert source.count("TrialOpponentDefeated") == 1
print("Chapter 3 Unreal Editor level generator contract passed.")
