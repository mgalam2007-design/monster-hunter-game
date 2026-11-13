#include "ArinValeCharacter.h"

#include "ArinProgressionComponent.h"
#include "Campaign/HunterCampaignSubsystem.h"
#include "Engine/GameInstance.h"
#include "Components/HunterHealthComponent.h"
#include "Components/HunterWeaponTraceComponent.h"
#include "Components/HunterInventoryComponent.h"
#include "Components/HunterEquipmentComponent.h"
#include "Components/HunterAbilityComponent.h"
#include "Components/HunterCurrencyComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Animation/AnimInstance.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
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

    // Temporary engine-native blockout so Arin is visible before the final
    // rigged character asset is imported. Keep this recognizable silhouette
    // stable while armor and abilities evolve through later production passes.
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
        TEXT("/Engine/BasicShapes/Sphere.Sphere"));

    auto AddBlockoutPart = [this](const FName PartName, UStaticMesh* Mesh,
        const FVector& Location, const FVector& Scale, const FRotator& Rotation)
    {
        if (!Mesh)
        {
            return;
        }

        UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(PartName);
        Part->SetupAttachment(GetCapsuleComponent());
        Part->SetStaticMesh(Mesh);
        Part->SetRelativeLocation(Location);
        Part->SetRelativeRotation(Rotation);
        Part->SetRelativeScale3D(Scale);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Part->SetGenerateOverlapEvents(false);
        Part->SetCanEverAffectNavigation(false);
        Part->CastShadow = true;
    };

    UStaticMesh* Cube = CubeMesh.Succeeded() ? CubeMesh.Object : nullptr;
    UStaticMesh* Sphere = SphereMesh.Succeeded() ? SphereMesh.Object : nullptr;
    AddBlockoutPart(TEXT("ArinBlockoutTorso"), Cube, FVector(0, 0, 8), FVector(0.34f, 0.23f, 0.48f), FRotator::ZeroRotator);
    AddBlockoutPart(TEXT("ArinBlockoutShoulderL"), Cube, FVector(0, -27, 36), FVector(0.18f, 0.29f, 0.15f), FRotator::ZeroRotator);
    AddBlockoutPart(TEXT("ArinBlockoutShoulderR"), Cube, FVector(0, 27, 36), FVector(0.18f, 0.29f, 0.15f), FRotator::ZeroRotator);
    AddBlockoutPart(TEXT("ArinBlockoutArmL"), Cube, FVector(0, -32, -2), FVector(0.14f, 0.15f, 0.39f), FRotator::ZeroRotator);
    AddBlockoutPart(TEXT("ArinBlockoutArmR"), Cube, FVector(0, 32, -2), FVector(0.14f, 0.15f, 0.39f), FRotator::ZeroRotator);
    AddBlockoutPart(TEXT("ArinBlockoutLegL"), Cube, FVector(0, -13, -48), FVector(0.16f, 0.17f, 0.42f), FRotator::ZeroRotator);
    AddBlockoutPart(TEXT("ArinBlockoutLegR"), Cube, FVector(0, 13, -48), FVector(0.16f, 0.17f, 0.42f), FRotator::ZeroRotator);
    AddBlockoutPart(TEXT("ArinBlockoutBootL"), Cube, FVector(10, -13, -78), FVector(0.25f, 0.19f, 0.10f), FRotator::ZeroRotator);
    AddBlockoutPart(TEXT("ArinBlockoutBootR"), Cube, FVector(10, 13, -78), FVector(0.25f, 0.19f, 0.10f), FRotator::ZeroRotator);
    AddBlockoutPart(TEXT("ArinBlockoutHead"), Sphere, FVector(0, 0, 63), FVector(0.19f, 0.19f, 0.22f), FRotator::ZeroRotator);
    AddBlockoutPart(TEXT("ArinBlockoutHair"), Cube, FVector(-3, 0, 79), FVector(0.20f, 0.21f, 0.08f), FRotator::ZeroRotator);
    AddBlockoutPart(TEXT("ArinBlockoutSword"), Cube, FVector(17, 43, 8), FVector(0.07f, 0.07f, 0.48f), FRotator(0, 0, 8));

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
    AbilityComponent = CreateDefaultSubobject<UHunterAbilityComponent>(TEXT("HunterAbilities"));
    CurrencyComponent = CreateDefaultSubobject<UHunterCurrencyComponent>(TEXT("HunterCurrency"));
}

