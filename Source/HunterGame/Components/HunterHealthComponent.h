#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HunterHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHunterHealthChanged, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHunterOwnerDied);

/** Reusable health, healing and death-state component for hunters and bosses. */
UCLASS(ClassGroup=(Hunter), meta=(BlueprintSpawnableComponent))
class HUNTERGAME_API UHunterHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHunterHealthComponent();

    UFUNCTION(BlueprintCallable, Category="Health")
    float ApplyDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category="Health")
    float Heal(float Amount);

    UFUNCTION(BlueprintCallable, Category="Health")
    void RestoreFullHealth();

    UFUNCTION(BlueprintCallable, Category="Health")
    void ReviveFullHealth();

    UFUNCTION(BlueprintCallable, Category="Health")
    void SetMaxHealth(float NewMaxHealth, bool bPreserveHealthPercentage = true);

    UFUNCTION(BlueprintPure, Category="Health")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category="Health")
    bool IsDead() const { return bIsDead; }

    UFUNCTION(BlueprintPure, Category="Health")
    float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintPure, Category="Health")
    float GetMaxHealth() const { return MaxHealth; }

    UPROPERTY(BlueprintAssignable, Category="Health")
    FHunterHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category="Health")
    FHunterOwnerDied OnOwnerDied;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health", meta=(ClampMin="1.0"))
    float MaxHealth = 100.0f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Health")
    float CurrentHealth = 100.0f;

private:
    bool bIsDead = false;
};
