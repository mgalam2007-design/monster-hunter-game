#include "HunterEquipmentComponent.h"

#include "ArinValeCharacter.h"
#include "ArinProgressionComponent.h"
#include "Misc/GuardValue.h"

bool UHunterEquipmentComponent::EquipItem(const FHunterEquipmentItem& NewItem)
{
    if (bEquipmentTransactionInProgress) return false;
    TGuardValue<bool> TransactionGuard(bEquipmentTransactionInProgress, true);
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
    if (!Inventory)
    {
        return false;
    }

    const EHunterItemType RequiredType = NewItem.Slot == EHunterEquipmentSlot::Weapon ? EHunterItemType::Weapon : EHunterItemType::Armor;
    FHunterInventoryItem SourceInventoryItem;
    bool bHasMatchingInventoryItem = false;
    for (const FHunterInventoryItem& InventoryItem : Inventory->GetItems())
    {
        if (InventoryItem.ItemId == NewItem.ItemId && InventoryItem.ItemType == RequiredType && InventoryItem.Quantity > 0)
        {
            SourceInventoryItem = InventoryItem;
            bHasMatchingInventoryItem = true;
            break;
        }
    }
    if (!bHasMatchingInventoryItem)
    {
        return false;
    }

    // Inventory metadata is authoritative. Never trust caller-supplied
    // combat stats or evolution requirements for an item ID.
    if (!FMath::IsFinite(SourceInventoryItem.DamageBonus) ||
        !FMath::IsFinite(SourceInventoryItem.DamageReduction) ||
        SourceInventoryItem.RequiredEvolutionStage < 0 ||
        SourceInventoryItem.RequiredEvolutionStage > 4)
    {
        return false;
    }

    const int32 RequiredStage = SourceInventoryItem.RequiredEvolutionStage;
    if (RequiredStage > 0 &&
        (!Progression || static_cast<int32>(Progression->GetEvolutionStage()) < RequiredStage))
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
        if (!Inventory->AddItem(SourceInventoryItem))
        {
            UE_LOG(LogTemp, Error, TEXT("Equipment rollback failed for item '%s'; manual inventory recovery may be required."), *NewItem.ItemId.ToString());
        }
        return false;
    }

    FHunterEquipmentItem Equipped;
    Equipped.ItemId = SourceInventoryItem.ItemId;
    Equipped.DisplayName = SourceInventoryItem.DisplayName;
    Equipped.Slot = NewItem.Slot;
    Equipped.DamageBonus = FMath::Max(0.0f, SourceInventoryItem.DamageBonus);
    Equipped.DamageReduction = FMath::Clamp(SourceInventoryItem.DamageReduction, 0.0f, 0.8f);
    Equipped.RequiredEvolutionStage = RequiredStage;
    Equipped.bIsEquipped = true;
    SlotItem = Equipped;

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
    if (bEquipmentTransactionInProgress) return false;
    TGuardValue<bool> TransactionGuard(bEquipmentTransactionInProgress, true);
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
    return FMath::IsFinite(EquippedWeapon.DamageBonus)
        ? FMath::Max(0.0f, EquippedWeapon.DamageBonus)
        : 0.0f;
}

float UHunterEquipmentComponent::GetDamageReduction() const
{
    return FMath::IsFinite(EquippedArmor.DamageReduction)
        ? FMath::Clamp(EquippedArmor.DamageReduction, 0.0f, 0.8f)
        : 0.0f;
}

void UHunterEquipmentComponent::RestoreEquipment(const FHunterEquipmentItem& Weapon, const FHunterEquipmentItem& Armor)
{
    EquippedWeapon = Weapon;
    EquippedArmor = Armor;

    const AArinValeCharacter* Arin = Cast<AArinValeCharacter>(GetOwner());
    const UArinProgressionComponent* Progression = Arin ? Arin->GetProgressionComponent() : nullptr;
    const int32 CurrentStage = Progression ? static_cast<int32>(Progression->GetEvolutionStage()) : 0;

    const auto SanitizeEquipment = [CurrentStage](FHunterEquipmentItem& Item, EHunterEquipmentSlot ExpectedSlot)
    {
        if (Item.ItemId.IsNone() || Item.Slot != ExpectedSlot ||
            !FMath::IsFinite(Item.DamageBonus) || !FMath::IsFinite(Item.DamageReduction) ||
            Item.RequiredEvolutionStage < 0 || Item.RequiredEvolutionStage > CurrentStage)
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
