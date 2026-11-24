#include "HunterCheckpointActor.h"

#include "ArinValeCharacter.h"
#include "Campaign/HunterCampaignSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
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

    CheckpointVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CheckpointVisual"));
    CheckpointVisual->SetupAttachment(ActivationSphere);
    CheckpointVisual->SetRelativeLocation(FVector(0.0f, 0.0f, 45.0f));
    CheckpointVisual->SetRelativeScale3D(FVector(0.55f, 0.55f, 0.75f));
    CheckpointVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CheckpointVisual->SetGenerateOverlapEvents(false);
    CheckpointVisual->SetCanEverAffectNavigation(false);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CheckpointMesh(
        TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (CheckpointMesh.Succeeded())
    {
        CheckpointVisual->SetStaticMesh(CheckpointMesh.Object);
    }
}

void AHunterCheckpointActor::BeginPlay()
{
    Super::BeginPlay();

    if (ActivationSphere)
    {
        ActivationRadius = FMath::IsFinite(ActivationRadius) ? FMath::Max(50.0f, ActivationRadius) : 180.0f;
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
