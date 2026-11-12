#include "ArinValeCharacter.h"

#include "ArinProgressionComponent.h"
#include "Components/HunterHealthComponent.h"
#include "Components/HunterWeaponTraceComponent.h"
#include "Components/HunterInventoryComponent.h"
#include "Components/HunterEquipmentComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

AArinValeCharacter::AArinValeCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 620.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 620.0f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
    GetCharacterMovement()->BrakingDecelerationWalking = 1800.0f;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 340.0f;
    CameraBoom->SocketOffset = FVector(0.0f, 55.0f, 70.0f);
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 12.0f;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    ProgressionComponent = CreateDefaultSubobject<UArinProgressionComponent>(TEXT("ArinProgression"));
    HealthComponent = CreateDefaultSubobject<UHunterHealthComponent>(TEXT("ArinHealth"));
    WeaponTraceComponent = CreateDefaultSubobject<UHunterWeaponTraceComponent>(TEXT("WeaponTrace"));
    InventoryComponent = CreateDefaultSubobject<UHunterInventoryComponent>(TEXT("HunterInventory"));
    EquipmentComponent = CreateDefaultSubobject<UHunterEquipmentComponent>(TEXT("HunterEquipment"));
}

void AArinValeCharacter::BeginPlay()
{
    Super::BeginPlay();
    DefaultWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
    UnmodifiedWeaponDamage = WeaponTraceComponent ? WeaponTraceComponent->GetBaseDamage() : 35.0f;
}

float AArinValeCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    const float DamageReduction = EquipmentComponent ? EquipmentComponent->GetDamageReduction() : 0.0f;
    const float MitigatedDamage = DamageAmount * (1.0f - DamageReduction);
    const float AppliedDamage = Super::TakeDamage(MitigatedDamage, DamageEvent, EventInstigator, DamageCauser);
    if (HealthComponent)
    {
        HealthComponent->ApplyDamage(AppliedDamage);
    }
    return AppliedDamage;
}

void AArinValeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    check(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &AArinValeCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AArinValeCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &AArinValeCharacter::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &AArinValeCharacter::LookUp);
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AArinValeCharacter::StartSprint);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AArinValeCharacter::StopSprint);
    PlayerInputComponent->BindAction("Dodge", IE_Pressed, this, &AArinValeCharacter::Dodge);
    PlayerInputComponent->BindAction("SwordAttack", IE_Pressed, this, &AArinValeCharacter::SwordAttack);
}

void AArinValeCharacter::MoveForward(float Value)
{
    if (Controller && !FMath::IsNearlyZero(Value))
    {
        const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
        AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value);
    }
}

void AArinValeCharacter::MoveRight(float Value)
{
    if (Controller && !FMath::IsNearlyZero(Value))
    {
        const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
        AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value);
    }
}

void AArinValeCharacter::Turn(float Value) { AddControllerYawInput(Value); }
void AArinValeCharacter::LookUp(float Value) { AddControllerPitchInput(Value); }
void AArinValeCharacter::StartSprint() { GetCharacterMovement()->MaxWalkSpeed = SprintSpeed; }
void AArinValeCharacter::StopSprint() { GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed; }

void AArinValeCharacter::Dodge()
{
    if (!bCanDodge || (HealthComponent && HealthComponent->IsDead())) return;

    FVector DodgeDirection = GetLastMovementInputVector().GetSafeNormal();
    if (DodgeDirection.IsNearlyZero()) DodgeDirection = GetActorForwardVector();

    LaunchCharacter(DodgeDirection * DodgeStrength + FVector::UpVector * DodgeUpwardStrength, true, true);
    bCanDodge = false;
    GetWorldTimerManager().SetTimer(DodgeCooldownHandle, this, &AArinValeCharacter::ResetDodge, 0.8f, false);
}

void AArinValeCharacter::ResetDodge() { bCanDodge = true; }

void AArinValeCharacter::BeginWeaponTrace()
{
    if (WeaponTraceComponent) WeaponTraceComponent->BeginWeaponTrace();
}

void AArinValeCharacter::EndWeaponTrace()
{
    if (WeaponTraceComponent) WeaponTraceComponent->EndWeaponTrace();
}

void AArinValeCharacter::SwordAttack()
{
    if (!bCanAttack || (HealthComponent && HealthComponent->IsDead())) return;

    bCanAttack = false;
    if (SwordAttackMontage)
    {
        if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
        {
            AnimInstance->Montage_Play(SwordAttackMontage);
        }
    }

    if (WeaponTraceComponent)
    {
        const float BaseDamage = UnmodifiedWeaponDamage;
        const float GearBonus = EquipmentComponent ? EquipmentComponent->GetTotalDamageBonus() : 0.0f;
        const float StageMultiplier = ProgressionComponent ? ProgressionComponent->GetCurrentStageProfile().DamageMultiplier : 1.0f;
        WeaponTraceComponent->SetBaseDamage((BaseDamage + GearBonus) * StageMultiplier);
    }

    BeginWeaponTrace();
    GetWorldTimerManager().SetTimer(WeaponTraceHandle, this, &AArinValeCharacter::EndWeaponTrace, FMath::Max(0.05f, DefaultWeaponTraceWindow), false);
    GetWorldTimerManager().SetTimer(AttackCooldownHandle, this, &AArinValeCharacter::ResetAttack, SwordAttackCooldown, false);
}

void AArinValeCharacter::ResetAttack() { bCanAttack = true; }
