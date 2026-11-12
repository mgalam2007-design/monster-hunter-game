#include "HunterQuestTriggerActor.h"

#include "ArinValeCharacter.h"
#include "Components/BoxComponent.h"
#include "Quests/HunterQuestSubsystem.h"
#include "Engine/GameInstance.h"

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
}

void AHunterQuestTriggerActor::BeginPlay()
{
    Super::BeginPlay();
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AHunterQuestTriggerActor::HandleOverlap);
}

void AHunterQuestTriggerActor::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (bHasTriggered || QuestEventTag.IsNone() || EventAmount < 1 || !Cast<AArinValeCharacter>(OtherActor))
    {
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
    UHunterQuestSubsystem* Quest = GameInstance ? GameInstance->GetSubsystem<UHunterQuestSubsystem>() : nullptr;
    if (!Quest || !Quest->RegisterQuestEvent(QuestEventTag, EventAmount))
    {
        return;
    }

    BP_OnQuestEventTriggered(OtherActor);

    if (bDestroyAfterTrigger)
    {
        bHasTriggered = true;
        Destroy();
    }
    else if (bTriggerOnce)
    {
        bHasTriggered = true;
        SetActorEnableCollision(false);
    }
}
