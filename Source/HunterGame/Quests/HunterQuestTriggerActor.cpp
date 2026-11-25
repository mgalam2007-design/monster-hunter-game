#include "HunterQuestTriggerActor.h"

#include "ArinValeCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "Quests/HunterQuestSubsystem.h"
#include "Campaign/HunterCampaignSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"

AHunterQuestTriggerActor::AHunterQuestTriggerActor()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("QuestTriggerVolume"));
    SetRootComponent(TriggerVolume);
    TriggerVolume->SetBoxExtent(FVector(180.0f, 180.0f, 120.0f));
    TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerVolume->SetCollisionObjectType(ECC_WorldDynamic);
    TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    TriggerVolume->SetGenerateOverlapEvents(true);

    InteractionPrompt = CreateDefaultSubobject<UTextRenderComponent>(TEXT("InteractionPrompt"));
    InteractionPrompt->SetupAttachment(RootComponent);
    InteractionPrompt->SetRelativeLocation(FVector(0.0f, 0.0f, 145.0f));
    InteractionPrompt->SetHorizontalAlignment(EHTA_Center);
    InteractionPrompt->SetVerticalAlignment(EVRTA_TextCenter);
    InteractionPrompt->SetWorldSize(42.0f);
    InteractionPrompt->SetTextRenderColor(FColor(255, 225, 145));
    InteractionPrompt->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    InteractionPrompt->SetHiddenInGame(true);
}

void AHunterQuestTriggerActor::BeginPlay()
{
    Super::BeginPlay();
    if (InteractionPrompt)
    {
        InteractionPrompt->SetText(InteractionPromptText);
        InteractionPrompt->SetVisibility(false);
    }
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AHunterQuestTriggerActor::HandleOverlap);
    TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AHunterQuestTriggerActor::HandleEndOverlap);
}

void AHunterQuestTriggerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearInteractionInput();
    Super::EndPlay(EndPlayReason);
}

void AHunterQuestTriggerActor::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepResult)
{
    AArinValeCharacter* Arin = Cast<AArinValeCharacter>(OtherActor);
    if (bHasTriggered || QuestEventTag.IsNone() || EventAmount < 1 || !Arin ||
        !Arin->IsLocallyControlled())
    {
        return;
    }

    if (!bRequiresInteraction)
    {
        TryRegisterQuestEvent(Arin);
        return;
    }

    APlayerController* PC = Cast<APlayerController>(Arin->GetController());
    if (!PC)
    {
        return;
    }

    NearbyArin = Arin;
    NearbyController = PC;
    if (InteractionPrompt)
    {
        InteractionPrompt->SetVisibility(true);
    }
    EnableInput(PC);
    if (InputComponent)
    {
        InputComponent->BindKey(InteractKey, IE_Pressed, this,
            &AHunterQuestTriggerActor::HandleInteractPressed);
        bInputBound = true;
        BP_OnInteractionAvailable(true);
    }
}

void AHunterQuestTriggerActor::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (OtherActor != NearbyArin.Get())
    {
        return;
    }

    BP_OnInteractionAvailable(false);
    if (InteractionPrompt)
    {
        InteractionPrompt->SetVisibility(false);
    }
    ClearInteractionInput();
}

void AHunterQuestTriggerActor::HandleInteractPressed()
{
    AArinValeCharacter* Arin = NearbyArin.Get();
    if (Arin && TriggerVolume && TriggerVolume->IsOverlappingActor(Arin))
    {
        TryRegisterQuestEvent(Arin);
    }
}

bool AHunterQuestTriggerActor::TryRegisterQuestEvent(AArinValeCharacter* Arin)
{
    if (bHasTriggered || !Arin || QuestEventTag.IsNone() || EventAmount < 1)
    {
        return false;
    }

    UGameInstance* GameInstance = GetGameInstance();
    UHunterQuestSubsystem* Quest = GameInstance
        ? GameInstance->GetSubsystem<UHunterQuestSubsystem>()
        : nullptr;
    if (!Quest || !Quest->RegisterQuestEvent(QuestEventTag, EventAmount))
    {
        return false;
    }

    if (!StoryChoiceValue.IsNone())
    {
        UHunterCampaignSubsystem* Campaign = GameInstance
            ? GameInstance->GetSubsystem<UHunterCampaignSubsystem>()
            : nullptr;
        if (!Campaign || !Campaign->SetChapter12StoryChoice(StoryChoiceValue))
        {
            UE_LOG(LogTemp, Warning, TEXT("HUNTER: Story choice %s was not accepted."), *StoryChoiceValue.ToString());
            return false;
        }
    }

    bHasTriggered = true;
    BP_OnQuestEventTriggered(Arin);
    BP_OnInteractionAvailable(false);
    if (InteractionPrompt)
    {
        if (bRequiresInteraction && !bDestroyAfterTrigger)
        {
            InteractionPrompt->SetText(CompletionPromptText);
            InteractionPrompt->SetVisibility(true);
        }
        else
        {
            InteractionPrompt->SetVisibility(false);
        }
    }

    if (bDestroyAfterTrigger)
    {
        Destroy();
    }
    else if (bTriggerOnce)
    {
        SetActorEnableCollision(false);
        ClearInteractionInput();
    }
    else
    {
        bHasTriggered = false;
    }
    return true;
}

void AHunterQuestTriggerActor::ClearInteractionInput()
{
    if (bInputBound)
    {
        DisableInput(NearbyController.Get());
        bInputBound = false;
    }
    NearbyArin.Reset();
    NearbyController.Reset();
}
