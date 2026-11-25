#include "AI/HunterLoadoutRecommendationSubsystem.h"

float UHunterLoadoutRecommendationSubsystem::ScoreItem(
    const FHunterInventoryItem& Item, int32 EvolutionStage,
    EHunterPlayerArchetype PlayerArchetype) const
{
    if ((Item.ItemType != EHunterItemType::Weapon && Item.ItemType != EHunterItemType::Armor) ||
        Item.ItemId.IsNone() || Item.RequiredEvolutionStage > EvolutionStage)
    {
        return -1.0f;
    }

    const float Damage = FMath::IsFinite(Item.DamageBonus)
        ? FMath::Clamp(Item.DamageBonus, 0.0f, 500.0f) : 0.0f;
    const float Reduction = FMath::IsFinite(Item.DamageReduction)
        ? FMath::Clamp(Item.DamageReduction, 0.0f, 0.8f) : 0.0f;

    float DamageWeight = 1.0f;
    float DefenseWeight = 100.0f;
    switch (PlayerArchetype)
    {
    case EHunterPlayerArchetype::Aggressive:
        DamageWeight = 1.35f;
        DefenseWeight = 75.0f;
        break;
    case EHunterPlayerArchetype::Defensive:
        DamageWeight = 0.80f;
        DefenseWeight = 140.0f;
        break;
    case EHunterPlayerArchetype::Evasive:
        DamageWeight = 1.0f;
        DefenseWeight = 110.0f;
        break;
    case EHunterPlayerArchetype::Balanced:
    default:
        break;
    }

    float Score = 0.0f;
    if (Item.ItemType == EHunterItemType::Weapon)
    {
        Score = Damage * DamageWeight;
    }
    else
    {
        Score = Reduction * DefenseWeight;
    }

    // Slightly prefer usable equipment with a stable, explicit item identifier.
    return FMath::IsFinite(Score) ? FMath::Clamp(Score, 0.0f, 1000.0f) : 0.0f;
}

TArray<FHunterEquipmentRecommendation> UHunterLoadoutRecommendationSubsystem::RecommendEquipment(
    const TArray<FHunterInventoryItem>& Candidates, int32 EvolutionStage,
    EHunterPlayerArchetype PlayerArchetype, int32 MaxResults) const
{
    TArray<FHunterEquipmentRecommendation> Results;
    const int32 SafeStage = FMath::Clamp(EvolutionStage, 0, 4);
    const int32 ResultLimit = FMath::Clamp(MaxResults, 0, 10);
    if (ResultLimit == 0 || Candidates.IsEmpty())
    {
        return Results;
    }

    for (const FHunterInventoryItem& Item : Candidates)
    {
        const float Score = ScoreItem(Item, SafeStage, PlayerArchetype);
        if (Score < 0.0f || Score <= KINDA_SMALL_NUMBER)
        {
            continue;
        }

        FHunterEquipmentRecommendation& Entry = Results.AddDefaulted_GetRef();
        Entry.Item = Item;
        Entry.Score = Score;
    }

    Results.Sort([](const FHunterEquipmentRecommendation& A, const FHunterEquipmentRecommendation& B)
    {
        if (!FMath::IsNearlyEqual(A.Score, B.Score))
        {
            return A.Score > B.Score;
        }
        return A.Item.ItemId.LexicalLess(B.Item.ItemId);
    });

    if (Results.Num() > ResultLimit)
    {
        Results.SetNum(ResultLimit);
    }
    return Results;
}
