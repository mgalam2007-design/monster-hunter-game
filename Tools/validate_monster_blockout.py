from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HEADER = (ROOT / "Source/HunterGame/Characters/HunterMonsterBase.h").read_text(encoding="utf-8")
SOURCE = (ROOT / "Source/HunterGame/Characters/HunterMonsterBase.cpp").read_text(encoding="utf-8")

required_parts = [
    "MonsterTorso", "MonsterShoulderLeft", "MonsterShoulderRight",
    "MonsterArmLeft", "MonsterArmRight", "MonsterLegLeft",
    "MonsterLegRight", "MonsterHead", "MonsterHornLeft", "MonsterHornRight",
]
for part in required_parts:
    assert part in SOURCE, f"Missing monster blockout part: {part}"

assert "TArray<TObjectPtr<UStaticMeshComponent>> BlockoutVisuals" in HEADER
assert "/Engine/BasicShapes/Cube.Cube" in SOURCE
assert "/Engine/BasicShapes/Sphere.Sphere" in SOURCE
assert "SetCollisionEnabled(ECollisionEnabled::NoCollision)" in SOURCE
assert "SetCanEverAffectNavigation(false)" in SOURCE
assert "SetupAttachment(GetRootComponent())" in SOURCE
assert "AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned" in SOURCE
assert "GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed" in SOURCE
assert "ChaseRepathInterval" in HEADER
assert "ChaseRepathTimer <= 0.0f" in SOURCE
assert "ChaseRepathTimer = FMath::Max(0.05f, ChaseRepathInterval)" in SOURCE
# Editable Blueprint defaults are normalized before entering combat.
assert "FMath::IsFinite(MaxHealth)" in SOURCE
assert "FMath::IsFinite(DetectionRadius)" in SOURCE
assert "FMath::IsFinite(AttackRange)" in SOURCE
assert "FMath::IsFinite(AttackDamage)" in SOURCE
assert "FMath::IsFinite(AttackCooldown)" in SOURCE
assert "FMath::IsFinite(ChaseSpeed)" in SOURCE
assert "FMath::IsFinite(ChaseRepathInterval)" in SOURCE
assert "FMath::Max(0.0f, DetectionRadius)" in SOURCE
assert "FMath::Max(0.0f, AttackRange)" in SOURCE
assert "FMath::Max(0.0f, AttackDamage)" in SOURCE
assert "FMath::Max(0.1f, AttackCooldown)" in SOURCE
assert "AttackWindupDuration" in HEADER
assert "BP_OnMonsterAttackTelegraph(AActor* Target)" in HEADER
assert "AttackWindupDuration = FMath::IsFinite(AttackWindupDuration)" in SOURCE
assert "FMath::Clamp(AttackWindupDuration, 0.0f, 2.0f)" in SOURCE
assert "AttackWindupTimer = FMath::Max(0.0f, AttackWindupTimer - DeltaSeconds)" in SOURCE
assert "PendingAttackTarget = Target" in SOURCE
assert "BP_OnMonsterAttackTelegraph(Target)" in SOURCE
assert "Distance > AttackRange" in SOURCE
assert "BP_OnMonsterAttack(Target)" in SOURCE
assert "bAttackPending = false" in SOURCE
assert "PendingAttackTarget.Reset()" in SOURCE
assert "FMath::Max(0.0f, ChaseSpeed)" in SOURCE
assert "FMath::Max(0.05f, ChaseRepathInterval)" in SOURCE
assert "return FMath::Clamp(SafeCurrentHealth / SafeMaxHealth, 0.0f, 1.0f)" in SOURCE
print("Monster blockout source checks passed.")
