from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter01_level.py"
source = SCRIPT.read_text(encoding="utf-8")

required = [
    "LEVEL_PATH = \"/Game/Maps/Chapter01/L_CH01_FirstBreach\"",
    "HunterPrototypeArenaActor",
    "ArinValeCharacter",
    "HunterMonsterBase",
    "HunterCheckpointActor",
    "HunterChapterExitActor",
    "NavMeshBoundsVolume",
    "CH01_BreachStalker_",
    "save_current_level",
    "AutoPossessAI",
]
missing = [token for token in required if token not in source]
if missing:
    raise SystemExit("Chapter 1 level generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
print("Chapter 1 Unreal Editor level generator contract passed.")
