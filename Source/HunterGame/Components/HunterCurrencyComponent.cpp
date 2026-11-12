#include "HunterCurrencyComponent.h"

bool UHunterCurrencyComponent::AddCurrency(int64 Amount)
{
    if (Amount <= 0 || Balance > MAX_int64 - Amount)
    {
        return false;
    }

    Balance += Amount;
    OnCurrencyChanged.Broadcast(Balance);
    return true;
}

bool UHunterCurrencyComponent::SpendCurrency(int64 Amount)
{
    if (Amount <= 0 || Balance < Amount)
    {
        return false;
    }

    Balance -= Amount;
    OnCurrencyChanged.Broadcast(Balance);
    return true;
}

void UHunterCurrencyComponent::SetBalance(int64 NewBalance)
{
    Balance = FMath::Max<int64>(0, NewBalance);
    OnCurrencyChanged.Broadcast(Balance);
}
