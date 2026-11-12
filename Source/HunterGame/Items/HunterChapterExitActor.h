#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HunterChapterExitActor.generated.h"

class USphereComponent;
class UPrimitiveComponent;
class AArinValeCharacter;

/**
 * Chapter exit volume. Completes the current chapter when campaign rules allow it,
 * then autosaves the updated campaign state.
 */
UCLASS()
class HUNTERGAME_API AHunterChapterExitActor : public AActor
{
    GENERATED_BODY()

public:
    AHunterChapterExitActor();

    UFUNCTION(BlueprintCallable, Category="Campaign")
    bool TryCompleteChapter(AArinValeCharacter* Arin);

    UFUNCTION(BlueprintPure, Category="Campaign")
    bool HasActivated() const { return bHasActivated; }

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Campaign")
    TObjectPtr<USphereComponent> ExitTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Campaign|Trigger", meta=(ClampMin="50.0"))
    float TriggerRadius = 220.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Campaign|Save")
    FString SaveSlotName = TEXT("HunterCampaign");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Campaign|Save")
    int32 UserIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Campaign")
    bool bOneTimeActivation = true;

    UFUNCTION(BlueprintImplementableEvent, Category="Campaign")
    void BP_OnChapterExitResult(int32 CompletedChapterNumber, bool bChapterCompleted, bool bSaveSucceeded);

    UFUNCTION()
    void HandleExitOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

private:
    bool bHasActivated = false;
};
