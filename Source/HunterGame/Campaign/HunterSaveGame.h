#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Components/HunterInventoryComponent.h"
#include "Components/HunterEquipmentComponent.h"
#include "Components/HunterEquipmentComponent.h"
#include "HunterSaveGame.generated.h"

/** Persistent single-player campaign slot for HUNTER 2.0. */
UCLASS()
class HUNTERGAME_API UHunterSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, Category="Save")
    int32 SaveVersion = 3;

    UPROPERTY(VisibleAnywhere, Category="Save")
    int32 CurrentChapterNumber = 1;

    UPROPERTY(VisibleAnywhere, Category="Save")
    TArray<int32> CompletedChapterNumbers;

    UPROPERTY(VisibleAnywhere, Category="Save")
    int32 ArinExperience = 0;

    UPROPERTY(VisibleAnywhere, Category="Save")
    int32 ArinEvolutionStage = 0;

    UPROPERTY(VisibleAnywhere, Category="Save")
    TArray<FHunterInventoryItem> ArinInventoryItems;

    UPROPERTY(VisibleAnywhere, Category="Save")
    FHunterEquipmentItem ArinEquippedWeapon;

    UPROPERTY(VisibleAnywhere, Category="Save")
    FHunterEquipmentItem ArinEquippedArmor;

    UPROPERTY(VisibleAnywhere, Category="Save")
    FHunterEquipmentItem ArinEquippedWeapon;

    UPROPERTY(VisibleAnywhere, Category="Save")
    FHunterEquipmentItem ArinEquippedArmor;
};
