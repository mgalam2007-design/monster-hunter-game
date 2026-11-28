from pathlib import Path

source = Path("Source/HunterGame/Characters/HunterMonsterBase.cpp").read_text(encoding="utf-8")
attack = source.index("BP_OnMonsterAttack(Target);")
damage = source.index("AppliedDamage = UGameplayStatics::ApplyDamage", attack)
block = source.index("bAttackBlocked", attack)
if not (attack < block < damage):
    raise SystemExit("Monster attack line-of-sight check must precede damage application")
for token in ("LineTraceSingleByChannel", "ECC_Visibility", "AttackSightParams.AddIgnoredActor(this)", "AttackSightHit.GetActor() != Target"):
    if token not in source[attack:damage]:
        raise SystemExit("Monster attack line-of-sight check missing: " + token)
if "if (!bAttackBlocked && FMath::IsFinite(AttackDamage)" not in source[attack:damage]:
    raise SystemExit("Monster damage must be gated by clear line of sight")
print("Monster melee line-of-sight checks passed.")
