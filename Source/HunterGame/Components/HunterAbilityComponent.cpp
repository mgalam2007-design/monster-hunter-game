#include "HunterAbilityComponent.h"

#include "ArinProgressionComponent.h"
#include "ArinValeCharacter.h"
#include "Characters/HunterMonsterBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"

UHunterAbilityComponent::UHunterAbilityComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UHunterAbilityComponent::BeginPlay()
{
    Super::BeginPlay();
    MaxEnergy = FMath::IsFinite(MaxEnergy) ? FMath::Max(0.0f, MaxEnergy) : 100.0f;
    EnergyRegenerationPerSecond = FMath::IsFinite(EnergyRegenerationPerSecond)
        ? FMath::Max(0.0f, EnergyRegenerationPerSecond) : 8.0f;
    EnergyStrikeDamage = FMath::IsFinite(EnergyStrikeDamage)
        ? FMath::Max(0.0f, EnergyStrikeDamage) : 65.0f;
    EnergyStrikeRadius = FMath::IsFinite(EnergyStrikeRadius)
        ? FMath::Max(0.0f, EnergyStrikeRadius) : 420.0f;
    ShadowStepStrength = FMath::IsFinite(ShadowStepStrength)
        ? FMath::Max(0.0f, ShadowStepStrength) : 1250.0f;
    ShadowEchoSpawnDistance = FMath::IsFinite(ShadowEchoSpawnDistance)
        ? FMath::Max(0.0f, ShadowEchoSpawnDistance) : 180.0f;
    CurrentEnergy = FMath::Clamp(FMath::IsFinite(CurrentEnergy) ? CurrentEnergy : MaxEnergy, 0.0f, MaxEnergy);
    OnEnergyChanged.Broadcast(CurrentEnergy, MaxEnergy);
}

float UHunterAbilityComponent::RestoreEnergy(float Amount)
{
    if (!FMath::IsFinite(Amount) || Amount <= 0.0f || MaxEnergy <= 0.0f) return 0.0f;
    const float PreviousEnergy = CurrentEnergy;
    CurrentEnergy = FMath::Clamp(CurrentEnergy + Amount, 0.0f, MaxEnergy);
    const float Restored = CurrentEnergy - PreviousEnergy;
    if (Restored > 0.0f) OnEnergyChanged.Broadcast(CurrentEnergy, MaxEnergy);
    return Restored;
}

void UHunterAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!FMath::IsFinite(DeltaTime) || DeltaTime <= 0.0f)
    {
        return;
    }

    for (auto It = CooldownRemaining.CreateIterator(); It; ++It)
    {
        It.Value() = FMath::Max(0.0f, It.Value() - DeltaTime);
    }

    const float PreviousEnergy = CurrentEnergy;
    const float SafeCurrentEnergy = FMath::IsFinite(CurrentEnergy) ? CurrentEnergy : MaxEnergy;
    const float Regeneration = FMath::IsFinite(EnergyRegenerationPerSecond)
        ? FMath::Max(0.0f, EnergyRegenerationPerSecond) * DeltaTime : 0.0f;
    CurrentEnergy = FMath::Clamp(SafeCurrentEnergy + Regeneration, 0.0f, MaxEnergy);
    if (!FMath::IsNearlyEqual(PreviousEnergy, CurrentEnergy))
    {
        OnEnergyChanged.Broadcast(CurrentEnergy, MaxEnergy);
    }
}

bool UHunterAbilityComponent::TryActivateAbility(EHunterAbility Ability)
{
    if (!IsAbilityUnlocked(Ability) || GetCooldownRemaining(Ability) > 0.0f)
    {
        return false;
    }

    const float Cost = GetAbilityCost(Ability);
    if (!FMath::IsFinite(Cost) || Cost < 0.0f ||
        !FMath::IsFinite(GetAbilityCooldown(Ability)) || GetAbilityCooldown(Ability) < 0.0f)
    {
        return false;
    }

    if (!FMath::IsFinite(CurrentEnergy) || CurrentEnergy < Cost)
    {
        return false;
    }

    if (!ExecuteAbility(Ability))
    {
        return false;
    }

    CurrentEnergy -= Cost;
    CooldownRemaining.Add(Ability, GetAbilityCooldown(Ability));
    OnEnergyChanged.Broadcast(CurrentEnergy, MaxEnergy);
    OnAbilityActivated.Broadcast(Ability);
    BP_OnAbilityActivated(Ability);
    return true;
}

float UHunterAbilityComponent::GetCooldownRemaining(EHunterAbility Ability) const
{
    const float* Remaining = CooldownRemaining.Find(Ability);
    return Remaining ? FMath::Max(0.0f, *Remaining) : 0.0f;
}

bool UHunterAbilityComponent::IsAbilityUnlocked(EHunterAbility Ability) const
{
    const AArinValeCharacter* Arin = Cast<AArinValeCharacter>(GetOwner());
    const UArinProgressionComponent* Progression = Arin ? Arin->GetProgressionComponent() : nullptr;
    if (!Progression)
    {
        return false;
    }

    switch (Ability)
    {
        case EHunterAbility::EnergyStrike: return Progression->HasUnlockedEnergyStrike();
        case EHunterAbility::ShadowStep: return Progression->HasUnlockedShadowStep();
        case EHunterAbility::ShadowEcho: return Progression->HasUnlockedShadowEcho();
        default: return false;
    }
}

