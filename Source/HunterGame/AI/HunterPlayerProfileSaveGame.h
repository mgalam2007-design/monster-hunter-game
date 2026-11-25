#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "HunterPlayerProfileSaveGame.generated.h"

/** Persisted online-clustering centroids for the local player profile. */
UCLASS()
class HUNTERGAME_API UHunterPlayerProfileSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, Category="AI|Player Profile")
    int32 SaveVersion = 1;

    UPROPERTY(VisibleAnywhere, Category="AI|Player Profile")
    TArray<FVector> Centroids;

    UPROPERTY(VisibleAnywhere, Category="AI|Player Profile")
    TArray<int64> ClusterCounts;

    UPROPERTY(VisibleAnywhere, Category="AI|Player Profile")
    int64 ObservationCount = 0;
};
