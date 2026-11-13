#!/usr/bin/env python3
"""Static safeguards for HUNTER 2.0 shop transactions.

This checks source-level rollback and reentrancy patterns; it does not execute
Unreal gameplay or prove cross-component atomicity.
"""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SHOP_CPP = ROOT / "Source/HunterGame/Components/HunterShopComponent.cpp"


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def function_body(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0:
        fail(f"cannot find function {signature}")
    opening = source.find("{", start)
    if opening < 0:
        fail(f"cannot find opening brace for {signature}")
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:index]
    fail(f"unterminated function body for {signature}")


def require(body: str, snippet: str, context: str) -> None:
    if snippet not in body:
        fail(f"{context}: missing required safeguard: {snippet}")


def main() -> None:
    source = SHOP_CPP.read_text(encoding="utf-8")
    buy = function_body(source, "bool UHunterShopComponent::BuyItem(")
    sell = function_body(source, "bool UHunterShopComponent::SellItem(")

    for name, body in (("BuyItem", buy), ("SellItem", sell)):
        require(body, "bTransactionInProgress", name)
        require(body, "TGuardValue<bool> TransactionGuard", name)

    require(buy, "!Currency->AddCurrency(TotalCost)", "BuyItem")
    require(buy, "Shop purchase refund failed", "BuyItem")
    require(buy, "UE_LOG(LogTemp, Error", "BuyItem")

    require(sell, "!Currency->AddCurrency(Payout)", "SellItem")
    require(sell, "!Inventory->AddItem(SoldItem)", "SellItem")
    require(sell, "Shop transaction rollback failed", "SellItem")
    require(sell, "UE_LOG(LogTemp, Error", "SellItem")

    payout_check = sell.find("if (!Currency->AddCurrency(Payout))")
    stock_change = sell.find("Entry->Stock =", payout_check)
    if payout_check < 0 or stock_change < 0 or stock_change < payout_check:
        fail("SellItem: stock must only change after successful currency payout")

    print("Validated shop buy/sell reentrancy guards, checked refund/rollback paths, failure logging, and post-payout stock updates.")


if __name__ == "__main__":
    main()
