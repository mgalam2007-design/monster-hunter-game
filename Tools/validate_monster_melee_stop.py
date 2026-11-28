from pathlib import Path

source = Path("Source/HunterGame/Characters/HunterMonsterBase.cpp").read_text(encoding="utf-8")
range_start = source.index("    if (Distance > AttackRange)")
attack_start = source.index("    if (AttackTimer <= 0.0f && !bAttackPending)", range_start)
in_range = source[range_start:attack_start]
stop_ai = in_range.find("AI->StopMovement();")
stop_velocity = in_range.find("GetCharacterMovement()->StopMovementImmediately();")
if stop_ai < 0 or stop_velocity < 0:
    raise SystemExit("Monster must stop AI chase and movement when entering melee range")
if stop_ai < in_range.find("if (Distance > AttackRange)"):
    raise SystemExit("Chase movement must remain outside the melee-range branch")
print("Monster melee-range movement checks passed.")
