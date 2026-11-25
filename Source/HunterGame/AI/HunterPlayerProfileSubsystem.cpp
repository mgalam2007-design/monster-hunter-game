#include "HunterPlayerProfileSubsystem.h"

#include "HunterPlayerProfileSaveGame.h"
#include "ArinValeCharacter.h"
#include "Components/HunterHealthComponent.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    const FString PlayerProfileSlotName = TEXT("HunterPlayerProfile");
}

void UHunterPlayerProfileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeCentroids();
    LoadProfile();
}

void UHunterPlayerProfileSubsystem::Deinitialize()
{
    SaveProfile();
    Super::Deinitialize();
}

void UHunterPlayerProfileSubsystem::InitializeCentroids()
{
    Centroids = {
        FVector(0.90f, 0.55f, 0.25f), // Aggressive: close, moving, healthy
        FVector(0.20f, 0.20f, 0.20f), // Defensive: distant, slow, healthy
        FVector(0.45f, 0.95f, 0.25f), // Evasive: highly mobile
        FVector(0.50f, 0.45f, 0.55f)  // Balanced: mixed behaviour
    };
    ClusterCounts.Init(0, ClusterCount);
}

void UHunterPlayerProfileSubsystem::ValidateModel()
{
    if (Centroids.Num() != ClusterCount || ClusterCounts.Num() != ClusterCount)
    {
        InitializeCentroids();
    }

    for (FVector& Centroid : Centroids)
    {
        if (!FMath::IsFinite(Centroid.X) || !FMath::IsFinite(Centroid.Y) ||
            !FMath::IsFinite(Centroid.Z))
        {
            InitializeCentroids();
            return;
        }
        Centroid.X = FMath::Clamp(Centroid.X, 0.0f, 1.0f);
        Centroid.Y = FMath::Clamp(Centroid.Y, 0.0f, 1.0f);
        Centroid.Z = FMath::Clamp(Centroid.Z, 0.0f, 1.0f);
    }

    for (int64& Count : ClusterCounts)
    {
        Count = FMath::Max<int64>(0, Count);
    }
    ObservationCount = FMath::Max<int64>(0, ObservationCount);
}

int32 UHunterPlayerProfileSubsystem::FindNearestCluster(const FVector& Features) const
{
    int32 BestCluster = 0;
    float BestDistance = TNumericLimits<float>::Max();
    for (int32 Index = 0; Index < Centroids.Num(); ++Index)
    {
        const float Distance = FVector::DistSquared(Features, Centroids[Index]);
        if (FMath::IsFinite(Distance) && Distance < BestDistance)
        {
            BestDistance = Distance;
            BestCluster = Index;
        }
    }
    return BestCluster;
}

EHunterPlayerArchetype UHunterPlayerProfileSubsystem::ObservePlayer(
    AArinValeCharacter* Player, FVector ObserverLocation)
{
    if (!IsValid(Player))
    {
        return CurrentArchetype;
    }

    ValidateModel();
    const float Distance = FVector::Distance(Player->GetActorLocation(), ObserverLocation);
    const float Aggression = FMath::Clamp(1.0f - Distance / 1400.0f, 0.0f, 1.0f);
    const float Mobility = FMath::Clamp(Player->GetVelocity().Size() / 850.0f, 0.0f, 1.0f);
    const UHunterHealthComponent* Health = Player->GetHealthComponent();
    const float HealthRatio = Health ? FMath::Clamp(Health->GetHealthPercent(), 0.0f, 1.0f) : 1.0f;
    const FVector Features(Aggression, Mobility, 1.0f - HealthRatio);

    const int32 Cluster = FindNearestCluster(Features);
    if (!Centroids.IsValidIndex(Cluster) || !ClusterCounts.IsValidIndex(Cluster))
    {
        return CurrentArchetype;
    }

    const int64 NewCount = FMath::Min<int64>(ClusterCounts[Cluster] + 1, 1000000);
    ClusterCounts[Cluster] = NewCount;
    const float LearningRate = FMath::Max(0.01f, 0.20f / FMath::Sqrt(static_cast<float>(NewCount)));
    Centroids[Cluster] += (Features - Centroids[Cluster]) * LearningRate;
    Centroids[Cluster].X = FMath::Clamp(Centroids[Cluster].X, 0.0f, 1.0f);
    Centroids[Cluster].Y = FMath::Clamp(Centroids[Cluster].Y, 0.0f, 1.0f);
    Centroids[Cluster].Z = FMath::Clamp(Centroids[Cluster].Z, 0.0f, 1.0f);

    CurrentArchetype = static_cast<EHunterPlayerArchetype>(Cluster);
    ++ObservationCount;
    ++ObservationsSinceSave;
    if (ObservationsSinceSave >= 25)
    {
        SaveProfile();
    }
    return CurrentArchetype;
}

bool UHunterPlayerProfileSubsystem::SaveProfile()
{
    ValidateModel();
    UHunterPlayerProfileSaveGame* SaveObject = Cast<UHunterPlayerProfileSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UHunterPlayerProfileSaveGame::StaticClass()));
    if (!SaveObject)
    {
        return false;
    }

    SaveObject->Centroids = Centroids;
    SaveObject->ClusterCounts = ClusterCounts;
    SaveObject->ObservationCount = ObservationCount;
    const bool bSaved = UGameplayStatics::SaveGameToSlot(SaveObject, PlayerProfileSlotName, 0);
    if (bSaved)
    {
        ObservationsSinceSave = 0;
    }
    return bSaved;
}

bool UHunterPlayerProfileSubsystem::LoadProfile()
{
    if (!UGameplayStatics::DoesSaveGameExist(PlayerProfileSlotName, 0))
    {
        return false;
    }

    UHunterPlayerProfileSaveGame* SaveObject = Cast<UHunterPlayerProfileSaveGame>(
        UGameplayStatics::LoadGameFromSlot(PlayerProfileSlotName, 0));
    if (!SaveObject || SaveObject->SaveVersion != 1 ||
        SaveObject->Centroids.Num() != ClusterCount || SaveObject->ClusterCounts.Num() != ClusterCount)
    {
        ResetProfile();
        return false;
    }

    Centroids = SaveObject->Centroids;
    ClusterCounts = SaveObject->ClusterCounts;
    ObservationCount = SaveObject->ObservationCount;
    ValidateModel();
    ObservationsSinceSave = 0;
    return true;
}

void UHunterPlayerProfileSubsystem::ResetProfile()
{
    InitializeCentroids();
    ObservationCount = 0;
    ObservationsSinceSave = 0;
    CurrentArchetype = EHunterPlayerArchetype::Balanced;
    SaveProfile();
}
