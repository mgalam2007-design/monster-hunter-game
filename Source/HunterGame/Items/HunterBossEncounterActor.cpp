#include "HunterBossEncounterActor.h"

#include "ArinValeCharacter.h"
#include "Characters/HunterBossBase.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Quests/HunterQuestSubsystem.h"

AHunterBossEncounterActor::AHunterBossEncounterActor()
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

void AHunterBossEncounterActor::BeginPlay()
{
    Super::BeginPlay();

    if (ActivationSphere)
    {
        ActivationSphere->SetSphereRadius(ActivationRadius);
        ActivationSphere->OnComponentBeginOverlap.AddDynamic(
            this, &AHunterBossEncounterActor::HandleActivationOverlap);
    }
}

void AHunterBossEncounterActor::HandleActivationOverlap(
    UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (bStartAutomatically)
    {
        if (AArinValeCharacter* Arin = Cast<AArinValeCharacter>(OtherActor))
        {
            if (Arin->IsLocallyControlled())
            {
                StartBossEncounter(Arin);
            }
        }
    }
}

bool AHunterBossEncounterActor::StartBossEncounter(AArinValeCharacter* Arin)
{
    if (!Arin || !Arin->IsLocallyControlled() || bEncounterStarted || !BossClass || !GetWorld())
    {
        return false;
    }

    const FTransform SpawnTransform = BossSpawnOffset * GetActorTransform();
    SpawnedBoss = GetWorld()->SpawnActor<AHunterBossBase>(
        BossClass, SpawnTransform, FActorSpawnParameters());

    if (!SpawnedBoss)
    {
        BP_OnBossSpawnFailed();
        return false;
    }

    bEncounterStarted = true;
    SpawnedBoss->OnMonsterDied.AddDynamic(this, &AHunterBossEncounterActor::HandleBossDied);

    if (ActivationSphere)
    {
        ActivationSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    BP_OnBossEncounterStarted(SpawnedBoss);
    return true;
}

void AHunterBossEncounterActor::HandleBossDied(AActor* DeadBoss)
{
    if (!bEncounterStarted || bEncounterComplete || DeadBoss != SpawnedBoss)
    {
        return;
    }

    bEncounterComplete = true;
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UHunterQuestSubsystem* QuestSubsystem = GameInstance->GetSubsystem<UHunterQuestSubsystem>())
        {
            if (!BossDefeatQuestEventTag.IsNone())
            {
                QuestSubsystem->RegisterQuestEvent(BossDefeatQuestEventTag, 1);
            }
        }
    }

    if (SpawnedBoss)
    {
        SpawnedBoss->OnMonsterDied.RemoveDynamic(this, &AHunterBossEncounterActor::HandleBossDied);
    }

    BP_OnBossEncounterDefeated(SpawnedBoss);
}
