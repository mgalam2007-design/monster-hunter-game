#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HunterShadowEchoActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;

/**
 * Short-lived native Shadow Echo. It pulses damage to nearby monsters while
 * remaining a readable, non-blocking blockout silhouette.
 */
UCLASS()
class HUNTERGAME_API AHunterShadowEchoActor : public AActor
{
    GENERATED_BODY()

public:
    AHunterShadowEchoActor();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shadow Echo")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shadow Echo")
    TObjectPtr<UStaticMeshComponent> EchoVisual;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Shadow Echo|Combat", meta=(ClampMin="0.0"))
    float PulseRadius = 360.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Shadow Echo|Combat", meta=(ClampMin="0.0"))
    float PulseDamage = 22.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Shadow Echo|Combat", meta=(ClampMin="0.05"))
    float PulseInterval = 0.8f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Shadow Echo|Lifetime", meta=(ClampMin="0.1"))
    float EchoLifetime = 6.0f;

private:
    FTimerHandle PulseTimerHandle;
    FTimerHandle LifetimeTimerHandle;
    bool bEnding = false;

    void PulseNearbyMonsters();
    void ExpireEcho();
};
