#include "HunterHUDWidget.h"

#include "ArinValeCharacter.h"
#include "Components/HunterHealthComponent.h"
#include "Components/HunterAbilityComponent.h"
#include "Components/HunterCurrencyComponent.h"
#include "ArinProgressionComponent.h"

void UHunterHUDWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BindToCharacter();
    RefreshHUD();
}

void UHunterHUDWidget::NativeDestruct()
{
    UnbindFromCharacter();
    Super::NativeDestruct();
}

void UHunterHUDWidget::BindToCharacter()
{
    AArinValeCharacter* NewCharacter = Cast<AArinValeCharacter>(GetOwningPlayerPawn());
    if (ArinCharacter == NewCharacter && ArinCharacter)
    {
        return;
    }

    UnbindFromCharacter();
    ArinCharacter = NewCharacter;
    if (!ArinCharacter)
    {
        return;
    }

    if (UHunterHealthComponent* Health = ArinCharacter->GetHealthComponent())
    {
        Health->OnHealthChanged.AddDynamic(this, &UHunterHUDWidget::HandleHealthChanged);
    }
    if (UHunterAbilityComponent* Abilities = ArinCharacter->GetAbilityComponent())
    {
        Abilities->OnEnergyChanged.AddDynamic(this, &UHunterHUDWidget::HandleEnergyChanged);
    }
    if (UArinProgressionComponent* Progression = ArinCharacter->GetProgressionComponent())
    {
        Progression->OnExperienceChanged.AddDynamic(this, &UHunterHUDWidget::HandleExperienceChanged);
        Progression->OnStageChanged.AddDynamic(this, &UHunterHUDWidget::HandleStageChanged);
    }
    if (UHunterCurrencyComponent* Currency = ArinCharacter->GetCurrencyComponent())
    {
        Currency->OnCurrencyChanged.AddDynamic(this, &UHunterHUDWidget::HandleCurrencyChanged);
    }
}

void UHunterHUDWidget::UnbindFromCharacter()
{
    if (!ArinCharacter)
    {
        return;
    }

    if (UHunterHealthComponent* Health = ArinCharacter->GetHealthComponent())
    {
        Health->OnHealthChanged.RemoveDynamic(this, &UHunterHUDWidget::HandleHealthChanged);
    }
    if (UHunterAbilityComponent* Abilities = ArinCharacter->GetAbilityComponent())
    {
        Abilities->OnEnergyChanged.RemoveDynamic(this, &UHunterHUDWidget::HandleEnergyChanged);
    }
    if (UArinProgressionComponent* Progression = ArinCharacter->GetProgressionComponent())
    {
        Progression->OnExperienceChanged.RemoveDynamic(this, &UHunterHUDWidget::HandleExperienceChanged);
        Progression->OnStageChanged.RemoveDynamic(this, &UHunterHUDWidget::HandleStageChanged);
    }
    if (UHunterCurrencyComponent* Currency = ArinCharacter->GetCurrencyComponent())
    {
        Currency->OnCurrencyChanged.RemoveDynamic(this, &UHunterHUDWidget::HandleCurrencyChanged);
    }
    ArinCharacter = nullptr;
}

void UHunterHUDWidget::RefreshHUD()
{
    if (!ArinCharacter)
    {
        BindToCharacter();
    }

    Snapshot = FHunterHUDSnapshot();
    if (!ArinCharacter)
    {
        BP_OnHUDSnapshotUpdated(Snapshot);
        return;
    }

    if (UHunterHealthComponent* Health = ArinCharacter->GetHealthComponent())
    {
        Snapshot.Health = Health->GetCurrentHealth();
        Snapshot.MaxHealth = Health->GetMaxHealth();
        Snapshot.HealthPercent = Health->GetHealthPercent();
        Snapshot.bIsDead = Health->IsDead();
    }
    if (UHunterAbilityComponent* Abilities = ArinCharacter->GetAbilityComponent())
    {
        Snapshot.Energy = Abilities->GetCurrentEnergy();
        Snapshot.MaxEnergy = Abilities->GetMaxEnergy();
        Snapshot.EnergyPercent = Abilities->GetEnergyPercent();
    }
    if (UArinProgressionComponent* Progression = ArinCharacter->GetProgressionComponent())
    {
        Snapshot.Experience = Progression->GetExperience();
        Snapshot.ExperienceForNextStage = Progression->GetExperienceForNextStage();
        Snapshot.EvolutionStage = Progression->GetEvolutionStage();
        Snapshot.EvolutionStageName = Progression->GetCurrentStageProfile().DisplayName;
        const int32 CurrentThreshold = Progression->GetCurrentStageProfile().RequiredExperience;
        const int32 NextThreshold = Snapshot.ExperienceForNextStage;
        const int32 StageRange = NextThreshold - CurrentThreshold;
        Snapshot.ExperiencePercent = StageRange > 0
            ? FMath::Clamp(static_cast<float>(Snapshot.Experience - CurrentThreshold) / StageRange, 0.0f, 1.0f)
            : 1.0f;
    }
    if (UHunterCurrencyComponent* Currency = ArinCharacter->GetCurrencyComponent())
    {
        Snapshot.Currency = Currency->GetBalance();
    }

    BP_OnHUDSnapshotUpdated(Snapshot);
}

void UHunterHUDWidget::HandleHealthChanged(float CurrentHealth, float MaxHealth)
{
    RefreshHUD();
}

void UHunterHUDWidget::HandleEnergyChanged(float CurrentEnergy, float MaxEnergy)
{
    RefreshHUD();
}

void UHunterHUDWidget::HandleExperienceChanged(int32 NewExperience)
{
    RefreshHUD();
}

void UHunterHUDWidget::HandleStageChanged(EArinEvolutionStage PreviousStage, EArinEvolutionStage NewStage)
{
    RefreshHUD();
}

void UHunterHUDWidget::HandleCurrencyChanged(int64 NewBalance)
{
    RefreshHUD();
}
