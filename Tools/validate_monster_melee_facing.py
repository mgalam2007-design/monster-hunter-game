from pathlib import Path

source = Path("Source/HunterGame/Characters/HunterMonsterBase.cpp").read_text(encoding="utf-8")
stop = source.index("GetCharacterMovement()->StopMovementImmediately();")
attack = source.index("if (AttackTimer <= 0.0f && !bAttackPending)", stop)
facing = source.find("SetActorRotation(FMath::RInterpTo(GetActorRotation(), FacingRotation, DeltaSeconds, 10.0f));", stop, attack)
if facing < 0:
    raise SystemExit("Monster must smoothly face its target before melee windup logic")
for token in ("FMath::RInterpTo", "FacingRotation.Pitch = 0.0f", "FacingRotation.Roll = 0.0f"):
    if token not in source[stop:attack]:
        raise SystemExit("Monster facing logic missing: " + token)
print("Monster melee facing checks passed.")
