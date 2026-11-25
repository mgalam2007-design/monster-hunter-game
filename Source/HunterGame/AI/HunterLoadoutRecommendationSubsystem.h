#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Components/HunterInventoryComponent.h"
#include "AI/HunterPlayerProfileSubsystem.h"
#include "HunterLoadoutRecommendationSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FHunterEquipmentRecommendation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="AI|Recommendation")
    FHunterInventoryItem Item;

    UPROPERTY(BlueprintReadOnly, Category="AI|Recommendation")
    float Score = 0.0f;
};

/**
 * Deterministic local content-based equipment recommender. It ranks existing
 * candidate items using player archetype, evolution eligibility and item stats.
 */
UCLASS()
class HUNTERGAME_API UHunterLoadoutRecommendationSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="AI|Recommendation")
    TArray<FHunterEquipmentRecommendation> RecommendEquipment(
        const TArray<FHunterInventoryItem>& Candidates,
        int32 EvolutionStage,
        EHunterPlayerArchetype PlayerArchetype,
        int32 MaxResults = 3) const;

private:
    float ScoreItem(const FHunterInventoryItem& Item, int32 EvolutionStage,
        EHunterPlayerArchetype PlayerArchetype) const;
};
