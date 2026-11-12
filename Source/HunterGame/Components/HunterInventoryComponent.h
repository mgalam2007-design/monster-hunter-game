#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HunterInventoryComponent.generated.h"

UENUM(BlueprintType)
enum class EHunterItemType : uint8
{
    Consumable,
    Material,
    Weapon,
    Armor,
    Quest
};

USTRUCT(BlueprintType)
struct FHunterInventoryItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    FName ItemId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    EHunterItemType ItemType = EHunterItemType::Material;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory", meta=(ClampMin="1"))
    int32 MaxStack = 99;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory", meta=(ClampMin="1"))
    int32 Quantity = 1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHunterInventoryChanged, FName, ItemId);

UCLASS(ClassGroup=(Hunter), meta=(BlueprintSpawnableComponent))
class HUNTERGAME_API UHunterInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool AddItem(FHunterInventoryItem Item);

    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool RemoveItem(FName ItemId, int32 Amount = 1);

    UFUNCTION(BlueprintPure, Category="Inventory")
    int32 GetItemCount(FName ItemId) const;

    UFUNCTION(BlueprintPure, Category="Inventory")
    TArray<FHunterInventoryItem> GetItems() const { return Items; }

    UFUNCTION(BlueprintPure, Category="Inventory")
    bool HasItem(FName ItemId, int32 Amount = 1) const;

    UPROPERTY(BlueprintAssignable, Category="Inventory")
    FHunterInventoryChanged OnInventoryChanged;

private:
    UPROPERTY(VisibleAnywhere, Category="Inventory")
    TArray<FHunterInventoryItem> Items;
};