float UHunterAbilityComponent::GetAbilityCost(EHunterAbility Ability) const
{
    float BaseCost = MaxEnergy;
    switch (Ability)
    {
        case EHunterAbility::EnergyStrike: BaseCost = 25.0f; break;
        case EHunterAbility::ShadowStep: BaseCost = 35.0f; break;
        case EHunterAbility::ShadowEcho: BaseCost = 60.0f; break;
        default: return MaxEnergy;
    }

    const AArinValeCharacter* Arin = Cast<AArinValeCharacter>(GetOwner());
    const UArinProgressionComponent* Progression = Arin ? Arin->GetProgressionComponent() : nullptr;
    const int32 Stage = Progression ? FMath::Clamp(static_cast<int32>(Progression->GetEvolutionStage()), 0, 4) : 0;
    return BaseCost * FMath::Max(0.75f, 1.0f - 0.05f * Stage);
}

float UHunterAbilityComponent::GetAbilityCooldown(EHunterAbility Ability) const
{
    float BaseCooldown = 1.0f;
    switch (Ability)
    {
        case EHunterAbility::EnergyStrike: BaseCooldown = 4.0f; break;
        case EHunterAbility::ShadowStep: BaseCooldown = 7.0f; break;
        case EHunterAbility::ShadowEcho: BaseCooldown = 25.0f; break;
        default: return BaseCooldown;
    }

    const AArinValeCharacter* Arin = Cast<AArinValeCharacter>(GetOwner());
    const UArinProgressionComponent* Progression = Arin ? Arin->GetProgressionComponent() : nullptr;
    const int32 Stage = Progression ? FMath::Clamp(static_cast<int32>(Progression->GetEvolutionStage()), 0, 4) : 0;
    return BaseCooldown * FMath::Max(0.80f, 1.0f - 0.05f * Stage);
}

bool UHunterAbilityComponent::ExecuteAbility(EHunterAbility Ability)
{
    AArinValeCharacter* Arin = Cast<AArinValeCharacter>(GetOwner());
    if (!Arin || !GetWorld())
    {
        return false;
    }

    switch (Ability)
    {
        case EHunterAbility::EnergyStrike:
        {
            const UArinProgressionComponent* Progression = Arin->GetProgressionComponent();
            const FArinStageProfile Profile = Progression
                ? Progression->GetCurrentStageProfile() : FArinStageProfile();
            const int32 Stage = Progression
                ? FMath::Clamp(static_cast<int32>(Progression->GetEvolutionStage()), 0, 4) : 0;
            const float DamageMultiplier = FMath::IsFinite(Profile.DamageMultiplier)
                ? FMath::Clamp(Profile.DamageMultiplier, 0.5f, 3.0f) : 1.0f;
            const float ScaledRadius = EnergyStrikeRadius * (1.0f + 0.05f * Stage);
            const float ScaledDamage = EnergyStrikeDamage * DamageMultiplier;
            const FVector Origin = Arin->GetActorLocation() + Arin->GetActorForwardVector() * 150.0f;
            const FCollisionShape Shape = FCollisionShape::MakeSphere(FMath::Max(0.0f, ScaledRadius));
            FCollisionQueryParams Params(SCENE_QUERY_STAT(EnergyStrike), false, Arin);
            TArray<FOverlapResult> Overlaps;
            GetWorld()->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity,
                FCollisionObjectQueryParams(ECC_Pawn), Shape, Params);

            TSet<AActor*> DamagedActors;
            for (const FOverlapResult& Overlap : Overlaps)
            {
                AActor* Target = Overlap.GetActor();
                if (!Target || Target == Arin || !Target->IsA<AHunterMonsterBase>() || DamagedActors.Contains(Target))
                {
                    continue;
                }

                DamagedActors.Add(Target);
                UGameplayStatics::ApplyDamage(Target, ScaledDamage, Arin->GetController(), Arin, UDamageType::StaticClass());
            }
            return true;
        }

        case EHunterAbility::ShadowStep:
        {
            const UArinProgressionComponent* Progression = Arin->GetProgressionComponent();
            const FArinStageProfile Profile = Progression
                ? Progression->GetCurrentStageProfile() : FArinStageProfile();
            const float MovementMultiplier = FMath::IsFinite(Profile.MovementMultiplier)
                ? FMath::Clamp(Profile.MovementMultiplier, 0.5f, 2.0f) : 1.0f;
            const FVector Direction = Arin->GetActorForwardVector();
            Arin->LaunchCharacter(Direction * ShadowStepStrength * MovementMultiplier, true, false);
            return true;
        }

        case EHunterAbility::ShadowEcho:
        {
            if (!ShadowEchoActorClass)
            {
                return false;
            }

            const FVector SpawnLocation = Arin->GetActorLocation() - Arin->GetActorForwardVector() * ShadowEchoSpawnDistance;
            const FTransform SpawnTransform(Arin->GetActorRotation(), SpawnLocation);
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = Arin;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
            return GetWorld()->SpawnActor<AActor>(ShadowEchoActorClass, SpawnTransform, SpawnParams) != nullptr;
        }

        default:
            return false;
    }
}
