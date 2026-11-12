#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/HunterShopComponent.h"
#include "HunterShopWidget.generated.h"

class AArinValeCharacter;
class UHunterItemCatalog;
class UTexture2D;

USTRUCT(BlueprintType)
struct FHunterShopDisplayEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Shop")
    FHunterInventoryItem Item;

    UPROPERTY(BlueprintReadOnly, Category="Shop")
    FText Description;

    UPROPERTY(BlueprintReadOnly, Category="Shop")
    FName Rarity;

    UPROPERTY(BlueprintReadOnly, Category="Shop")
    TObjectPtr<UTexture2D> Icon = nullptr;

    UPROPERTY(BlueprintReadOnly, Category="Shop")
    int64 BuyPrice = 0;

    UPROPERTY(BlueprintReadOnly, Category="Shop")
    int64 SellPrice = 0;

    UPROPERTY(BlueprintReadOnly, Category="Shop")
    int32 Stock = -1;

    UPROPERTY(BlueprintReadOnly, Category="Shop")
    bool bCanAffordOne = false;

    UPROPERTY(BlueprintReadOnly, Category="Shop")
    bool bIsSoldOut = false;
};

USTRUCT(BlueprintType)
struct FHunterShopSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Shop")
    TArray<FHunterShopDisplayEntry> Entries;

    UPROPERTY(BlueprintReadOnly, Category="Shop")
    int64 PlayerBalance = 0;

    UPROPERTY(BlueprintReadOnly, Category="Shop")
    int32 AvailableListingCount = 0;
};

/** Blueprint-facing data and transaction bridge for a vendor screen. */
UCLASS()
class HUNTERGAME_API UHunterShopWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Shop")
    void InitializeShop(UHunterShopComponent* InShop, AArinValeCharacter* InBuyer);

    UFUNCTION(BlueprintCallable, Category="Shop")
    bool BuyItemById(FName ItemId, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category="Shop")
    bool SellItemById(FName ItemId, int32 Quantity = 1);

    UFUNCTION(BlueprintPure, Category="Shop")
    FHunterShopSnapshot GetShopSnapshot() const;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Shop")
    TObjectPtr<UHunterItemCatalog> ItemCatalog = nullptr;

protected:
    virtual void NativeDestruct() override;

    UFUNCTION(BlueprintImplementableEvent, Category="Shop")
    void BP_OnShopSnapshotUpdated(const FHunterShopSnapshot& Snapshot);

private:
    UPROPERTY(Transient)
    TObjectPtr<UHunterShopComponent> ShopComponent = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<AArinValeCharacter> BuyerCharacter = nullptr;

    UFUNCTION()
    void HandleShopChanged();

    void RefreshShop();
};
