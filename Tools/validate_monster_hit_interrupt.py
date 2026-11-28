from pathlib import Path

source = Path("Source/HunterGame/Characters/HunterMonsterBase.cpp").read_text(encoding="utf-8")
start = source.index("void AHunterMonsterBase::ReceiveHunterDamage")
end = source.index("void AHunterMonsterBase::", start + 10)
damage = source[start:end]
for token in ("if (bAttackPending)", "bAttackPending = false;", "PendingAttackTarget.Reset();", "AttackWindupTimer = 0.0f;", "AttackWarningVisual->SetHiddenInGame(true)"):
    if token not in damage:
        raise SystemExit("Monster hit interruption missing: " + token)
if damage.index("CurrentHealth = FMath::Clamp") > damage.index("if (bAttackPending)"):
    raise SystemExit("Hit interruption should occur after valid damage is applied")
print("Monster hit-interruption checks passed.")
