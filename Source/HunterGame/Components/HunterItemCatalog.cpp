#include "HunterItemCatalog.h"

bool UHunterItemCatalog::FindItem(FName ItemId, FHunterItemDefinition& OutDefinition) const
{
    if (ItemId.IsNone())
    {
        return false;
    }

    const FHunterItemDefinition* Found = Items.FindByPredicate([ItemId](const FHunterItemDefinition& Definition)
    {
        return Definition.InventoryData.ItemId == ItemId;
    });

    if (!Found)
    {
        return false;
    }

    OutDefinition = *Found;
    return true;
}
