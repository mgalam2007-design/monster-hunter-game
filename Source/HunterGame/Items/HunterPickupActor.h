#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HunterInventoryComponent.h"
#include "HunterPickupActor.generated.h"

class USceneComponent;
class UPrimitiveComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class HUNTERGAME_API AHunterPickupActor : public AActor
{
    GENERATED_BODY()

public:
    AHunterPickupActor();

    UFUNCTION(BlueprintCallable, Category="Pickup")
    void SetPickupItem(const FHunterInventoryItem& NewItem);

    UFUNCTION(BlueprintCallable, Category="Pickup|Quest")
    void SetQuestEventTag(FName NewQuestEventTag) { QuestEventTag = NewQuestEventTag; }

    UFUNCTION(BlueprintPure, Category="Pickup")
    FHunterInventoryItem GetPickupItem() const { return PickupItem; }

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pickup")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pickup")
    TObjectPtr<UStaticMeshComponent> PickupMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pickup")
    TObjectPtr<USphereComponent> PickupCollision;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup")
    FHunterInventoryItem PickupItem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup")
    bool bDestroyAfterPickup = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup|Quest")
    FName QuestEventTag = NAME_None;

    UFUNCTION(BlueprintImplementableEvent, Category="Pickup")
    void BP_OnPickedUp(AActor* Picker);

private:
    UFUNCTION()
    void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);
};
