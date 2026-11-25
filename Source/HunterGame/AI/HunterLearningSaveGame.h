#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "HunterLearningSaveGame.generated.h"

/** Local, offline storage for the game's learned combat policy. */
UCLASS()
class HUNTERGAME_API UHunterLearningSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, Category="Learning")
    int32 SaveVersion = 1;

    UPROPERTY(VisibleAnywhere, Category="Learning")
    TArray<float> QValues;

    UPROPERTY(VisibleAnywhere, Category="Learning")
    float ExplorationRate = 0.10f;

    UPROPERTY(VisibleAnywhere, Category="Learning")
    int64 UpdateCount = 0;
};
