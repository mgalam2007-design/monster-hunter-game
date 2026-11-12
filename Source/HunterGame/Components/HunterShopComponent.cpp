#include "HunterShopComponent.h"

#include "ArinValeCharacter.h"
#include "Components/HunterCurrencyComponent.h"

FHunterShopEntry* UHunterShopComponent::FindEntry(FName ItemId)
{
    return Catalog.FindByPredicate([ItemId](const FHunterShopEntry& Entry)
    {
        return Entry.Item.ItemId == ItemId;
    });
}

const FHunterShopEntry* UHunterShopComponent::FindEntry(FName ItemId) const
{
    return Catalog.FindByPredicate([ItemId](const FHunterShopEntry& Entry)
    {
        return Entry.Item.ItemId == ItemId;
    });
}

bool UHunterShopComponent::BuyItem(FName ItemId, int32 Quantity)
{
    if (ItemId.IsNone() || Quantity <= 0)
    {
        return false;
    }

    FHunterShopEntry* Entry = FindEntry(ItemId);
    AArinValeCharacter* Arin = Cast<AArinValeCharacter>(GetOwner());
    UHunterInventoryComponent* Inventory = Arin ? Arin->GetInventoryComponent() : nullptr;
    UHunterCurrencyComponent* Currency = Arin ? Arin->GetCurrencyComponent() : nullptr;
    if (!Entry || !Inventory || !Currency || Entry->Item.Quantity <= 0 ||
        Entry->Item.MaxStack <= 0 || (Entry->Stock >= 0 && Entry->Stock < Quantity) ||
        Entry->BuyPrice < 0 || Entry->BuyPrice > MAX_int64 / Quantity)
    {
        return false;
    }

    FHunterInventoryItem Purchase = Entry->Item;
    Purchase.Quantity = Quantity;
    const int64 TotalCost = Entry->BuyPrice * static_cast<int64>(Quantity);
    if (!Currency->SpendCurrency(TotalCost))
    {
        return false;
    }

    if (!Inventory->AddItem(Purchase))
    {
        Currency->AddCurrency(TotalCost);
        return false;
    }

    if (Entry->Stock >= 0)
    {
        Entry->Stock -= Quantity;
    }
    OnShopChanged.Broadcast();
    return true;
}

bool UHunterShopComponent::SellItem(FName ItemId, int32 Quantity)
{
    if (ItemId.IsNone() || Quantity <= 0)
    {
        return false;
    }

    FHunterShopEntry* Entry = FindEntry(ItemId);
    AArinValeCharacter* Arin = Cast<AArinValeCharacter>(GetOwner());
    UHunterInventoryComponent* Inventory = Arin ? Arin->GetInventoryComponent() : nullptr;
    UHunterCurrencyComponent* Currency = Arin ? Arin->GetCurrencyComponent() : nullptr;
    if (!Entry || !Inventory || !Currency || Entry->SellPrice <= 0 ||
        Entry->SellPrice > MAX_int64 / Quantity || !Inventory->HasItem(ItemId, Quantity))
    {
        return false;
    }

    bool bSellable = false;
    for (const FHunterInventoryItem& Item : Inventory->GetItems())
    {
        if (Item.ItemId == ItemId && Item.Quantity > 0 && Item.ItemType != EHunterItemType::Quest)
        {
            bSellable = true;
            break;
        }
    }
    if (!bSellable)
    {
        return false;
    }

    const int64 Payout = Entry->SellPrice * static_cast<int64>(Quantity);
    if (!Inventory->RemoveItem(ItemId, Quantity))
    {
        return false;
    }

    if (!Currency->AddCurrency(Payout))
    {
        Entry->Item.Quantity = FMath::Max(1, Entry->Item.Quantity);
        Inventory->AddItem(Entry->Item);
        return false;
    }

    if (Entry->Stock >= 0)
    {
        Entry->Stock = Entry->Stock > MAX_int32 - Quantity ? MAX_int32 : Entry->Stock + Quantity;
    }
    OnShopChanged.Broadcast();
    return true;
}
