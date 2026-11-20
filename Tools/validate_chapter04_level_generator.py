from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter04_level.py"
source = SCRIPT.read_text(encoding="utf-8")

required = [
    'LEVEL_PATH = "/Game/Maps/Chapter04/L_CH04_TransitGate"',
    "HunterPrototypeArenaActor",
    "HunterQuestTriggerActor",
    "HunterBossEncounterActor",
    "HunterBossBase",
    "HunterChapterExitActor",
    "TransitGateReached",
    "TransitGuardianDefeated",
    "boss_defeat_quest_event_tag",
    "boss_spawn_offset",
    "activation_radius",
    "NavMeshBoundsVolume",
    "save_current_level",
]
missing = [token for token in required if token not in source]
if missing:
    raise SystemExit("Chapter 4 level generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
assert "TransitGateReached" in source and "TransitGuardianDefeated" in source
print("Chapter 4 Unreal Editor level generator contract passed.")
