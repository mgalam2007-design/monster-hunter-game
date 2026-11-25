from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HEADER = (ROOT / "Source/HunterGame/Characters/HunterBossBase.h").read_text(encoding="utf-8")
SOURCE = (ROOT / "Source/HunterGame/Characters/HunterBossBase.cpp").read_text(encoding="utf-8")

def require(condition, message):
    if not condition:
        raise SystemExit(f"FAIL: {message}")
    print(f"PASS: {message}")

require("AttackRangeIncreasePerPhase" in HEADER, "configurable phase reach increase exists")
require("BaseAttackRange" in HEADER and "BaseAttackRange = AttackRange" in SOURCE, "base melee reach is captured")
require("FMath::Clamp(AttackRangeIncreasePerPhase, 0.0f, 0.5f)" in SOURCE, "phase reach tuning is bounded")
require("AttackRange = BaseAttackRange * (1.0f + AttackRangeIncreasePerPhase * CurrentBossPhase)" in SOURCE, "later phases expand melee threat radius from baseline")
require("BP_OnBossPhaseChanged" in SOURCE, "existing phase presentation event remains")
print("Boss phase reach validation passed; Unreal compilation and runtime testing remain required.")
