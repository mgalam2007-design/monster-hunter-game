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
print("Monster blockout source checks passed.")
