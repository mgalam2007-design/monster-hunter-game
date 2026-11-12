#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ArinValeCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
class UArinProgressionComponent;
class UHunterHealthComponent;

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    TObjectPtr<UCameraComponent> FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Progression")
    TObjectPtr<UArinProgressionComponent> ProgressionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health")
    TObjectPtr<UHunterHealthComponent> HealthComponent;

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

private:
    float DefaultWalkSpeed = 500.0f;
    bool bCanDodge = true;
    bool bCanAttack = true;

    FTimerHandle DodgeCooldownHandle;
    FTimerHandle AttackCooldownHandle;

    void ResetDodge();
    void ResetAttack();
};
