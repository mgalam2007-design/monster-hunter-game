#include "Misc/GuardValue.h"

#include "HunterCurrencyComponent.h"

bool UHunterCurrencyComponent::AddCurrency(int64 Amount)
{
    if (bMutationInProgress || Amount <= 0 || Balance > MAX_int64 - Amount)
    {
        return false;
    }

    TGuardValue<bool> MutationGuard(bMutationInProgress, true);
    Balance += Amount;
    OnCurrencyChanged.Broadcast(Balance);
    return true;
}

bool UHunterCurrencyComponent::SpendCurrency(int64 Amount)
{
    if (bMutationInProgress || Amount <= 0 || Balance < Amount)
    {
        return false;
    }

    TGuardValue<bool> MutationGuard(bMutationInProgress, true);
    Balance -= Amount;
    OnCurrencyChanged.Broadcast(Balance);
    return true;
}

void UHunterCurrencyComponent::SetBalance(int64 NewBalance)
{
    if (bMutationInProgress) return;
    TGuardValue<bool> MutationGuard(bMutationInProgress, true);
    Balance = FMath::Max<int64>(0, NewBalance);
    OnCurrencyChanged.Broadcast(Balance);
}
