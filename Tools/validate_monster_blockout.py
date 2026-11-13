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
assert "DetectionRadius = FMath::Max(0.0f, DetectionRadius)" in SOURCE
assert "AttackRange = FMath::Max(0.0f, AttackRange)" in SOURCE
assert "AttackDamage = FMath::Max(0.0f, AttackDamage)" in SOURCE
assert "AttackCooldown = FMath::Max(0.1f, AttackCooldown)" in SOURCE
assert "ChaseSpeed = FMath::Max(0.0f, ChaseSpeed)" in SOURCE
assert "ChaseRepathInterval = FMath::Max(0.05f, ChaseRepathInterval)" in SOURCE
assert "return CurrentHealth / FMath::Max(1.0f, MaxHealth)" in SOURCE
print("Monster blockout source checks passed.")
