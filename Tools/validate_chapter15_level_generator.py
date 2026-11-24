from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter15_level.py"
source = SCRIPT.read_text(encoding="utf-8")
required = [
    'LEVEL_PATH = "/Game/Maps/Chapter15/L_CH15_WorldsUnsealed"',
    "HunterPrototypeArenaActor",
    "HunterQuestTriggerActor",
    "HunterMonsterBase",
    "HunterChapterExitActor",
    "BreachAnchorStabilized",
    "BreachGuardianDefeated",
    "anchor_points =",
    "enumerate(anchor_points, start=1)",
    "guardian_points =",
    "enumerate(guardian_points, start=1)",
    "defeat_quest_event_tag",
    "requires_interaction",
    "E  STABILIZE BREACH ANCHOR",
    "next_chapter_map_name",
    "CH16_SevenBeacons",
    "NavMeshBoundsVolume",
    "save_current_level",
]
missing = [token for token in required if token not in source]
if missing: raise SystemExit("Chapter 15 generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
assert source.count("BreachAnchorStabilized") == 1
print("Chapter 15 Unreal Editor level generator contract passed.")
