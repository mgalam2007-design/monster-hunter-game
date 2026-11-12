#include "HunterEquipmentComponent.h"

#include "ArinValeCharacter.h"
#include "ArinProgressionComponent.h"

bool UHunterEquipmentComponent::EquipItem(const FHunterEquipmentItem& NewItem)
{
    if (NewItem.ItemId.IsNone() || NewItem.RequiredEvolutionStage < 0)
    {
        return false;
    }

    AArinValeCharacter* Arin = Cast<AArinValeCharacter>(GetOwner());
    UHunterInventoryComponent* Inventory = Arin ? Arin->GetInventoryComponent() : nullptr;
    UArinProgressionComponent* Progression = Arin ? Arin->GetProgressionComponent() : nullptr;
    if (!Inventory || !Inventory->HasItem(NewItem.ItemId))
    {
        return false;
    }

    if (Progression && static_cast<int32>(Progression->GetEvolutionStage()) < NewItem.RequiredEvolutionStage)
    {
        return false;
    }

    const EHunterItemType RequiredType = NewItem.Slot == EHunterEquipmentSlot::Weapon ? EHunterItemType::Weapon : EHunterItemType::Armor;
    bool bHasMatchingInventoryItem = false;
    for (const FHunterInventoryItem& InventoryItem : Inventory->GetItems())
    {
        if (InventoryItem.ItemId == NewItem.ItemId && InventoryItem.ItemType == RequiredType && InventoryItem.Quantity > 0)
        {
            bHasMatchingInventoryItem = true;
            break;
        }
    }
    if (!bHasMatchingInventoryItem)
    {
        return false;
    }

    FHunterEquipmentItem& SlotItem = NewItem.Slot == EHunterEquipmentSlot::Weapon ? EquippedWeapon : EquippedArmor;
    if (SlotItem.ItemId == NewItem.ItemId)
    {
        return true;
    }

    if (!Inventory->RemoveItem(NewItem.ItemId, 1))
    {
        return false;
    }

    if (!ReturnEquippedItem(SlotItem))
    {
        FHunterInventoryItem RollbackItem;
        RollbackItem.ItemId = NewItem.ItemId;
        RollbackItem.DisplayName = NewItem.DisplayName;
        RollbackItem.ItemType = NewItem.Slot == EHunterEquipmentSlot::Weapon ? EHunterItemType::Weapon : EHunterItemType::Armor;
        RollbackItem.MaxStack = 1;
        RollbackItem.Quantity = 1;
        Inventory->AddItem(RollbackItem);
        return false;
    }

    SlotItem = NewItem;
    SlotItem.bIsEquipped = true;
    OnEquipmentChanged.Broadcast();
    return true;
}

bool UHunterEquipmentComponent::ReturnEquippedItem(FHunterEquipmentItem& Item)
{
    if (Item.ItemId.IsNone())
    {
        return true;
    }

    AArinValeCharacter* Arin = Cast<AArinValeCharacter>(GetOwner());
    UHunterInventoryComponent* Inventory = Arin ? Arin->GetInventoryComponent() : nullptr;
    if (!Inventory)
    {
        return false;
    }

    FHunterInventoryItem InventoryItem;
    InventoryItem.ItemId = Item.ItemId;
    InventoryItem.DisplayName = Item.DisplayName;
    InventoryItem.ItemType = Item.Slot == EHunterEquipmentSlot::Weapon ? EHunterItemType::Weapon : EHunterItemType::Armor;
    InventoryItem.Quantity = 1;
    InventoryItem.MaxStack = 1;
    if (!Inventory->AddItem(InventoryItem))
    {
        return false;
    }

    Item = FHunterEquipmentItem();
    return true;
}

bool UHunterEquipmentComponent::UnequipSlot(EHunterEquipmentSlot Slot)
{
    FHunterEquipmentItem& Item = Slot == EHunterEquipmentSlot::Weapon ? EquippedWeapon : EquippedArmor;
    if (Item.ItemId.IsNone() || !ReturnEquippedItem(Item))
    {
        return false;
    }

    OnEquipmentChanged.Broadcast();
    return true;
}

float UHunterEquipmentComponent::GetTotalDamageBonus() const
{
    return FMath::Max(0.0f, EquippedWeapon.DamageBonus);
}

float UHunterEquipmentComponent::GetDamageReduction() const
{
    return FMath::Clamp(EquippedArmor.DamageReduction, 0.0f, 0.8f);
}

void UHunterEquipmentComponent::RestoreEquipment(const FHunterEquipmentItem& Weapon, const FHunterEquipmentItem& Armor)
{
    EquippedWeapon = Weapon;
    EquippedArmor = Armor;
    EquippedWeapon.bIsEquipped = !EquippedWeapon.ItemId.IsNone();
    EquippedArmor.bIsEquipped = !EquippedArmor.ItemId.IsNone();
    OnEquipmentChanged.Broadcast();
}
