#include "HunterShopComponent.h"

#include "ArinValeCharacter.h"
#include "Components/HunterCurrencyComponent.h"
#include "Misc/GuardValue.h"

FHunterShopEntry* UHunterShopComponent::FindEntry(FName ItemId)
{
    FHunterShopEntry* Match = nullptr;
    for (FHunterShopEntry& Entry : Catalog)
    {
        if (Entry.Item.ItemId != ItemId) continue;
        if (Match) return nullptr;
        Match = &Entry;
    }
    return Match;
}

const FHunterShopEntry* UHunterShopComponent::FindEntry(FName ItemId) const
{
    const FHunterShopEntry* Match = nullptr;
    for (const FHunterShopEntry& Entry : Catalog)
    {
        if (Entry.Item.ItemId != ItemId) continue;
        if (Match) return nullptr;
        Match = &Entry;
    }
    return Match;
}

bool UHunterShopComponent::BuyItem(AArinValeCharacter* Buyer, FName ItemId, int32 Quantity)
{
    if (bTransactionInProgress || ItemId.IsNone() || Quantity <= 0)
    {
        return false;
    }

    TGuardValue<bool> TransactionGuard(bTransactionInProgress, true);

    FHunterShopEntry* Entry = FindEntry(ItemId);
    UHunterInventoryComponent* Inventory = Buyer ? Buyer->GetInventoryComponent() : nullptr;
    UHunterCurrencyComponent* Currency = Buyer ? Buyer->GetCurrencyComponent() : nullptr;
    if (!Entry || !Inventory || !Currency ||
        static_cast<uint8>(Entry->Item.ItemType) > static_cast<uint8>(EHunterItemType::Quest) ||
        Entry->Item.ItemType == EHunterItemType::Quest || Entry->Item.Quantity <= 0 || Entry->Item.MaxStack <= 0 ||
        !FMath::IsFinite(Entry->Item.HealthRestoreAmount) || !FMath::IsFinite(Entry->Item.EnergyRestoreAmount) ||
        !FMath::IsFinite(Entry->Item.DamageBonus) || !FMath::IsFinite(Entry->Item.DamageReduction) ||
        (Entry->Stock >= 0 && Entry->Stock < Quantity) || Entry->BuyPrice < 0 || Entry->BuyPrice > MAX_int64 / Quantity)
    {
        return false;
    }

    FHunterInventoryItem Purchase = Entry->Item;
    Purchase.Quantity = Quantity;
    const int64 TotalCost = Entry->BuyPrice * static_cast<int64>(Quantity);
    if (TotalCost > 0 && !Currency->SpendCurrency(TotalCost))
    {
        return false;
    }

    if (!Inventory->AddItem(Purchase))
    {
        if (TotalCost > 0) Currency->AddCurrency(TotalCost);
        return false;
    }

    if (Entry->Stock >= 0)
    {
        Entry->Stock -= Quantity;
    }
    OnShopChanged.Broadcast();
    return true;
}

bool UHunterShopComponent::SellItem(AArinValeCharacter* Seller, FName ItemId, int32 Quantity)
{
    if (bTransactionInProgress || ItemId.IsNone() || Quantity <= 0)
    {
        return false;
    }

    TGuardValue<bool> TransactionGuard(bTransactionInProgress, true);

    FHunterShopEntry* Entry = FindEntry(ItemId);
    UHunterInventoryComponent* Inventory = Seller ? Seller->GetInventoryComponent() : nullptr;
    UHunterCurrencyComponent* Currency = Seller ? Seller->GetCurrencyComponent() : nullptr;
    if (!Entry || !Inventory || !Currency || Entry->SellPrice <= 0 ||
        Entry->SellPrice > MAX_int64 / Quantity || !Inventory->HasItem(ItemId, Quantity))
    {
        return false;
    }

    bool bSellable = false;
    bool bContainsQuestItem = false;
    for (const FHunterInventoryItem& Item : Inventory->GetItems())
    {
        if (Item.ItemId == ItemId && Item.Quantity > 0 && Item.ItemType != EHunterItemType::Quest)
        {
            bSellable = true;
        }
        if (Item.ItemId == ItemId && Item.ItemType == EHunterItemType::Quest && Item.Quantity > 0)
        {
            bContainsQuestItem = true;
        }
    }
    if (!bSellable || bContainsQuestItem)
    {
        return false;
    }

    const int64 Payout = Entry->SellPrice * static_cast<int64>(Quantity);
    if (Currency->GetBalance() > MAX_int64 - Payout)
    {
        return false;
    }

    // Keep an authoritative copy so a currency-credit failure cannot silently
    // destroy the item after inventory listeners have run.
    FHunterInventoryItem SoldItem;
    bool bFoundSoldItem = false;
    for (const FHunterInventoryItem& Item : Inventory->GetItems())
    {
        if (Item.ItemId == ItemId && Item.Quantity > 0)
        {
            SoldItem = Item;
            SoldItem.Quantity = Quantity;
            bFoundSoldItem = true;
            break;
        }
    }
    if (!bFoundSoldItem || !Inventory->RemoveItem(ItemId, Quantity))
    {
        return false;
    }

    if (!Currency->AddCurrency(Payout))
    {
        // A listener may have changed the wallet during the inventory-change
        // broadcast. Restore the sold quantity and leave shop stock untouched.
        if (!Inventory->AddItem(SoldItem))
        {
            UE_LOG(LogTemp, Error,
                TEXT("Shop transaction rollback failed for item %s (quantity %d)."),
                *ItemId.ToString(), Quantity);
        }
        return false;
    }

    if (Entry->Stock >= 0)
    {
        Entry->Stock = Entry->Stock > MAX_int32 - Quantity ? MAX_int32 : Entry->Stock + Quantity;
    }
    OnShopChanged.Broadcast();
    return true;
}
