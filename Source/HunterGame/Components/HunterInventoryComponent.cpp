#include "HunterInventoryComponent.h"

bool UHunterInventoryComponent::AddItem(FHunterInventoryItem Item)
{
    if (Item.ItemId.IsNone() || Item.Quantity <= 0 ||
        static_cast<uint8>(Item.ItemType) > static_cast<uint8>(EHunterItemType::Quest) ||
        !FMath::IsFinite(Item.HealthRestoreAmount) || !FMath::IsFinite(Item.EnergyRestoreAmount) ||
        !FMath::IsFinite(Item.DamageBonus) || !FMath::IsFinite(Item.DamageReduction))
    {
        return false;
    }

    const int64 ExistingTotal = static_cast<int64>(GetItemCount(Item.ItemId));
    if (ExistingTotal + static_cast<int64>(Item.Quantity) > MAX_int32)
    {
        return false;
    }

    Item.MaxStack = FMath::Max(1, Item.MaxStack);
    Item.HealthRestoreAmount = FMath::Max(0.0f, Item.HealthRestoreAmount);
    Item.EnergyRestoreAmount = FMath::Max(0.0f, Item.EnergyRestoreAmount);
    Item.DamageBonus = FMath::Max(0.0f, Item.DamageBonus);
    Item.DamageReduction = FMath::Clamp(Item.DamageReduction, 0.0f, 0.8f);
    Item.RequiredEvolutionStage = FMath::Clamp(Item.RequiredEvolutionStage, 0, 4);
    int32 Remaining = Item.Quantity;

    for (FHunterInventoryItem& Existing : Items)
    {
        if (Existing.ItemId != Item.ItemId)
        {
            continue;
        }

        const int32 Space = FMath::Max(0, Existing.MaxStack - Existing.Quantity);
        const int32 Added = FMath::Min(Space, Remaining);
        Existing.Quantity += Added;
        Remaining -= Added;
        if (Added > 0)
        {
            OnInventoryChanged.Broadcast(Item.ItemId);
        }
        if (Remaining <= 0)
        {
            return true;
        }
    }

    while (Remaining > 0)
    {
        FHunterInventoryItem NewStack = Item;
        NewStack.Quantity = FMath::Min(Item.MaxStack, Remaining);
        Items.Add(NewStack);
        Remaining -= NewStack.Quantity;
        OnInventoryChanged.Broadcast(Item.ItemId);
    }

    return true;
}

bool UHunterInventoryComponent::RemoveItem(FName ItemId, int32 Amount)
{
    if (ItemId.IsNone() || Amount <= 0 || GetItemCount(ItemId) < Amount)
    {
        return false;
    }

    int32 Remaining = Amount;
    for (int32 Index = Items.Num() - 1; Index >= 0 && Remaining > 0; --Index)
    {
        if (Items[Index].ItemId != ItemId)
        {
            continue;
        }

        const int32 Removed = FMath::Min(Items[Index].Quantity, Remaining);
        Items[Index].Quantity -= Removed;
        Remaining -= Removed;
        if (Items[Index].Quantity <= 0)
        {
            Items.RemoveAt(Index);
        }
    }

    OnInventoryChanged.Broadcast(ItemId);
    return true;
}

int32 UHunterInventoryComponent::GetItemCount(FName ItemId) const
{
    int64 Total = 0;
    for (const FHunterInventoryItem& Item : Items)
    {
        if (Item.ItemId == ItemId)
        {
            Total += Item.Quantity;
        }
    }
    return static_cast<int32>(FMath::Min<int64>(Total, MAX_int32));
}

bool UHunterInventoryComponent::HasItem(FName ItemId, int32 Amount) const
{
    return Amount > 0 && GetItemCount(ItemId) >= Amount;
}

void UHunterInventoryComponent::RestoreItems(const TArray<FHunterInventoryItem>& SavedItems)
{
    Items.Reset();
    for (FHunterInventoryItem Item : SavedItems)
    {
        if (Item.ItemId.IsNone() || Item.Quantity <= 0)
        {
            continue;
        }

        Item.MaxStack = FMath::Max(1, Item.MaxStack);
        AddItem(Item);
    }
    OnInventoryChanged.Broadcast(NAME_None);
}

void UHunterInventoryComponent::ClearInventory()
{
    Items.Reset();
    OnInventoryChanged.Broadcast(NAME_None);
}
