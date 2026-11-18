#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputCoreTypes.h"
#include "HunterQuestTriggerActor.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class AArinValeCharacter;
class APlayerController;

/**
 * World-space quest event volume. Can report immediately on overlap or wait
 * for an explicit interaction key while Arin remains inside the volume.
 */
UCLASS()
class HUNTERGAME_API AHunterQuestTriggerActor : public AActor
{
    GENERATED_BODY()

public:
    AHunterQuestTriggerActor();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Quest Trigger")
    TObjectPtr<UBoxComponent> TriggerVolume;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Trigger")
    FName QuestEventTag = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Trigger", meta=(ClampMin="1"))
    int32 EventAmount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Trigger")
    bool bTriggerOnce = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Trigger")
    bool bDestroyAfterTrigger = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Trigger|Interaction")
    bool bRequiresInteraction = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quest Trigger|Interaction")
    FKey InteractKey = EKeys::E;

    UFUNCTION(BlueprintImplementableEvent, Category="Quest Trigger")
    void BP_OnQuestEventTriggered(AActor* TriggeringActor);

    UFUNCTION(BlueprintImplementableEvent, Category="Quest Trigger")
    void BP_OnInteractionAvailable(bool bAvailable);

private:
    bool bHasTriggered = false;
    bool bInputBound = false;
    TWeakObjectPtr<AArinValeCharacter> NearbyArin;
    TWeakObjectPtr<APlayerController> NearbyController;

    UFUNCTION()
    void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

    void HandleInteractPressed();
    bool TryRegisterQuestEvent(AArinValeCharacter* Arin);
    void ClearInteractionInput();
};
