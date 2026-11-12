#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HunterBossEncounterActor.generated.h"

class USphereComponent;
class UPrimitiveComponent;
class AArinValeCharacter;
class AHunterBossBase;

/** Starts a configurable boss fight when Arin enters its trigger volume. */
UCLASS()
class HUNTERGAME_API AHunterBossEncounterActor : public AActor
{
    GENERATED_BODY()

public:
    AHunterBossEncounterActor();

    UFUNCTION(BlueprintCallable, Category="Boss Encounter")
    bool StartBossEncounter(AArinValeCharacter* Arin);

    UFUNCTION(BlueprintPure, Category="Boss Encounter")
    bool HasEncounterStarted() const { return bEncounterStarted; }

    UFUNCTION(BlueprintPure, Category="Boss Encounter")
    bool IsEncounterComplete() const { return bEncounterComplete; }

    UFUNCTION(BlueprintPure, Category="Boss Encounter")
    AHunterBossBase* GetSpawnedBoss() const { return SpawnedBoss; }

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Boss Encounter")
    TObjectPtr<USphereComponent> ActivationSphere;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Boss Encounter|Spawn")
    TSubclassOf<AHunterBossBase> BossClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Boss Encounter|Spawn")
    FTransform BossSpawnOffset = FTransform(FRotator::ZeroRotator, FVector(650.0f, 0.0f, 0.0f));

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Boss Encounter|Trigger", meta=(ClampMin="100.0"))
    float ActivationRadius = 450.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Boss Encounter|Trigger")
    bool bStartAutomatically = true;

    UFUNCTION(BlueprintImplementableEvent, Category="Boss Encounter")
    void BP_OnBossEncounterStarted(AHunterBossBase* Boss);

    UFUNCTION(BlueprintImplementableEvent, Category="Boss Encounter")
    void BP_OnBossEncounterDefeated(AHunterBossBase* Boss);

    UFUNCTION(BlueprintImplementableEvent, Category="Boss Encounter")
    void BP_OnBossSpawnFailed();

    UFUNCTION()
    void HandleActivationOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void HandleBossDied(AActor* DeadBoss);

private:
    UPROPERTY(Transient)
    TObjectPtr<AHunterBossBase> SpawnedBoss;

    bool bEncounterStarted = false;
    bool bEncounterComplete = false;
};
