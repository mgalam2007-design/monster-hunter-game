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
    "HunterBreachEncounterActor",
    "HunterPickupActor",
    "HunterInventoryItem",
    "BreachShardCollected",
    "MonsterDefeated",
    "monster_spawn_points",
    "save_current_level",
]
missing = [token for token in required if token not in source]
if missing:
    raise SystemExit("Chapter 1 level generator missing: " + ", ".join(missing))
if "\\n    #" in source:
    raise SystemExit("Chapter 1 level generator contains a literal escaped newline in Python source")
compile(source, str(SCRIPT), "exec")
assert "HunterBreachEncounterActor" in source and "BreachShardCollected" in source
assert 'encounter = spawn(breach_class, (0, -650, 0), "CH01_FirstBreach_Encounter")' in source
assert "CH01_Entry_Checkpoint" in source and "CH01_PlayerStart" in source
print("Chapter 1 Unreal Editor level generator contract passed.")
