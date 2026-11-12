#include "ArinProgressionComponent.h"

UArinProgressionComponent::UArinProgressionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    const FText Names[] = {
        FText::FromString(TEXT("Awakening")), FText::FromString(TEXT("Evolved")),
        FText::FromString(TEXT("Elite")), FText::FromString(TEXT("Legendary")),
        FText::FromString(TEXT("Ultimate Awakening"))
    };
    const int32 Thresholds[] = { 0, 100, 350, 800, 1600 };
    const float Health[] = { 1.0f, 1.12f, 1.28f, 1.48f, 1.75f };
    const float Damage[] = { 1.0f, 1.15f, 1.35f, 1.65f, 2.0f };
    const float Movement[] = { 1.0f, 1.04f, 1.08f, 1.12f, 1.18f };

    StageProfiles.SetNum(5);
    for (int32 Index = 0; Index < StageProfiles.Num(); ++Index)
    {
        StageProfiles[Index].DisplayName = Names[Index];
        StageProfiles[Index].RequiredExperience = Thresholds[Index];
        StageProfiles[Index].HealthMultiplier = Health[Index];
        StageProfiles[Index].DamageMultiplier = Damage[Index];
        StageProfiles[Index].MovementMultiplier = Movement[Index];
    }
}

void UArinProgressionComponent::AddExperience(int32 Amount)
{
    if (Amount <= 0) return;
    const int64 NewTotal = static_cast<int64>(Experience) + static_cast<int64>(Amount);
    Experience = static_cast<int32>(FMath::Clamp<int64>(NewTotal, 0, MAX_int32));
    OnExperienceChanged.Broadcast(Experience);
    RecalculateStage();
}

bool UArinProgressionComponent::SetEvolutionStage(EArinEvolutionStage NewStage)
{
    const int32 NewIndex = GetStageIndex(NewStage);
    const int32 CurrentIndex = GetStageIndex(CurrentStage);
    if (!StageProfiles.IsValidIndex(NewIndex) || NewIndex < CurrentIndex) return false;

    const int32 PreviousExperience = Experience;
    Experience = FMath::Max(Experience, StageProfiles[NewIndex].RequiredExperience);
    const EArinEvolutionStage PreviousStage = CurrentStage;
    CurrentStage = NewStage;

    if (Experience != PreviousExperience) OnExperienceChanged.Broadcast(Experience);
    if (PreviousStage != CurrentStage) OnStageChanged.Broadcast(PreviousStage, CurrentStage);
    return true;
}

void UArinProgressionComponent::RestoreProgression(int32 SavedExperience, EArinEvolutionStage SavedStage)
{
    const int32 SafeStageIndex = FMath::Clamp(GetStageIndex(SavedStage), 0, StageProfiles.Num() - 1);
    const int32 SafeExperience = FMath::Max(0, SavedExperience);
    const int32 PreviousExperience = Experience;
    const EArinEvolutionStage PreviousStage = CurrentStage;

    Experience = FMath::Max(SafeExperience, StageProfiles[SafeStageIndex].RequiredExperience);
    RecalculateStage();

    if (Experience != PreviousExperience) OnExperienceChanged.Broadcast(Experience);
    if (CurrentStage != PreviousStage) OnStageChanged.Broadcast(PreviousStage, CurrentStage);
}

int32 UArinProgressionComponent::GetExperienceForNextStage() const
{
    const int32 NextIndex = GetStageIndex(CurrentStage) + 1;
    return StageProfiles.IsValidIndex(NextIndex) ? StageProfiles[NextIndex].RequiredExperience : -1;
}

FArinStageProfile UArinProgressionComponent::GetCurrentStageProfile() const
{
    const int32 Index = GetStageIndex(CurrentStage);
    return StageProfiles.IsValidIndex(Index) ? StageProfiles[Index] : FArinStageProfile();
}

bool UArinProgressionComponent::HasUnlockedShadowStep() const
{
    return GetStageIndex(CurrentStage) >= GetStageIndex(EArinEvolutionStage::Legendary);
}

bool UArinProgressionComponent::HasUnlockedEnergyStrike() const
{
    return GetStageIndex(CurrentStage) >= GetStageIndex(EArinEvolutionStage::Elite);
}

bool UArinProgressionComponent::HasUnlockedShadowEcho() const
{
    return GetStageIndex(CurrentStage) >= GetStageIndex(EArinEvolutionStage::Legendary);
}

int32 UArinProgressionComponent::GetStageIndex(EArinEvolutionStage Stage) const
{
    return static_cast<int32>(Stage);
}

void UArinProgressionComponent::RecalculateStage()
{
    EArinEvolutionStage NewStage = EArinEvolutionStage::Awakening;
    for (int32 Index = 0; Index < StageProfiles.Num(); ++Index)
    {
        if (Experience >= StageProfiles[Index].RequiredExperience)
            NewStage = static_cast<EArinEvolutionStage>(Index);
    }

    if (NewStage != CurrentStage)
    {
        const EArinEvolutionStage PreviousStage = CurrentStage;
        CurrentStage = NewStage;
        OnStageChanged.Broadcast(PreviousStage, CurrentStage);
    }
}
