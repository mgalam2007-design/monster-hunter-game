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

    // Normalize editor-authored values before they reach collision or timers.
    TriggerRadius = FMath::IsFinite(TriggerRadius)
        ? FMath::Clamp(TriggerRadius, 50.0f, 5000.0f)
        : 350.0f;
    SpawnIntervalSeconds = FMath::IsFinite(SpawnIntervalSeconds)
        ? FMath::Clamp(SpawnIntervalSeconds, 0.1f, 30.0f)
        : 1.25f;
    FallbackMonsterCount = FMath::Clamp(FallbackMonsterCount, 1, 20);

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
    if (bEncounterStarted || !Arin || !MonsterClass || !RewardPickupClass ||
        RewardItem.ItemId.IsNone() || RewardQuestEventTag.IsNone() || !GetWorld())
    {
        return;
    }

    bEncounterStarted = true;
    const int32 DesiredCount = MonsterSpawnPoints.IsEmpty()
        ? FallbackMonsterCount
        : FMath::Clamp(MonsterSpawnPoints.Num(), 1, 20);

    PendingSpawnTransforms.Reserve(DesiredCount);
    for (int32 Index = 0; Index < DesiredCount; ++Index)
    {
        const bool bHasValidAuthoredTransform = MonsterSpawnPoints.IsValidIndex(Index)
            && MonsterSpawnPoints[Index].IsValid();
        const FTransform FallbackTransform(Arin->GetActorRotation(),
            Arin->GetActorLocation() + Arin->GetActorForwardVector() * (450.0f + Index * 180.0f)
                + Arin->GetActorRightVector() * ((Index % 2 == 0) ? 220.0f : -220.0f));
        const FTransform SpawnTransform = bHasValidAuthoredTransform
            ? MonsterSpawnPoints[Index]
            : FallbackTransform;
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
        // Finish construction before counting the actor as a live wave member.
        // A Blueprint BeginPlay failure may destroy it during FinishSpawningActor.
        UGameplayStatics::FinishSpawningActor(Monster, SpawnTransform);
        if (IsValid(Monster))
        {
            Monster->OnMonsterDied.AddDynamic(this, &AHunterBreachEncounterActor::HandleMonsterDied);
            SpawnedMonsters.Add(Monster);
            ++RemainingEnemies;
        }
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
    // Do not complete the encounter while later paced spawns are still pending.
    if (RemainingEnemies == 0 && NextSpawnIndex >= PendingSpawnTransforms.Num())
    {
        SpawnReward();
    }
}

void AHunterBreachEncounterActor::RetryRewardSpawn()
{
    if (bEncounterStarted && !bEncounterComplete &&
        RemainingEnemies <= 0 && NextSpawnIndex >= PendingSpawnTransforms.Num())
    {
        SpawnReward();
    }
}

void AHunterBreachEncounterActor::SpawnReward()
{
    if (bEncounterComplete || !GetWorld() || !RewardPickupClass ||
        RewardItem.ItemId.IsNone() || RewardQuestEventTag.IsNone())
    {
        return;
    }

    const FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 45.0f);
    const FTransform SpawnTransform(GetActorRotation(), SpawnLocation);
    AHunterPickupActor* Pickup = GetWorld()->SpawnActorDeferred<AHunterPickupActor>(
        RewardPickupClass, SpawnTransform, this, nullptr,
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

    if (!Pickup)
    {
        BP_OnRewardSpawnFailed();
        return;
    }

    Pickup->SetPickupItem(RewardItem);
    Pickup->SetQuestEventTag(RewardQuestEventTag);
    UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);
    if (!IsValid(Pickup))
    {
        BP_OnRewardSpawnFailed();
        return;
    }

    bEncounterComplete = true;
    BP_OnRewardSpawned(Pickup);
    BP_OnEncounterCompleted();
}
