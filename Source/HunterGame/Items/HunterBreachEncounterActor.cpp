#include "HunterBreachEncounterActor.h"

#include "ArinValeCharacter.h"
#include "Characters/HunterMonsterBase.h"
#include "Items/HunterPickupActor.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AHunterBreachEncounterActor::AHunterBreachEncounterActor()
{
    PrimaryActorTick.bCanEverTick = false;

    StartTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("EncounterTrigger"));
    SetRootComponent(StartTrigger);
    StartTrigger->InitSphereRadius(TriggerRadius);
    StartTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    StartTrigger->SetCollisionObjectType(ECC_WorldDynamic);
    StartTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    StartTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    StartTrigger->SetGenerateOverlapEvents(true);
}

void AHunterBreachEncounterActor::BeginPlay()
{
    Super::BeginPlay();

    if (StartTrigger)
    {
        StartTrigger->SetSphereRadius(TriggerRadius);
        StartTrigger->OnComponentBeginOverlap.AddDynamic(this, &AHunterBreachEncounterActor::HandleTriggerOverlap);
    }
}

void AHunterBreachEncounterActor::HandleTriggerOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (AArinValeCharacter* Arin = Cast<AArinValeCharacter>(OtherActor))
    {
        if (Arin->IsLocallyControlled())
        {
            StartEncounter(Arin);
        }
    }
}

void AHunterBreachEncounterActor::StartEncounter(AArinValeCharacter* Arin)
{
    if (bEncounterStarted || !Arin || !MonsterClass || !GetWorld())
    {
        return;
    }

    bEncounterStarted = true;
    const int32 DesiredCount = MonsterSpawnPoints.IsEmpty()
        ? FMath::Max(1, FallbackMonsterCount)
        : MonsterSpawnPoints.Num();

    PendingSpawnTransforms.Reserve(DesiredCount);
    for (int32 Index = 0; Index < DesiredCount; ++Index)
    {
        const FTransform SpawnTransform = MonsterSpawnPoints.IsValidIndex(Index)
            ? MonsterSpawnPoints[Index]
            : FTransform(Arin->GetActorRotation(),
                Arin->GetActorLocation() + Arin->GetActorForwardVector() * (450.0f + Index * 180.0f)
                    + Arin->GetActorRightVector() * ((Index % 2 == 0) ? 220.0f : -220.0f));
        PendingSpawnTransforms.Add(SpawnTransform);
    }

    BP_OnEncounterStarted(DesiredCount);
    SpawnNextMonster();
    if (NextSpawnIndex < PendingSpawnTransforms.Num())
    {
        GetWorldTimerManager().SetTimer(SpawnTimerHandle, this,
            &AHunterBreachEncounterActor::SpawnNextMonster,
            FMath::Max(0.1f, SpawnIntervalSeconds), true);
    }
    else
    {
        FinishSpawningWave();
    }
}

void AHunterBreachEncounterActor::SpawnNextMonster()
{
    if (!MonsterClass || !GetWorld() || !PendingSpawnTransforms.IsValidIndex(NextSpawnIndex))
    {
        GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
        FinishSpawningWave();
        return;
    }

    const FTransform SpawnTransform = PendingSpawnTransforms[NextSpawnIndex++];
    AHunterMonsterBase* Monster = GetWorld()->SpawnActorDeferred<AHunterMonsterBase>(
        MonsterClass, SpawnTransform, this, nullptr,
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

    if (Monster)
    {
        Monster->OnMonsterDied.AddDynamic(this, &AHunterBreachEncounterActor::HandleMonsterDied);
        SpawnedMonsters.Add(Monster);
        ++RemainingEnemies;
        UGameplayStatics::FinishSpawningActor(Monster, SpawnTransform);
    }

    if (NextSpawnIndex >= PendingSpawnTransforms.Num())
    {
        GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
        FinishSpawningWave();
    }
}

void AHunterBreachEncounterActor::FinishSpawningWave()
{
    if (NextSpawnIndex >= PendingSpawnTransforms.Num() && RemainingEnemies <= 0)
    {
        SpawnReward();
    }
}

void AHunterBreachEncounterActor::HandleMonsterDied(AActor* DeadMonster)
{
    if (bEncounterComplete || RemainingEnemies <= 0)
    {
        return;
    }

    AHunterMonsterBase* Monster = Cast<AHunterMonsterBase>(DeadMonster);
    if (!Monster || SpawnedMonsters.RemoveSingle(Monster) == 0)
    {
        return;
    }

    Monster->OnMonsterDied.RemoveDynamic(this, &AHunterBreachEncounterActor::HandleMonsterDied);
    RemainingEnemies = FMath::Max(0, RemainingEnemies - 1);
    if (RemainingEnemies == 0)
    {
        SpawnReward();
    }
}

void AHunterBreachEncounterActor::SpawnReward()
{
    if (bEncounterComplete)
    {
        return;
    }

    bEncounterComplete = true;
    if (RewardPickupClass && !RewardItem.ItemId.IsNone() && GetWorld())
    {
        const FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 45.0f);
        const FTransform SpawnTransform(GetActorRotation(), SpawnLocation);
        AHunterPickupActor* Pickup = GetWorld()->SpawnActorDeferred<AHunterPickupActor>(
            RewardPickupClass, SpawnTransform, this, nullptr,
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

        if (Pickup)
        {
            Pickup->SetPickupItem(RewardItem);
            Pickup->SetQuestEventTag(RewardQuestEventTag);
            UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
            BP_OnRewardSpawned(Pickup);
        }
    }

    BP_OnEncounterCompleted();
}
