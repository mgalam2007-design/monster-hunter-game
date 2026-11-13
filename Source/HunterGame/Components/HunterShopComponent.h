#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/HunterInventoryComponent.h"

#include "HunterShopComponent.generated.h"

class AArinValeCharacter;

USTRUCT(BlueprintType)
struct FHunterShopEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shop")
    FHunterInventoryItem Item;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shop", meta=(ClampMin="0"))
    int64 BuyPrice = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shop", meta=(ClampMin="0"))
    int64 SellPrice = 0;

    /** -1 means unlimited stock; zero means sold out. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shop", meta=(ClampMin="-1"))
    int32 Stock = -1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHunterShopChanged);

/** Data-driven vendor transactions. Add this component to a vendor Blueprint actor. */
UCLASS(ClassGroup=(Hunter), meta=(BlueprintSpawnableComponent))
class HUNTERGAME_API UHunterShopComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Shop")
    bool BuyItem(AArinValeCharacter* Buyer, FName ItemId, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category="Shop")
    bool SellItem(AArinValeCharacter* Seller, FName ItemId, int32 Quantity = 1);

    UFUNCTION(BlueprintPure, Category="Shop")
    TArray<FHunterShopEntry> GetCatalog() const { return Catalog; }

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop")
    TArray<FHunterShopEntry> Catalog;

    UPROPERTY(BlueprintAssignable, Category="Shop")
    FHunterShopChanged OnShopChanged;

private:
    FHunterShopEntry* FindEntry(FName ItemId);
    const FHunterShopEntry* FindEntry(FName ItemId) const;

    // Prevent event listeners from recursively starting overlapping transactions.
    bool bTransactionInProgress = false;
};
