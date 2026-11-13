#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HunterPrototypeArenaActor.generated.h"

class UPointLightComponent;
class USceneComponent;
class UStaticMeshComponent;

/**
 * Engine-native greybox arena for early combat and encounter testing.
 * Replace this actor's primitive pieces with authored environment assets later.
 */
UCLASS(Blueprintable)
class HUNTERGAME_API AHunterPrototypeArenaActor : public AActor
{
    GENERATED_BODY()

public:
    AHunterPrototypeArenaActor();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Arena|Dimensions", meta=(ClampMin="800.0"))
    float ArenaHalfWidth = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Arena|Dimensions", meta=(ClampMin="800.0"))
    float ArenaHalfDepth = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Arena|Dimensions", meta=(ClampMin="300.0"))
    float WallHeight = 600.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Arena")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Arena")
    TArray<TObjectPtr<UStaticMeshComponent>> ArenaPieces;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Arena")
    TObjectPtr<UPointLightComponent> GateLight;
};
