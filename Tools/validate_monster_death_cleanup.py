from pathlib import Path

source = Path("Source/HunterGame/Characters/HunterMonsterBase.cpp").read_text(encoding="utf-8")
death_start = source.index("void AHunterMonsterBase::Die()")
death_end = source.index("void AHunterMonsterBase::SpawnLootDrops()", death_start)
death = source[death_start:death_end]

required = (
    "bIsDead = true;",
    "AttackTimer = 0.0f;",
    "ChaseRepathTimer = 0.0f;",
    "TargetCharacter.Reset();",
    "AI->StopMovement();",
    "GetCharacterMovement()->StopMovementImmediately();",
    "GetCharacterMovement()->DisableMovement();",
    "SetActorEnableCollision(false);",
)
missing = [token for token in required if token not in death]
if missing:
    raise SystemExit("Monster death cleanup missing: " + ", ".join(missing))
if death.index("bIsDead = true;") > death.index("BP_OnMonsterDeath();"):
    raise SystemExit("Monster must enter dead state before Blueprint death callback")
print("Monster death AI/movement cleanup checks passed.")
