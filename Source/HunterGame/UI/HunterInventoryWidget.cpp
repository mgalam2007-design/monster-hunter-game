#include "HunterInventoryWidget.h"

#include "ArinValeCharacter.h"
#include "Components/HunterHealthComponent.h"
#include "Components/HunterAbilityComponent.h"

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

            if (Item.ItemType == EHunterItemType::Consumable)
            {
                const UHunterHealthComponent* Health = ArinCharacter ? ArinCharacter->GetHealthComponent() : nullptr;
                const UHunterAbilityComponent* Abilities = ArinCharacter ? ArinCharacter->GetAbilityComponent() : nullptr;
                const bool bCanHeal = Health && Item.HealthRestoreAmount > 0.0f
                    && Health->GetCurrentHealth() < Health->GetMaxHealth();
                const bool bCanRestoreEnergy = Abilities && Item.EnergyRestoreAmount > 0.0f
                    && Abilities->GetCurrentEnergy() < Abilities->GetMaxEnergy();
                DisplayEntry.bCanUseConsumable = bCanHeal || bCanRestoreEnergy;
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

bool UHunterInventoryWidget::UseConsumableById(FName ItemId)
{
    if (ItemId.IsNone() || !InventoryComponent || !ArinCharacter)
    {
        return false;
    }

    const TArray<FHunterInventoryItem> Items = InventoryComponent->GetItems();
    const FHunterInventoryItem* FoundItem = Items.FindByPredicate([ItemId](const FHunterInventoryItem& Item)
    {
        return Item.ItemId == ItemId && Item.ItemType == EHunterItemType::Consumable && Item.Quantity > 0;
    });
    if (!FoundItem)
    {
        return false;
    }

    UHunterHealthComponent* Health = ArinCharacter->GetHealthComponent();
    UHunterAbilityComponent* Abilities = ArinCharacter->GetAbilityComponent();
    const bool bCanHeal = Health && FoundItem->HealthRestoreAmount > 0.0f
        && Health->GetCurrentHealth() < Health->GetMaxHealth();
    const bool bCanRestoreEnergy = Abilities && FoundItem->EnergyRestoreAmount > 0.0f
        && Abilities->GetCurrentEnergy() < Abilities->GetMaxEnergy();

    if ((!bCanHeal && !bCanRestoreEnergy) || (Health && Health->IsDead()))
    {
        return false;
    }

    const float HealthRestore = FoundItem->HealthRestoreAmount;
    const float EnergyRestore = FoundItem->EnergyRestoreAmount;
    if (!InventoryComponent->RemoveItem(ItemId, 1))
    {
        return false;
    }

    if (bCanHeal) Health->Heal(HealthRestore);
    if (bCanRestoreEnergy) Abilities->RestoreEnergy(EnergyRestore);
    RefreshInventory();
    return true;
}
