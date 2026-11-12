#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HunterQuestTriggerActor.generated.h"

class UBoxComponent;
class UPrimitiveComponent;

/**
 * World-space quest event volume. Place one in a level to report reach/interact-style
 * objectives when Arin enters its bounds.
 */
UCLASS()
class HUNTERGAME_API AHunterQuestTriggerActor : public AActor
{
    GENERATED_BODY()

public:
    AHunterQuestTriggerActor();

protected:
    virtual void BeginPlay() override;

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

    UFUNCTION(BlueprintImplementableEvent, Category="Quest Trigger")
    void BP_OnQuestEventTriggered(AActor* TriggeringActor);

private:
    bool bHasTriggered = false;

    UFUNCTION()
    void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);
};
