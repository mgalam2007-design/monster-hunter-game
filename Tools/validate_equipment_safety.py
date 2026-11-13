#!/usr/bin/env python3
"""Validate inventory-backed equipment metadata and combat-stat safety."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "Source/HunterGame/Components/HunterEquipmentComponent.cpp").read_text(encoding="utf-8")

equip_start = SOURCE.find("bool UHunterEquipmentComponent::EquipItem")
equip_end = SOURCE.find("bool UHunterEquipmentComponent::EquipInventoryItem", equip_start)
assert equip_start >= 0 and equip_end > equip_start, "Could not locate EquipItem"
equip = SOURCE[equip_start:equip_end]
for token in (
    "!FMath::IsFinite(SourceInventoryItem.DamageBonus)",
    "!FMath::IsFinite(SourceInventoryItem.DamageReduction)",
    "SourceInventoryItem.RequiredEvolutionStage < 0",
    "SourceInventoryItem.RequiredEvolutionStage > 4",
    "const int32 RequiredStage = SourceInventoryItem.RequiredEvolutionStage;",
):
    assert token in equip, f"Missing inventory equipment validation: {token}"

for function, field in (
    ("float UHunterEquipmentComponent::GetTotalDamageBonus() const", "EquippedWeapon.DamageBonus"),
    ("float UHunterEquipmentComponent::GetDamageReduction() const", "EquippedArmor.DamageReduction"),
):
    start = SOURCE.find(function)
    end = SOURCE.find("\n}", start)
    assert start >= 0 and end > start, f"Could not locate {function}"
    body = SOURCE[start:end]
    assert f"FMath::IsFinite({field})" in body, f"Missing finite guard for {field}"
    assert ": 0.0f;" in body, f"Missing safe fallback for {field}"

print("Validated equipment inventory metadata and finite combat-stat fallbacks.")
