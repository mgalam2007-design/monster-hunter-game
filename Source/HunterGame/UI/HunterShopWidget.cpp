#include "HunterShopWidget.h"

#include "ArinValeCharacter.h"
#include "Components/HunterCurrencyComponent.h"
#include "Components/HunterItemCatalog.h"

void UHunterShopWidget::InitializeShop(UHunterShopComponent* InShop, AArinValeCharacter* InBuyer)
{
    if (ShopComponent)
    {
        ShopComponent->OnShopChanged.RemoveDynamic(this, &UHunterShopWidget::HandleShopChanged);
    }
    if (CurrencyComponent)
    {
        CurrencyComponent->OnCurrencyChanged.RemoveDynamic(this, &UHunterShopWidget::HandleCurrencyChanged);
    }

    ShopComponent = InShop;
    BuyerCharacter = InBuyer;
    CurrencyComponent = BuyerCharacter ? BuyerCharacter->GetCurrencyComponent() : nullptr;

    if (ShopComponent)
    {
        ShopComponent->OnShopChanged.AddUniqueDynamic(this, &UHunterShopWidget::HandleShopChanged);
    }
    if (CurrencyComponent)
    {
        CurrencyComponent->OnCurrencyChanged.AddUniqueDynamic(this, &UHunterShopWidget::HandleCurrencyChanged);
    }

    RefreshShop();
}

void UHunterShopWidget::NativeDestruct()
{
    if (ShopComponent)
    {
        ShopComponent->OnShopChanged.RemoveDynamic(this, &UHunterShopWidget::HandleShopChanged);
    }
    if (CurrencyComponent)
    {
        CurrencyComponent->OnCurrencyChanged.RemoveDynamic(this, &UHunterShopWidget::HandleCurrencyChanged);
    }
    Super::NativeDestruct();
}

void UHunterShopWidget::HandleShopChanged()
{
    RefreshShop();
}

void UHunterShopWidget::HandleCurrencyChanged(int64 NewBalance)
{
    RefreshShop();
}

void UHunterShopWidget::RefreshShop()
{
    BP_OnShopSnapshotUpdated(GetShopSnapshot());
}

FHunterShopSnapshot UHunterShopWidget::GetShopSnapshot() const
{
    FHunterShopSnapshot Snapshot;
    if (!ShopComponent)
    {
        return Snapshot;
    }

    const UHunterCurrencyComponent* Currency = BuyerCharacter
        ? BuyerCharacter->GetCurrencyComponent()
        : nullptr;
    Snapshot.PlayerBalance = Currency ? Currency->GetBalance() : 0;

    const TArray<FHunterShopEntry> Listings = ShopComponent->GetCatalog();
    for (const FHunterShopEntry& Listing : Listings)
    {
        FHunterShopDisplayEntry Display;
        Display.Item = Listing.Item;
        Display.BuyPrice = Listing.BuyPrice;
        Display.SellPrice = Listing.SellPrice;
        Display.Stock = Listing.Stock;
        Display.bIsSoldOut = Listing.Stock == 0;
        Display.bCanAffordOne = !Display.bIsSoldOut
            && Listing.BuyPrice >= 0
            && Listing.BuyPrice <= Snapshot.PlayerBalance;

        if (ItemCatalog)
        {
            FHunterItemDefinition Definition;
            if (ItemCatalog->FindItem(Listing.Item.ItemId, Definition))
            {
                Display.Description = Definition.Description;
                Display.Rarity = StaticEnum<EHunterItemRarity>()->GetNameByValue(
                    static_cast<int64>(Definition.Rarity));
                Display.Icon = Definition.Icon;
            }
        }

        if (!Display.bIsSoldOut)
        {
            ++Snapshot.AvailableListingCount;
        }
        Snapshot.Entries.Add(MoveTemp(Display));
    }

    return Snapshot;
}

bool UHunterShopWidget::BuyItemById(FName ItemId, int32 Quantity)
{
    if (!ShopComponent || !BuyerCharacter || Quantity <= 0)
    {
        return false;
    }

    const bool bBought = ShopComponent->BuyItem(BuyerCharacter, ItemId, Quantity);
    RefreshShop();
    return bBought;
}

bool UHunterShopWidget::SellItemById(FName ItemId, int32 Quantity)
{
    if (!ShopComponent || !BuyerCharacter || Quantity <= 0)
    {
        return false;
    }

    const bool bSold = ShopComponent->SellItem(BuyerCharacter, ItemId, Quantity);
    RefreshShop();
    return bSold;
}