void AArinValeCharacter::BeginPlay()
{
    Super::BeginPlay();
    DefaultWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
    UnmodifiedMaxHealth = HealthComponent ? HealthComponent->GetMaxHealth() : 100.0f;
    if (ProgressionComponent)
    {
        ProgressionComponent->OnStageChanged.AddDynamic(this, &AArinValeCharacter::HandleEvolutionStageChanged);
    }
    if (HealthComponent)
    {
        HealthComponent->OnOwnerDied.AddDynamic(this, &AArinValeCharacter::HandleArinDeath);
    }
    ApplyEvolutionStats();
    if (IsLocallyControlled())
    {
        if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
        {
            if (HUDWidgetClass)
            {
                ActiveHUDWidget = CreateWidget<UUserWidget>(PlayerController, HUDWidgetClass);
                if (ActiveHUDWidget) ActiveHUDWidget->AddToViewport();
            }
            if (QuestTrackerWidgetClass)
            {
                ActiveQuestTrackerWidget = CreateWidget<UUserWidget>(PlayerController, QuestTrackerWidgetClass);
                if (ActiveQuestTrackerWidget) ActiveQuestTrackerWidget->AddToViewport();
            }
            if (InventoryWidgetClass)
            {
                ActiveInventoryWidget = CreateWidget<UUserWidget>(PlayerController, InventoryWidgetClass);
                if (ActiveInventoryWidget)
                {
                    ActiveInventoryWidget->AddToViewport();
                    ActiveInventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
                }
            }
        }
    }
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
    PlayerInputComponent->BindAction("AbilityEnergyStrike", IE_Pressed, this, &AArinValeCharacter::UseEnergyStrike);
    PlayerInputComponent->BindAction("AbilityShadowStep", IE_Pressed, this, &AArinValeCharacter::UseShadowStep);
    PlayerInputComponent->BindAction("AbilityShadowEcho", IE_Pressed, this, &AArinValeCharacter::UseShadowEcho);
    PlayerInputComponent->BindAction("UseConsumable", IE_Pressed, this, &AArinValeCharacter::UseConsumable);
    PlayerInputComponent->BindAction("InventoryMenu", IE_Pressed, this, &AArinValeCharacter::ToggleInventoryMenu);
}

void AArinValeCharacter::MoveForward(float Value)
{
    if ((!HealthComponent || !HealthComponent->IsDead()) && Controller && !FMath::IsNearlyZero(Value))
    {
        const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
        AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value);
    }
}

void AArinValeCharacter::MoveRight(float Value)
{
    if ((!HealthComponent || !HealthComponent->IsDead()) && Controller && !FMath::IsNearlyZero(Value))
    {
        const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
        AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value);
    }
}

void AArinValeCharacter::Turn(float Value)
{
    if (!HealthComponent || !HealthComponent->IsDead()) AddControllerYawInput(Value);
}

void AArinValeCharacter::LookUp(float Value)
{
    if (!HealthComponent || !HealthComponent->IsDead()) AddControllerPitchInput(Value);
}
void AArinValeCharacter::StartSprint()
{
    if (HealthComponent && HealthComponent->IsDead()) return;
    const float MovementMultiplier = ProgressionComponent ? ProgressionComponent->GetCurrentStageProfile().MovementMultiplier : 1.0f;
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed * FMath::Max(0.1f, MovementMultiplier);
}

void AArinValeCharacter::StopSprint()
{
    const float MovementMultiplier = ProgressionComponent ? ProgressionComponent->GetCurrentStageProfile().MovementMultiplier : 1.0f;
    GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed * FMath::Max(0.1f, MovementMultiplier);
}

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

void AArinValeCharacter::UseEnergyStrike()
{
    if (AbilityComponent && (!HealthComponent || !HealthComponent->IsDead()))
    {
        AbilityComponent->TryActivateAbility(EHunterAbility::EnergyStrike);
    }
}

void AArinValeCharacter::UseShadowStep()
{
    if (AbilityComponent && (!HealthComponent || !HealthComponent->IsDead()))
    {
        AbilityComponent->TryActivateAbility(EHunterAbility::ShadowStep);
    }
}

void AArinValeCharacter::UseShadowEcho()
{
    if (AbilityComponent && (!HealthComponent || !HealthComponent->IsDead()))
    {
        AbilityComponent->TryActivateAbility(EHunterAbility::ShadowEcho);
    }
}

