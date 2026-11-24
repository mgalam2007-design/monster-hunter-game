from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter10_level.py"
source = SCRIPT.read_text(encoding="utf-8")
required = [
    'LEVEL_PATH = "/Game/Maps/Chapter10/L_CH10_HollowDistrict"',
    "HunterPrototypeArenaActor", "HunterQuestTriggerActor", "HunterBossEncounterActor",
    "HunterBossBase", "HunterChapterExitActor", "HollowDistrictCoreReached",
    "ShadowAnchorDestroyed", "HollowWardenDefeated", "anchor_points =",
    "enumerate(anchor_points, start=1)", "requires_interaction", "E  DESTROY SHADOW ANCHOR",
    "boss_defeat_quest_event_tag", "boss_spawn_offset", "NavMeshBoundsVolume", "save_current_level",
    "next_chapter_map_name", "CH11_EchoesOfTheFallen",
]
missing = [token for token in required if token not in source]
if missing: raise SystemExit("Chapter 10 generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
assert source.count("ShadowAnchorDestroyed") == 1
print("Chapter 10 Unreal Editor level generator contract passed.")
