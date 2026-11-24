from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter17_level.py"
source = SCRIPT.read_text(encoding="utf-8")
required = [
    'LEVEL_PATH = "/Game/Maps/Chapter17/L_CH17_LastSanctuary"',
    "HunterPrototypeArenaActor",
    "HunterQuestTriggerActor",
    "HunterMonsterBase",
    "HunterChapterExitActor",
    "SanctuaryGroupEvacuated",
    "SanctuaryInvaderDefeated",
    "evacuation_points =",
    "enumerate(evacuation_points, start=1)",
    "invader_points =",
    "enumerate(invader_points, start=1)",
    "defeat_quest_event_tag",
    "requires_interaction",
    "E  ESCORT CIVILIANS TO SAFETY",
    "next_chapter_map_name",
    "CH18_CrownOfRuin",
    "NavMeshBoundsVolume",
    "save_current_level",
]
missing = [token for token in required if token not in source]
if missing: raise SystemExit("Chapter 17 generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
assert source.count("SanctuaryGroupEvacuated") == 1
print("Chapter 17 Unreal Editor level generator contract passed.")
