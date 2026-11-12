#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HunterWeaponTraceComponent.generated.h"

UCLASS(ClassGroup=(Hunter), meta=(BlueprintSpawnableComponent))
class HUNTERGAME_API UHunterWeaponTraceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHunterWeaponTraceComponent();

    UFUNCTION(BlueprintCallable, Category="Combat|Weapon")
    void BeginWeaponTrace();

    UFUNCTION(BlueprintCallable, Category="Combat|Weapon")
    void EndWeaponTrace();

    UFUNCTION(BlueprintCallable, Category="Combat|Weapon")
    void SetBaseDamage(float NewDamage);

    UFUNCTION(BlueprintPure, Category="Combat|Weapon")
    bool IsTracing() const { return bIsTracing; }

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Weapon")
    FName BaseSocket = TEXT("weapon_r");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Weapon")
    FName TipSocket = TEXT("weapon_tip");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Weapon", meta=(ClampMin="1.0"))
    float BaseDamage = 35.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Weapon")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

private:
    bool bIsTracing = false;
    FVector PreviousBase = FVector::ZeroVector;
    FVector PreviousTip = FVector::ZeroVector;
    TSet<TWeakObjectPtr<AActor>> HitActors;

    void TraceWeaponSegment(const FVector& CurrentBase, const FVector& CurrentTip);
};
