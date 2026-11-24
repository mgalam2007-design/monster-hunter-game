from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter11_level.py"
source = SCRIPT.read_text(encoding="utf-8")
required = [
    'LEVEL_PATH = "/Game/Maps/Chapter11/L_CH11_EchoesOfTheFallen"',
    "HunterPrototypeArenaActor", "HunterQuestTriggerActor", "HunterBossEncounterActor",
    "HunterBossBase", "HunterChapterExitActor", "ExpeditionMemoryRecovered",
    "FallenCaptainDefeated", "memory_points =", "enumerate(memory_points, start=1)",
    "requires_interaction", "E  RECOVER MEMORY ECHO", "boss_defeat_quest_event_tag",
    "next_chapter_map_name", "CH12_ShadowAccord", "NavMeshBoundsVolume", "save_current_level",
]
missing = [token for token in required if token not in source]
if missing: raise SystemExit("Chapter 11 generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
assert source.count("ExpeditionMemoryRecovered") == 1
print("Chapter 11 Unreal Editor level generator contract passed.")
