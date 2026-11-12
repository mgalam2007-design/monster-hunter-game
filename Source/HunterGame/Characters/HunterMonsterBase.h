#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/HunterInventoryComponent.h"
#include "HunterMonsterBase.generated.h"

class AHunterPickupActor;

USTRUCT(BlueprintType)
struct FHunterMonsterLootDrop
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loot")
    FHunterInventoryItem Item;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loot", meta=(ClampMin="0.0", ClampMax="1.0"))
    float DropChance = 1.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHunterMonsterDied, AActor*, DeadMonster);

/**
 * Reusable enemy foundation. Configure stats and visuals in a Blueprint child;
 * place a NavMeshBoundsVolume in the level for navigation-based pursuit.
 */
UCLASS()
class HUNTERGAME_API AHunterMonsterBase : public ACharacter
{
    GENERATED_BODY()

public:
    AHunterMonsterBase();

    virtual void Tick(float DeltaSeconds) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintCallable, Category="Monster|Combat")
    void ReceiveHunterDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintPure, Category="Monster|Combat")
    float GetHealthPercent() const;

    UPROPERTY(BlueprintAssignable, Category="Monster|Combat")
    FHunterMonsterDied OnMonsterDied;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Stats")
    float MaxHealth = 120.0f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Monster|Stats")
    float CurrentHealth = 120.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|AI")
    float DetectionRadius = 1400.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|AI")
    float AttackRange = 180.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Combat")
    float AttackDamage = 12.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Combat")
    float AttackCooldown = 1.6f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Rewards", meta=(ClampMin="0"))
    int32 ExperienceReward = 25;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Rewards", meta=(ClampMin="0"))
    int64 CurrencyReward = 5;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Rewards")
    FName DefeatQuestEventTag = TEXT("MonsterDefeated");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Rewards")
    TSubclassOf<AHunterPickupActor> PickupActorClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Rewards")
    TArray<FHunterMonsterLootDrop> LootDrops;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Movement")
    float ChaseSpeed = 430.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|AI")
    bool bCanChasePlayer = true;

    UFUNCTION(BlueprintImplementableEvent, Category="Monster|Combat")
    void BP_OnMonsterAttack(AActor* Target);

    UFUNCTION(BlueprintImplementableEvent, Category="Monster|Combat")
    void BP_OnMonsterDeath();

private:
    TWeakObjectPtr<ACharacter> TargetCharacter;
    float AttackTimer = 0.0f;
    bool bIsDead = false;

    void FindTarget();
    void UpdateCombat(float DeltaSeconds);
    void Die();
    void SpawnLootDrops();
};
