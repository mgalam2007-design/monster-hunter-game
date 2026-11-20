from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter06_level.py"
source = SCRIPT.read_text(encoding="utf-8")

required = [
    'LEVEL_PATH = "/Game/Maps/Chapter06/L_CH06_TrialOfSteel"',
    "HunterPrototypeArenaActor",
    "HunterQuestTriggerActor",
    "HunterBossEncounterActor",
    "HunterBossBase",
    "HunterChapterExitActor",
    "SteelTrialOpponentDefeated",
    "SteelTrialApproved",
    "boss_defeat_quest_event_tag",
    "boss_spawn_offset",
    "activation_radius",
    "requires_interaction",
    "E  CLAIM TRIAL APPROVAL",
    "TRIAL OF STEEL PASSED",
    "NavMeshBoundsVolume",
    "save_current_level",
]
missing = [token for token in required if token not in source]
if missing:
    raise SystemExit("Chapter 6 level generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
assert source.count("SteelTrialOpponentDefeated") == 1
assert source.count("SteelTrialApproved") == 1
print("Chapter 6 Unreal Editor level generator contract passed.")
