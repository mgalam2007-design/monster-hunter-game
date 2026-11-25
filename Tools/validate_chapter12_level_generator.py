from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter12_level.py"
source = SCRIPT.read_text(encoding="utf-8")
required = [
    'LEVEL_PATH = "/Game/Maps/Chapter12/L_CH12_ShadowAccord"',
    "HunterPrototypeArenaActor", "HunterQuestTriggerActor", "HunterMonsterBase",
    "HunterChapterExitActor", "ShadowContactReached", "ShadowSentinelDefeated",
    "ShadowPactResolved", "sentinel_points =", "enumerate(sentinel_points, start=1)",
    "defeat_quest_event_tag", "requires_interaction", "E  BIND THE SHADOWS",
    "E  FREE THE SHADOWS", "story_choice_value", "BindShadows", "FreeShadows",
    "next_chapter_map_name", "CH13_SiegeOfTheNorth", "NavMeshBoundsVolume", "save_current_level",
]
missing = [token for token in required if token not in source]
if missing: raise SystemExit("Chapter 12 generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
assert source.count("ShadowSentinelDefeated") == 1
print("Chapter 12 Unreal Editor level generator contract passed.")
