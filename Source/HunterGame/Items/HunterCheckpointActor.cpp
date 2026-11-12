#include "HunterCheckpointActor.h"

#include "ArinValeCharacter.h"
#include "Campaign/HunterCampaignSubsystem.h"
#include "Components/SphereComponent.h"
#include "Engine/GameInstance.h"

AHunterCheckpointActor::AHunterCheckpointActor()
{
    PrimaryActorTick.bCanEverTick = false;

    ActivationSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ActivationSphere"));
    SetRootComponent(ActivationSphere);
    ActivationSphere->InitSphereRadius(ActivationRadius);
    ActivationSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ActivationSphere->SetCollisionObjectType(ECC_WorldDynamic);
    ActivationSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    ActivationSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    ActivationSphere->SetGenerateOverlapEvents(true);
}

void AHunterCheckpointActor::BeginPlay()
{
    Super::BeginPlay();

    if (ActivationSphere)
    {
        ActivationSphere->SetSphereRadius(ActivationRadius);
        ActivationSphere->OnComponentBeginOverlap.AddDynamic(this, &AHunterCheckpointActor::HandleActivationOverlap);
    }
}

void AHunterCheckpointActor::HandleActivationOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (AArinValeCharacter* Arin = Cast<AArinValeCharacter>(OtherActor))
    {
        if (Arin->IsLocallyControlled())
        {
            ActivateCheckpoint(Arin);
        }
    }
}

bool AHunterCheckpointActor::ActivateCheckpoint(AArinValeCharacter* Arin)
{
    if (!Arin || SaveSlotName.IsEmpty() || (bOneTimeActivation && bHasBeenActivated))
    {
        return false;
    }

    UGameInstance* GameInstance = GetGameInstance();
    UHunterCampaignSubsystem* Campaign = GameInstance
        ? GameInstance->GetSubsystem<UHunterCampaignSubsystem>()
        : nullptr;

    const bool bSaved = Campaign && Campaign->SaveCampaign(Arin, SaveSlotName, UserIndex);
    if (bSaved)
    {
        bHasBeenActivated = true;
    }

    BP_OnCheckpointActivated(CheckpointId, bSaved);
    return bSaved;
}
