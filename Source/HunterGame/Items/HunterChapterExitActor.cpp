#include "HunterChapterExitActor.h"

#include "ArinValeCharacter.h"
#include "Campaign/HunterCampaignSubsystem.h"
#include "Characters/ArinProgressionComponent.h"
#include "Components/HunterCurrencyComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/GameInstance.h"

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
}

void AHunterChapterExitActor::BeginPlay()
{
    Super::BeginPlay();

    if (ExitTrigger)
    {
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
    if (!Arin || (bOneTimeActivation && bHasActivated))
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
    return bSaved;
}
