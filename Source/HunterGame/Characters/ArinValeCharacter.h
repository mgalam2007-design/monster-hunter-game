#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ArinValeCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
class UArinProgressionComponent;
class UHunterHealthComponent;
class UHunterWeaponTraceComponent;
class UHunterInventoryComponent;
class UHunterEquipmentComponent;
class UHunterAbilityComponent;

UCLASS()
class HUNTERGAME_API AArinValeCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AArinValeCharacter();

    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintPure, Category="Arin|Progression")
    UArinProgressionComponent* GetProgressionComponent() const { return ProgressionComponent; }

    UFUNCTION(BlueprintPure, Category="Arin|Health")
    UHunterHealthComponent* GetHealthComponent() const { return HealthComponent; }

    UFUNCTION(BlueprintPure, Category="Arin|Combat")
    UHunterWeaponTraceComponent* GetWeaponTraceComponent() const { return WeaponTraceComponent; }

    UFUNCTION(BlueprintPure, Category="Arin|Inventory")
    UHunterInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

    UFUNCTION(BlueprintPure, Category="Arin|Equipment")
    UHunterEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

    UFUNCTION(BlueprintPure, Category="Arin|Abilities")
    UHunterAbilityComponent* GetAbilityComponent() const { return AbilityComponent; }

    UFUNCTION(BlueprintCallable, Category="Arin|Combat")
    void BeginWeaponTrace();

    UFUNCTION(BlueprintCallable, Category="Arin|Combat")
    void EndWeaponTrace();

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void StartSprint();
    void StopSprint();
    void Dodge();
    void SwordAttack();
    UFUNCTION()
    void HandleEvolutionStageChanged(EArinEvolutionStage PreviousStage, EArinEvolutionStage NewStage);
    void ApplyEvolutionStats();
    void UseEnergyStrike();
    void UseShadowStep();
    void UseShadowEcho();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    TObjectPtr<UCameraComponent> FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Progression")
    TObjectPtr<UArinProgressionComponent> ProgressionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health")
    TObjectPtr<UHunterHealthComponent> HealthComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
    TObjectPtr<UHunterWeaponTraceComponent> WeaponTraceComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
    TObjectPtr<UHunterInventoryComponent> InventoryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Equipment")
    TObjectPtr<UHunterEquipmentComponent> EquipmentComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abilities")
    TObjectPtr<UHunterAbilityComponent> AbilityComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement")
    float SprintSpeed = 850.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement")
    float DodgeStrength = 900.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement")
    float DodgeUpwardStrength = 90.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
    TObjectPtr<UAnimMontage> SwordAttackMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
    float SwordAttackCooldown = 0.55f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
    float DefaultWeaponTraceWindow = 0.22f;

private:
    float DefaultWalkSpeed = 500.0f;
    float UnmodifiedWeaponDamage = 35.0f;
    float UnmodifiedMaxHealth = 100.0f;
    bool bCanDodge = true;
    bool bCanAttack = true;

    FTimerHandle DodgeCooldownHandle;
    FTimerHandle AttackCooldownHandle;
    FTimerHandle WeaponTraceHandle;

    void ResetDodge();
    void ResetAttack();
};
