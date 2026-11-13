#!/usr/bin/env python3
"""Static checks for Arin consumable and death-state input safeguards."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "Source/HunterGame/Characters/ArinValeCharacter.cpp"


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    start = source.find("void AArinValeCharacter::UseConsumable()")
    end = source.find("void AArinValeCharacter::ToggleInventoryMenu()", start)
    if start < 0 or end < 0:
        fail("could not locate consumable and inventory-menu functions")
    consumable = source[start:end]

    remove = consumable.find("InventoryComponent->RemoveItem(Item.ItemId, 1)")
    heal = consumable.find("HealthComponent->Heal(Item.HealthRestoreAmount)")
    energy = consumable.find("AbilityComponent->RestoreEnergy(Item.EnergyRestoreAmount)")
    if min(remove, heal, energy) < 0:
        fail("consumable removal and restoration calls must all be present")
    if not remove < heal or not remove < energy:
        fail("consumable must be removed before health or energy is granted")
    if "if (!InventoryComponent->RemoveItem(Item.ItemId, 1))" not in consumable:
        fail("failed consumable removal must abort the effect")

    menu_start = end
    menu_end = source.find("void AArinValeCharacter::HandleArinDeath()", menu_start)
    menu = source[menu_start:menu_end]
    if "HealthComponent->IsDead()" not in menu or "bRespawnPending" not in menu:
        fail("inventory menu must remain unavailable while dead or respawning")

    print("Validated consume-before-effect behavior and dead/respawn inventory input guard.")


if __name__ == "__main__":
    main()
