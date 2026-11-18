from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter02_level.py"
source = SCRIPT.read_text(encoding="utf-8")

required = [
    'LEVEL_PATH = "/Game/Maps/Chapter02/L_CH02_MissingSurvivors"',
    "HunterPrototypeArenaActor",
    "HunterQuestTriggerActor",
    "HunterChapterExitActor",
    "SurvivorDistrictLocated",
    "SurvivorRescued",
    "requires_interaction",
    "interaction_prompt_text",
    "E  RESCUE CIVILIAN",
    "rescue_points",
    "def add_civilian_blockout(point, index):",
    "CH02_Civilian_%02d_Body",
    "CH02_Civilian_%02d_Head",
    "CH02_Civilian_%02d_%sArm",
    "CH02_Civilian_%02d_%sLeg",
    "set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)",
    "add_civilian_blockout(point, index)",
    "NavMeshBoundsVolume",
    "save_current_level",
]
missing = [token for token in required if token not in source]
if missing:
    raise SystemExit("Chapter 2 level generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
assert 'event_tag == "SurvivorRescued"' in source
assert "enumerate(rescue_points, start=1)" in source
print("Chapter 2 Unreal Editor level generator contract passed.")
