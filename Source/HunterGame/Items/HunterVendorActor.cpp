#include "HunterVendorActor.h"

#include "ArinValeCharacter.h"
#include "HunterShopWidget.h"
#include "Components/HunterShopComponent.h"
#include "Components/SphereComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerInput.h"
#include "InputCoreTypes.h"

AHunterVendorActor::AHunterVendorActor()
{
    PrimaryActorTick.bCanEverTick = false;

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    SetRootComponent(InteractionSphere);
    InteractionSphere->InitSphereRadius(InteractionRadius);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    InteractionSphere->SetGenerateOverlapEvents(true);

    ShopComponent = CreateDefaultSubobject<UHunterShopComponent>(TEXT("VendorShop"));
}

void AHunterVendorActor::BeginPlay()
{
    Super::BeginPlay();

    if (InteractionSphere)
    {
        InteractionSphere->SetSphereRadius(InteractionRadius);
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AHunterVendorActor::HandleBeginOverlap);
        InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AHunterVendorActor::HandleEndOverlap);
    }
}

void AHunterVendorActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CloseVendor();

    if (bInputBound)
    {
        DisableInput(NearbyController.Get());
        bInputBound = false;
    }

    Super::EndPlay(EndPlayReason);
}

void AHunterVendorActor::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepResult)
{
    AArinValeCharacter* Buyer = Cast<AArinValeCharacter>(OtherActor);
    if (!Buyer || !Buyer->IsLocallyControlled() || ActiveShopWidget)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(Buyer->GetController());
    if (!PC)
    {
        return;
    }

    NearbyBuyer = Buyer;
    NearbyController = PC;
    EnableInput(PC);
    if (InputComponent)
    {
        InputComponent->BindKey(InteractKey, IE_Pressed, this, &AHunterVendorActor::HandleInteractPressed);
        InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AHunterVendorActor::HandleClosePressed);
        bInputBound = true;
    }
    BP_OnVendorNearby(true);
}

void AHunterVendorActor::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor != NearbyBuyer.Get())
    {
        return;
    }

    CloseVendor();
    BP_OnVendorNearby(false);

    if (bInputBound)
    {
        DisableInput(NearbyController.Get());
        bInputBound = false;
    }

    NearbyBuyer.Reset();
    NearbyController.Reset();
}

void AHunterVendorActor::HandleInteractPressed()
{
    if (NearbyBuyer.IsValid())
    {
        OpenVendor(NearbyBuyer.Get());
    }
}

void AHunterVendorActor::HandleClosePressed()
{
    CloseVendor();
}

void AHunterVendorActor::OpenVendor(AArinValeCharacter* Buyer)
{
    if (!Buyer || !ShopWidgetClass || !ShopComponent || ActiveShopWidget)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(Buyer->GetController());
    if (!PC || !PC->IsLocalController())
    {
        return;
    }

    ActiveShopWidget = CreateWidget<UHunterShopWidget>(PC, ShopWidgetClass);
    if (!ActiveShopWidget)
    {
        return;
    }

    ActiveShopWidget->InitializeShop(ShopComponent, Buyer);
    ActiveShopWidget->AddToViewport(20);
    PC->bShowMouseCursor = true;

    FInputModeGameAndUI InputMode;
    InputMode.SetWidgetToFocus(ActiveShopWidget->TakeWidget());
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    PC->SetInputMode(InputMode);

}

void AHunterVendorActor::CloseVendor()
{
    if (!ActiveShopWidget)
    {
        return;
    }

    ActiveShopWidget->RemoveFromParent();
    ActiveShopWidget = nullptr;

    if (APlayerController* PC = NearbyController.Get())
    {
        PC->bShowMouseCursor = false;
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
    }
}

