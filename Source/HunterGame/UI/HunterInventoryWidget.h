#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/HunterInventoryComponent.h"
#include "Components/HunterEquipmentComponent.h"
#include "Components/HunterItemCatalog.h"
#include "HunterInventoryWidget.generated.h"

class AArinValeCharacter;

USTRUCT(BlueprintType)
struct FHunterInventoryDisplayEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Inventory")
    FHunterInventoryItem Item;

    UPROPERTY(BlueprintReadOnly, Category="Inventory")
    FText Description;

    UPROPERTY(BlueprintReadOnly, Category="Inventory")
    EHunterItemRarity Rarity = EHunterItemRarity::Common;

    UPROPERTY(BlueprintReadOnly, Category="Inventory")
    TObjectPtr<UTexture2D> Icon = nullptr;

    UPROPERTY(BlueprintReadOnly, Category="Inventory")
    bool bCanUseConsumable = false;
};

USTRUCT(BlueprintType)
struct FHunterInventorySnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Inventory")
    TArray<FHunterInventoryDisplayEntry> Entries;

    UPROPERTY(BlueprintReadOnly, Category="Inventory")
    FHunterEquipmentItem EquippedWeapon;

    UPROPERTY(BlueprintReadOnly, Category="Inventory")
    FHunterEquipmentItem EquippedArmor;

    UPROPERTY(BlueprintReadOnly, Category="Inventory")
    int32 TotalItemStacks = 0;

    UPROPERTY(BlueprintReadOnly, Category="Inventory")
    int32 TotalItemCount = 0;
};

UCLASS()
class HUNTERGAME_API UHunterInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category="Inventory")
    FHunterInventorySnapshot GetInventorySnapshot() const { return Snapshot; }

    UFUNCTION(BlueprintCallable, Category="Inventory")
    void RefreshInventory();

    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool EquipItemById(FName ItemId);

    UFUNCTION(BlueprintCallable, Category="Equipment")
    bool UnequipSlot(EHunterEquipmentSlot Slot);

    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool UseConsumableById(FName ItemId);

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeDestruct() override;

    UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
    void BP_OnInventorySnapshotUpdated(const FHunterInventorySnapshot& NewSnapshot);

private:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UHunterItemCatalog> ItemCatalog;

    UPROPERTY(Transient)
    TObjectPtr<AArinValeCharacter> ArinCharacter;

    UPROPERTY(Transient)
    TObjectPtr<UHunterInventoryComponent> InventoryComponent;

    UPROPERTY(Transient)
    TObjectPtr<UHunterEquipmentComponent> EquipmentComponent;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Inventory", meta=(AllowPrivateAccess="true"))
    FHunterInventorySnapshot Snapshot;

    UFUNCTION()
    void HandleInventoryChanged(FName ItemId);

    UFUNCTION()
    void HandleEquipmentChanged();

    void BindCharacter();
    void UnbindCharacter();
};
