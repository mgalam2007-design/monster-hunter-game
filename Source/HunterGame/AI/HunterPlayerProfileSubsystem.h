#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HunterPlayerProfileSubsystem.generated.h"

class AArinValeCharacter;

UENUM(BlueprintType)
enum class EHunterPlayerArchetype : uint8
{
    Aggressive = 0,
    Defensive = 1,
    Evasive = 2,
    Balanced = 3
};

/**
 * Lightweight online k-means profile model. Features are normalized gameplay
 * observations; this is local clustering, not a hosted player analytics service.
 */
UCLASS()
class HUNTERGAME_API UHunterPlayerProfileSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category="AI|Player Profile")
    EHunterPlayerArchetype ObservePlayer(AArinValeCharacter* Player, FVector ObserverLocation);

    UFUNCTION(BlueprintPure, Category="AI|Player Profile")
    EHunterPlayerArchetype GetCurrentArchetype() const { return CurrentArchetype; }

    UFUNCTION(BlueprintPure, Category="AI|Player Profile")
    int64 GetObservationCount() const { return ObservationCount; }

    UFUNCTION(BlueprintCallable, Category="AI|Player Profile|Save")
    bool SaveProfile();

    UFUNCTION(BlueprintCallable, Category="AI|Player Profile|Save")
    bool LoadProfile();

    UFUNCTION(BlueprintCallable, Category="AI|Player Profile|Save")
    void ResetProfile();

private:
    static constexpr int32 ClusterCount = 4;

    UPROPERTY(Transient)
    TArray<FVector> Centroids;

    UPROPERTY(Transient)
    TArray<int64> ClusterCounts;

    UPROPERTY(Transient)
    EHunterPlayerArchetype CurrentArchetype = EHunterPlayerArchetype::Balanced;

    UPROPERTY(Transient)
    int64 ObservationCount = 0;

    int32 ObservationsSinceSave = 0;

    void InitializeCentroids();
    void ValidateModel();
    int32 FindNearestCluster(const FVector& Features) const;
};
