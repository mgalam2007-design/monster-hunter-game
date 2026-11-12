#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HunterBreachEncounterActor.generated.h"

class USphereComponent;
class AHunterMonsterBase;
class AHunterPickupActor;
class AArinValeCharacter;

/**
 * Reusable encounter director for authored breach arenas.
 * Starts a configured enemy wave when Arin enters, then spawns a quest pickup
 * after every successfully spawned enemy has been defeated.
 */
UCLASS()
class HUNTERGAME_API AHunterBreachEncounterActor : public AActor
{
    GENERATED_BODY()

public:
    AHunterBreachEncounterActor();

    UFUNCTION(BlueprintPure, Category="Encounter")
    bool HasEncounterStarted() const { return bEncounterStarted; }

    UFUNCTION(BlueprintPure, Category="Encounter")
    bool IsEncounterComplete() const { return bEncounterComplete; }

    UFUNCTION(BlueprintCallable, Category="Encounter")
    void StartEncounter(AArinValeCharacter* Arin);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Encounter")
    TObjectPtr<USphereComponent> StartTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Encounter|Wave")
    TSubclassOf<AHunterMonsterBase> MonsterClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Encounter|Wave")
    TArray<FTransform> MonsterSpawnPoints;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Encounter|Wave", meta=(ClampMin="1"))
    int32 FallbackMonsterCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Encounter|Reward")
    TSubclassOf<AHunterPickupActor> RewardPickupClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Encounter|Reward")
    FHunterInventoryItem RewardItem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Encounter|Reward")
    FName RewardQuestEventTag = TEXT("BreachShardCollected");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Encounter|Trigger", meta=(ClampMin="50.0"))
    float TriggerRadius = 350.0f;

    UFUNCTION(BlueprintImplementableEvent, Category="Encounter")
    void BP_OnEncounterStarted(int32 SpawnedEnemyCount);

    UFUNCTION(BlueprintImplementableEvent, Category="Encounter")
    void BP_OnEncounterCompleted();

    UFUNCTION(BlueprintImplementableEvent, Category="Encounter")
    void BP_OnRewardSpawned(AHunterPickupActor* RewardPickup);

private:
    UPROPERTY(Transient)
    TArray<TObjectPtr<AHunterMonsterBase>> SpawnedMonsters;

    bool bEncounterStarted = false;
    bool bEncounterComplete = false;
    int32 RemainingEnemies = 0;

    UFUNCTION()
    void HandleTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void HandleMonsterDied(AActor* DeadMonster);

    void SpawnReward();
};
