#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/HunterInventoryComponent.h"
#include "HunterEquipmentComponent.generated.h"

UENUM(BlueprintType)
enum class EHunterEquipmentSlot : uint8
{
    Weapon,
    Armor
};

USTRUCT(BlueprintType)
struct FHunterEquipmentItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment")
    FName ItemId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment")
    EHunterEquipmentSlot Slot = EHunterEquipmentSlot::Weapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment")
    float DamageBonus = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment", meta=(ClampMin="0.0", ClampMax="0.8"))
    float DamageReduction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment")
    int32 RequiredEvolutionStage = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment")
    bool bIsEquipped = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHunterEquipmentChanged);

UCLASS(ClassGroup=(Hunter), meta=(BlueprintSpawnableComponent))
class HUNTERGAME_API UHunterEquipmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Equipment")
    bool EquipItem(const FHunterEquipmentItem& Item);

    UFUNCTION(BlueprintCallable, Category="Equipment")
    bool UnequipSlot(EHunterEquipmentSlot Slot);

    UFUNCTION(BlueprintPure, Category="Equipment")
    FHunterEquipmentItem GetWeapon() const { return EquippedWeapon; }

    UFUNCTION(BlueprintPure, Category="Equipment")
    FHunterEquipmentItem GetArmor() const { return EquippedArmor; }

    UFUNCTION(BlueprintPure, Category="Equipment")
    float GetTotalDamageBonus() const;

    UFUNCTION(BlueprintPure, Category="Equipment")
    float GetDamageReduction() const;

    UFUNCTION(BlueprintCallable, Category="Equipment|Save")
    void RestoreEquipment(const FHunterEquipmentItem& Weapon, const FHunterEquipmentItem& Armor);

    UPROPERTY(BlueprintAssignable, Category="Equipment")
    FHunterEquipmentChanged OnEquipmentChanged;

private:
    UPROPERTY(VisibleAnywhere, Category="Equipment")
    FHunterEquipmentItem EquippedWeapon;

    UPROPERTY(VisibleAnywhere, Category="Equipment")
    FHunterEquipmentItem EquippedArmor;

    bool ReturnEquippedItem(FHunterEquipmentItem& Item);
};