void AArinValeCharacter::HandleEvolutionStageChanged(EArinEvolutionStage PreviousStage, EArinEvolutionStage NewStage)
{
    ApplyEvolutionStats();
}

void AArinValeCharacter::ApplyEvolutionStats()
{
    const FArinStageProfile Profile = ProgressionComponent
        ? ProgressionComponent->GetCurrentStageProfile()
        : FArinStageProfile();

    if (HealthComponent)
    {
        HealthComponent->SetMaxHealth(UnmodifiedMaxHealth * FMath::Max(0.1f, Profile.HealthMultiplier), true);
    }

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed * FMath::Max(0.1f, Profile.MovementMultiplier);
    }
}

void AArinValeCharacter::UseConsumable()
{
    if (!InventoryComponent || (HealthComponent && HealthComponent->IsDead()))
    {
        return;
    }

    const TArray<FHunterInventoryItem> Items = InventoryComponent->GetItems();
    for (const FHunterInventoryItem& Item : Items)
    {
        if (Item.ItemType != EHunterItemType::Consumable || Item.Quantity <= 0)
        {
            continue;
        }

        const bool bCanHeal = HealthComponent && Item.HealthRestoreAmount > 0.0f
            && HealthComponent->GetCurrentHealth() < HealthComponent->GetMaxHealth();
        const bool bCanRestoreEnergy = AbilityComponent && Item.EnergyRestoreAmount > 0.0f
            && AbilityComponent->GetCurrentEnergy() < AbilityComponent->GetMaxEnergy();

        if (!bCanHeal && !bCanRestoreEnergy)
        {
            continue;
        }

        // Consume the stack first. Inventory listeners may trigger other
        // gameplay mutations, so never grant the effect if removal fails.
        if (!InventoryComponent->RemoveItem(Item.ItemId, 1))
        {
            return;
        }

        if (bCanHeal) HealthComponent->Heal(Item.HealthRestoreAmount);
        if (bCanRestoreEnergy) AbilityComponent->RestoreEnergy(Item.EnergyRestoreAmount);
        return;
    }
}

void AArinValeCharacter::ToggleInventoryMenu()
{
    if ((HealthComponent && HealthComponent->IsDead()) || bRespawnPending)
    {
        return;
    }

    if (!ActiveInventoryWidget)
    {
        return;
    }

    const bool bShouldOpen = ActiveInventoryWidget->GetVisibility() != ESlateVisibility::Visible;
    ActiveInventoryWidget->SetVisibility(bShouldOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        PlayerController->bShowMouseCursor = bShouldOpen;
    }
}


void AArinValeCharacter::HandleArinDeath()
{
    if (!IsLocallyControlled() || !GetWorld() || bRespawnPending)
    {
        return;
    }

    bRespawnPending = true;
    GetWorldTimerManager().ClearTimer(AttackCooldownHandle);
    GetWorldTimerManager().ClearTimer(DodgeCooldownHandle);
    GetWorldTimerManager().ClearTimer(WeaponTraceHandle);
    EndWeaponTrace();
    bCanAttack = false;
    bCanDodge = false;
    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        if (ActiveInventoryWidget) ActiveInventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
        PlayerController->bShowMouseCursor = false;
        FInputModeGameOnly GameOnlyInput;
        PlayerController->SetInputMode(GameOnlyInput);
    }
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();
    }
    GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
    GetWorldTimerManager().SetTimer(
        RespawnTimerHandle,
        this,
        &AArinValeCharacter::RespawnAtLastCheckpoint,
        FMath::Max(0.0f, RespawnDelay),
        false);
}

void AArinValeCharacter::RespawnAtLastCheckpoint()
{
    if (!IsLocallyControlled() || !HealthComponent || !bRespawnPending)
    {
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
    UHunterCampaignSubsystem* Campaign = GameInstance
        ? GameInstance->GetSubsystem<UHunterCampaignSubsystem>()
        : nullptr;

    const bool bLoadedCheckpoint = Campaign
        && Campaign->LoadCampaign(this, CampaignSaveSlot, CampaignSaveUserIndex);

    if (!bLoadedCheckpoint && GetCharacterMovement())
    {
        // No checkpoint save exists yet: revive at the current location.
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    }

    HealthComponent->ReviveFullHealth();
    ApplyEvolutionStats();
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    }
    bCanAttack = true;
    bCanDodge = true;
    bRespawnPending = false;
}
