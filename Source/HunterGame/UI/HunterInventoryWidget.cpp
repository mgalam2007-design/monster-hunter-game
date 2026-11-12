#include "HunterInventoryWidget.h"

#include "ArinValeCharacter.h"

void UHunterInventoryWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BindCharacter();
    RefreshInventory();
}

void UHunterInventoryWidget::NativeDestruct()
{
    UnbindCharacter();
    Super::NativeDestruct();
}

void UHunterInventoryWidget::BindCharacter()
{
    AArinValeCharacter* NewCharacter = Cast<AArinValeCharacter>(GetOwningPlayerPawn());
    if (ArinCharacter == NewCharacter && ArinCharacter)
    {
        return;
    }

    UnbindCharacter();
    ArinCharacter = NewCharacter;
    if (!ArinCharacter)
    {
        return;
    }

    InventoryComponent = ArinCharacter->GetInventoryComponent();
    EquipmentComponent = ArinCharacter->GetEquipmentComponent();

    if (InventoryComponent)
    {
        InventoryComponent->OnInventoryChanged.AddDynamic(this, &UHunterInventoryWidget::HandleInventoryChanged);
    }
    if (EquipmentComponent)
    {
        EquipmentComponent->OnEquipmentChanged.AddDynamic(this, &UHunterInventoryWidget::HandleEquipmentChanged);
    }
}

void UHunterInventoryWidget::UnbindCharacter()
{
    if (InventoryComponent)
    {
        InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UHunterInventoryWidget::HandleInventoryChanged);
    }
    if (EquipmentComponent)
    {
        EquipmentComponent->OnEquipmentChanged.RemoveDynamic(this, &UHunterInventoryWidget::HandleEquipmentChanged);
    }

    InventoryComponent = nullptr;
    EquipmentComponent = nullptr;
    ArinCharacter = nullptr;
}

void UHunterInventoryWidget::RefreshInventory()
{
    if (!ArinCharacter)
    {
        BindCharacter();
    }

    Snapshot = FHunterInventorySnapshot();
    if (InventoryComponent)
    {
        const TArray<FHunterInventoryItem> Items = InventoryComponent->GetItems();
        Snapshot.TotalItemStacks = Items.Num();

        for (const FHunterInventoryItem& Item : Items)
        {
            FHunterInventoryDisplayEntry DisplayEntry;
            DisplayEntry.Item = Item;
            Snapshot.TotalItemCount += Item.Quantity;

            if (ItemCatalog)
            {
                FHunterItemDefinition Definition;
                if (ItemCatalog->FindItem(Item.ItemId, Definition))
                {
                    DisplayEntry.Description = Definition.Description;
                    DisplayEntry.Rarity = Definition.Rarity;
                    DisplayEntry.Icon = Definition.Icon;
                }
            }

            if (EquipmentComponent)
            {
                const FHunterEquipmentItem Weapon = EquipmentComponent->GetWeapon();
                const FHunterEquipmentItem Armor = EquipmentComponent->GetArmor();
                DisplayEntry.bIsEquipped = Weapon.ItemId == Item.ItemId || Armor.ItemId == Item.ItemId;
            }

            Snapshot.Entries.Add(DisplayEntry);
        }
    }

    if (EquipmentComponent)
    {
        Snapshot.EquippedWeapon = EquipmentComponent->GetWeapon();
        Snapshot.EquippedArmor = EquipmentComponent->GetArmor();
    }

    BP_OnInventorySnapshotUpdated(Snapshot);
}

bool UHunterInventoryWidget::EquipItemById(FName ItemId)
{
    if (!EquipmentComponent || !EquipmentComponent->EquipInventoryItem(ItemId))
    {
        return false;
    }

    RefreshInventory();
    return true;
}

bool UHunterInventoryWidget::UnequipSlot(EHunterEquipmentSlot Slot)
{
    if (!EquipmentComponent || !EquipmentComponent->UnequipSlot(Slot))
    {
        return false;
    }

    RefreshInventory();
    return true;
}

void UHunterInventoryWidget::HandleInventoryChanged(FName ItemId)
{
    RefreshInventory();
}

void UHunterInventoryWidget::HandleEquipmentChanged()
{
    RefreshInventory();
}
