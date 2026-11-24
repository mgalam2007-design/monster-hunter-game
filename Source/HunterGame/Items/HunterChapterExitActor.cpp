#include "HunterChapterExitActor.h"

#include "ArinValeCharacter.h"
#include "Campaign/HunterCampaignSubsystem.h"
#include "Characters/ArinProgressionComponent.h"
#include "Components/HunterCurrencyComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

AHunterChapterExitActor::AHunterChapterExitActor()
{
    PrimaryActorTick.bCanEverTick = false;

    ExitTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("ChapterExitTrigger"));
    SetRootComponent(ExitTrigger);
    ExitTrigger->InitSphereRadius(TriggerRadius);
    ExitTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ExitTrigger->SetCollisionObjectType(ECC_WorldDynamic);
    ExitTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    ExitTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    ExitTrigger->SetGenerateOverlapEvents(true);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> GateMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (GateMesh.Succeeded())
    {
        UStaticMesh* GateStaticMesh = GateMesh.Object;
        auto AddGatePart = [this, GateStaticMesh](const FName Name, const FVector& Location, const FVector& Scale)
        {
            UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
            Part->SetupAttachment(ExitTrigger);
            Part->SetStaticMesh(GateStaticMesh);
            Part->SetRelativeLocation(Location);
            Part->SetRelativeScale3D(Scale);
            Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Part->SetGenerateOverlapEvents(false);
            Part->SetCanEverAffectNavigation(false);
            ExitVisuals.Add(Part);
        };
        AddGatePart(TEXT("ExitGateLeftPillar"), FVector(0.0f, -135.0f, 150.0f), FVector(0.18f, 0.18f, 1.5f));
        AddGatePart(TEXT("ExitGateRightPillar"), FVector(0.0f, 135.0f, 150.0f), FVector(0.18f, 0.18f, 1.5f));
        AddGatePart(TEXT("ExitGateHeader"), FVector(0.0f, 0.0f, 300.0f), FVector(0.18f, 1.5f, 0.18f));
    }
}

void AHunterChapterExitActor::BeginPlay()
{
    Super::BeginPlay();

    if (ExitTrigger)
    {
        TriggerRadius = FMath::IsFinite(TriggerRadius) ? FMath::Max(50.0f, TriggerRadius) : 220.0f;
        ExitTrigger->SetSphereRadius(TriggerRadius);
        ExitTrigger->OnComponentBeginOverlap.AddDynamic(this, &AHunterChapterExitActor::HandleExitOverlap);
    }
}

void AHunterChapterExitActor::HandleExitOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (AArinValeCharacter* Arin = Cast<AArinValeCharacter>(OtherActor))
    {
        if (Arin->IsLocallyControlled())
        {
            TryCompleteChapter(Arin);
        }
    }
}

bool AHunterChapterExitActor::TryCompleteChapter(AArinValeCharacter* Arin)
{
    if (!Arin || SaveSlotName.IsEmpty() || (bOneTimeActivation && bHasActivated))
    {
        return false;
    }

    UGameInstance* GameInstance = GetGameInstance();
    UHunterCampaignSubsystem* Campaign = GameInstance
        ? GameInstance->GetSubsystem<UHunterCampaignSubsystem>()
        : nullptr;

    if (!Campaign)
    {
        BP_OnChapterExitResult(0, false, false);
        return false;
    }

    // If progression was committed but the disk save failed, retry only the save.
    // Never complete the newly unlocked chapter or grant the reward twice.
    if (!bCompletionCommitted)
    {
        CommittedChapterNumber = Campaign->GetCurrentChapterNumber();
        bCompletionCommitted = Campaign->CompleteCurrentChapter();
        if (!bCompletionCommitted)
        {
            BP_OnChapterExitResult(CommittedChapterNumber, false, false);
            return false;
        }

        if (CompletionExperienceReward > 0)
        {
            if (UArinProgressionComponent* Progression = Arin->GetProgressionComponent())
            {
                Progression->AddExperience(CompletionExperienceReward);
            }
        }

        if (CompletionCurrencyReward > 0)
        {
            if (UHunterCurrencyComponent* Currency = Arin->GetCurrencyComponent())
            {
                Currency->AddCurrency(CompletionCurrencyReward);
            }
        }
    }

    const bool bSaved = Campaign->SaveCampaign(Arin, SaveSlotName, UserIndex);
    if (bSaved)
    {
        bHasActivated = true;
    }

    BP_OnChapterExitResult(CommittedChapterNumber, true, bSaved);

    // Travel is opt-in: only map names explicitly configured on the exit are opened.
    // Keep the final chapter exit in place when no successor map is assigned.
    if (bSaved && !NextChapterMapName.IsNone() && GetWorld())
    {
        UGameplayStatics::OpenLevel(this, NextChapterMapName);
    }

    return bSaved;
}
