from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "Tools" / "unreal" / "create_chapter07_level.py"
source = SCRIPT.read_text(encoding="utf-8")
required = [
    'LEVEL_PATH = "/Game/Maps/Chapter07/L_CH07_BlueMark"',
    "HunterPrototypeArenaActor", "HunterQuestTriggerActor", "HunterChapterExitActor",
    "BlueMarkSourceLocated", "ResonanceFragmentCollected", "fragment_points =",
    "requires_interaction", "E  COLLECT RESONANCE", "RESONANCE FRAGMENT SECURED",
    "next_chapter_map_name", "CH08_HunterReborn", "NavMeshBoundsVolume", "save_current_level",
]
missing = [token for token in required if token not in source]
if missing: raise SystemExit("Chapter 7 generator missing: " + ", ".join(missing))
compile(source, str(SCRIPT), "exec")
print("Chapter 7 Unreal Editor level generator contract passed.")
