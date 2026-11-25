#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "HunterBehaviourModelSaveGame.generated.h"

UCLASS()
class HUNTERGAME_API UHunterBehaviourModelSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY()
    int32 SaveVersion = 1;

    UPROPERTY()
    TArray<float> Weights;

    UPROPERTY()
    int64 TrainingSamples = 0;
};
