#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HunterCheckpointActor.generated.h"

class USphereComponent;
class UPrimitiveComponent;
class AArinValeCharacter;

/**
 * World checkpoint that saves campaign and Arin progression when activated.
 * Place in a level and configure its save slot and activation radius.
 */
UCLASS()
class HUNTERGAME_API AHunterCheckpointActor : public AActor
{
    GENERATED_BODY()

public:
    AHunterCheckpointActor();

    UFUNCTION(BlueprintPure, Category="Checkpoint")
    bool HasBeenActivated() const { return bHasBeenActivated; }

    UFUNCTION(BlueprintCallable, Category="Checkpoint")
    bool ActivateCheckpoint(AArinValeCharacter* Arin);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Checkpoint")
    TObjectPtr<USphereComponent> ActivationSphere;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Checkpoint|Save")
    FString SaveSlotName = TEXT("HunterCampaign");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Checkpoint|Save")
    int32 UserIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Checkpoint|Save")
    FName CheckpointId = TEXT("Checkpoint_Default");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Checkpoint|Activation", meta=(ClampMin="50.0"))
    float ActivationRadius = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Checkpoint|Activation")
    bool bOneTimeActivation = true;

    UFUNCTION(BlueprintImplementableEvent, Category="Checkpoint")
    void BP_OnCheckpointActivated(FName ActivatedCheckpointId, bool bSaveSucceeded);

    UFUNCTION()
    void HandleActivationOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

private:
    bool bHasBeenActivated = false;
};
