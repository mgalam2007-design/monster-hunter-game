#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HunterCurrencyComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHunterCurrencyChanged, int64, NewBalance);

/** Persistent single-player economy wallet. */
UCLASS(ClassGroup=(Hunter), meta=(BlueprintSpawnableComponent))
class HUNTERGAME_API UHunterCurrencyComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Economy")
    bool AddCurrency(int64 Amount);

    UFUNCTION(BlueprintCallable, Category="Economy")
    bool SpendCurrency(int64 Amount);

    UFUNCTION(BlueprintCallable, Category="Economy")
    void SetBalance(int64 NewBalance);

    UFUNCTION(BlueprintPure, Category="Economy")
    int64 GetBalance() const { return Balance; }

    UPROPERTY(BlueprintAssignable, Category="Economy")
    FHunterCurrencyChanged OnCurrencyChanged;

private:
    UPROPERTY(VisibleAnywhere, Category="Economy", meta=(ClampMin="0"))
    int64 Balance = 0;
};
