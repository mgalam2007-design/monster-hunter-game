#include "HunterEquipmentComponent.h"

#include "ArinValeCharacter.h"
#include "ArinProgressionComponent.h"

bool UHunterEquipmentComponent::EquipItem(const FHunterEquipmentItem& NewItem)
{
    if (NewItem.ItemId.IsNone() ||
        static_cast<uint8>(NewItem.Slot) > static_cast<uint8>(EHunterEquipmentSlot::Armor) ||
        NewItem.RequiredEvolutionStage < 0 || NewItem.RequiredEvolutionStage > 4 ||
        !FMath::IsFinite(NewItem.DamageBonus) || !FMath::IsFinite(NewItem.DamageReduction))
    {
        return false;
    }

    FHunterEquipmentItem& SlotItem = NewItem.Slot == EHunterEquipmentSlot::Weapon ? EquippedWeapon : EquippedArmor;
    if (SlotItem.ItemId == NewItem.ItemId)
    {
        return true;
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
    FHunterInventoryItem SourceInventoryItem;
    for (const FHunterInventoryItem& InventoryItem : Inventory->GetItems())
    {
        if (InventoryItem.ItemId == NewItem.ItemId && InventoryItem.ItemType == RequiredType && InventoryItem.Quantity > 0)
        {
            bHasMatchingInventoryItem = true;
            SourceInventoryItem = InventoryItem;
            break;
        }
    }
    if (!bHasMatchingInventoryItem)
    {
        return false;
    }

    if (!Inventory->RemoveItem(NewItem.ItemId, 1))
    {
        return false;
    }

    if (!ReturnEquippedItem(SlotItem))
    {
        SourceInventoryItem.Quantity = 1;
        Inventory->AddItem(SourceInventoryItem);
        return false;
    }

    SlotItem = NewItem;
    SlotItem.bIsEquipped = true;
    OnEquipmentChanged.Broadcast();
    return true;
}

bool UHunterEquipmentComponent::EquipInventoryItem(FName ItemId)
{
    AArinValeCharacter* Arin = Cast<AArinValeCharacter>(GetOwner());
    UHunterInventoryComponent* Inventory = Arin ? Arin->GetInventoryComponent() : nullptr;
    if (!Inventory || ItemId.IsNone()) return false;

    for (const FHunterInventoryItem& Entry : Inventory->GetItems())
    {
        if (Entry.ItemId != ItemId || Entry.Quantity <= 0) continue;
        FHunterEquipmentItem EquipmentItem;
        EquipmentItem.ItemId = Entry.ItemId;
        EquipmentItem.DisplayName = Entry.DisplayName;
        EquipmentItem.DamageBonus = Entry.DamageBonus;
        EquipmentItem.DamageReduction = Entry.DamageReduction;
        EquipmentItem.RequiredEvolutionStage = Entry.RequiredEvolutionStage;
        if (Entry.ItemType == EHunterItemType::Weapon)
        {
            EquipmentItem.Slot = EHunterEquipmentSlot::Weapon;
            return EquipItem(EquipmentItem);
        }
        if (Entry.ItemType == EHunterItemType::Armor)
        {
            EquipmentItem.Slot = EHunterEquipmentSlot::Armor;
            return EquipItem(EquipmentItem);
        }
        return false;
    }
    return false;
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
    InventoryItem.DamageBonus = Item.DamageBonus;
    InventoryItem.DamageReduction = Item.DamageReduction;
    InventoryItem.RequiredEvolutionStage = Item.RequiredEvolutionStage;
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

    const auto SanitizeEquipment = [](FHunterEquipmentItem& Item, EHunterEquipmentSlot ExpectedSlot)
    {
        if (Item.ItemId.IsNone() || Item.Slot != ExpectedSlot ||
            !FMath::IsFinite(Item.DamageBonus) || !FMath::IsFinite(Item.DamageReduction))
        {
            Item = FHunterEquipmentItem();
            return;
        }

        Item.DamageBonus = FMath::Max(0.0f, Item.DamageBonus);
        Item.DamageReduction = FMath::Clamp(Item.DamageReduction, 0.0f, 0.8f);
        Item.RequiredEvolutionStage = FMath::Clamp(Item.RequiredEvolutionStage, 0, 4);
        Item.bIsEquipped = true;
    };

    SanitizeEquipment(EquippedWeapon, EHunterEquipmentSlot::Weapon);
    SanitizeEquipment(EquippedArmor, EHunterEquipmentSlot::Armor);
    OnEquipmentChanged.Broadcast();
}
