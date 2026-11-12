#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Components/HunterInventoryComponent.h"
#include "HunterItemCatalog.generated.h"

UENUM(BlueprintType)
enum class EHunterItemRarity : uint8
{
    Common,
    Uncommon,
    Rare,
    Epic,
    Legendary,
    Mythic
};

USTRUCT(BlueprintType)
struct FHunterItemDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
    FHunterInventoryItem InventoryData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item", meta=(MultiLine="true"))
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
    EHunterItemRarity Rarity = EHunterItemRarity::Common;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item", meta=(ClampMin="0"))
    int64 BaseSellValue = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
    TObjectPtr<UTexture2D> Icon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
    TObjectPtr<UStaticMesh> WorldMesh = nullptr;
};

/** Shared item definitions for pickups, shops, inventory screens and equipment menus. */
UCLASS(BlueprintType)
class HUNTERGAME_API UHunterItemCatalog : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Catalog")
    TArray<FHunterItemDefinition> Items;

    UFUNCTION(BlueprintPure, Category="Catalog")
    bool FindItem(FName ItemId, FHunterItemDefinition& OutDefinition) const;

    UFUNCTION(BlueprintPure, Category="Catalog")
    TArray<FHunterItemDefinition> GetAllItems() const { return Items; }
};
