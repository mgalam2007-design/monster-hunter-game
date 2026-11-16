#include "HunterInventoryWidget.h"

#include "ArinValeCharacter.h"
#include "Components/HunterHealthComponent.h"
#include "Components/HunterAbilityComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"

void UHunterInventoryWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BuildNativeLayout();
    BindCharacter();
    RefreshInventory();
}

void UHunterInventoryWidget::NativeDestruct()
{
    UnbindCharacter();
    Super::NativeDestruct();
}

void UHunterInventoryWidget::BindCharacter()
{
    AArinValeCharacter* NewCharacter = Cast<AArinValeCharacter>(GetOwningPlayerPawn());
    if (ArinCharacter == NewCharacter && ArinCharacter)
    {
        return;
    }

    UnbindCharacter();
    ArinCharacter = NewCharacter;
    if (!ArinCharacter)
    {
        return;
    }

    InventoryComponent = ArinCharacter->GetInventoryComponent();
    EquipmentComponent = ArinCharacter->GetEquipmentComponent();

    if (InventoryComponent)
    {
        InventoryComponent->OnInventoryChanged.AddDynamic(this, &UHunterInventoryWidget::HandleInventoryChanged);
    }
    if (EquipmentComponent)
    {
        EquipmentComponent->OnEquipmentChanged.AddDynamic(this, &UHunterInventoryWidget::HandleEquipmentChanged);
    }
}

void UHunterInventoryWidget::UnbindCharacter()
{
    if (InventoryComponent)
    {
        InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UHunterInventoryWidget::HandleInventoryChanged);
    }
    if (EquipmentComponent)
    {
        EquipmentComponent->OnEquipmentChanged.RemoveDynamic(this, &UHunterInventoryWidget::HandleEquipmentChanged);
    }

    InventoryComponent = nullptr;
    EquipmentComponent = nullptr;
    ArinCharacter = nullptr;
}

void UHunterInventoryWidget::RefreshInventory()
{
    if (!ArinCharacter)
    {
        BindCharacter();
    }

    Snapshot = FHunterInventorySnapshot();
    if (InventoryComponent)
    {
        const TArray<FHunterInventoryItem> Items = InventoryComponent->GetItems();
        Snapshot.TotalItemStacks = Items.Num();

        for (const FHunterInventoryItem& Item : Items)
        {
            FHunterInventoryDisplayEntry DisplayEntry;
            DisplayEntry.Item = Item;
            Snapshot.TotalItemCount += Item.Quantity;

            if (ItemCatalog)
            {
                FHunterItemDefinition Definition;
                if (ItemCatalog->FindItem(Item.ItemId, Definition))
                {
                    DisplayEntry.Description = Definition.Description;
                    DisplayEntry.Rarity = Definition.Rarity;
                    DisplayEntry.Icon = Definition.Icon;
                }
            }

            if (Item.ItemType == EHunterItemType::Consumable)
            {
                const UHunterHealthComponent* Health = ArinCharacter ? ArinCharacter->GetHealthComponent() : nullptr;
                const UHunterAbilityComponent* Abilities = ArinCharacter ? ArinCharacter->GetAbilityComponent() : nullptr;
                const bool bCanHeal = Health && !Health->IsDead() && Item.HealthRestoreAmount > 0.0f
                    && Health->GetCurrentHealth() < Health->GetMaxHealth();
                const bool bCanRestoreEnergy = Abilities && Item.EnergyRestoreAmount > 0.0f
                    && Abilities->GetCurrentEnergy() < Abilities->GetMaxEnergy();
                DisplayEntry.bCanUseConsumable = bCanHeal || bCanRestoreEnergy;
            }

            Snapshot.Entries.Add(DisplayEntry);
        }
    }

    if (EquipmentComponent)
    {
        Snapshot.EquippedWeapon = EquipmentComponent->GetWeapon();
        Snapshot.EquippedArmor = EquipmentComponent->GetArmor();
    }

    ApplyNativeLayout();
    BP_OnInventorySnapshotUpdated(Snapshot);
}

void UHunterInventoryWidget::BuildNativeLayout()
{
    if (!WidgetTree || WidgetTree->RootWidget)
    {
        return;
    }
    NativeRoot = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("NativeInventoryRoot"));
    WidgetTree->RootWidget = NativeRoot;
    UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("InventoryPanel"));
    Panel->SetBrushColor(FLinearColor(0.012f, 0.02f, 0.035f, 0.94f));
    Panel->SetPadding(FMargin(22.0f));
    UCanvasPanelSlot* PanelSlot = NativeRoot->AddChildToCanvas(Panel);
    PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
    PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    PanelSlot->SetPosition(FVector2D::ZeroVector);
    PanelSlot->SetSize(FVector2D(520.0f, 460.0f));
    NativeStack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InventoryStack"));
    Panel->SetContent(NativeStack);
}

void UHunterInventoryWidget::ApplyNativeLayout()
{
    if (!NativeStack || !WidgetTree)
    {
        return;
    }
    NativeStack->ClearChildren();
    auto AddLine = [this](const FText& Value, int32 FontSize, const FLinearColor& Color)
    {
        UTextBlock* Line = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        Line->SetText(Value);
        FSlateFontInfo Font = Line->GetFont();
        Font.Size = FontSize;
        Line->SetFont(Font);
        Line->SetColorAndOpacity(FSlateColor(Color));
        UVerticalBoxSlot* Slot = NativeStack->AddChildToVerticalBox(Line);
        Slot->SetPadding(FMargin(0.0f, 3.0f, 0.0f, 5.0f));
    };

    AddLine(FText::FromString(TEXT("HUNTER // INVENTORY")), 22, FLinearColor(0.96f, 0.97f, 1.0f, 1.0f));
    AddLine(FText::FromString(FString::Printf(TEXT("%d STACKS  //  %d ITEMS"), Snapshot.TotalItemStacks, Snapshot.TotalItemCount)),
        11, FLinearColor(0.35f, 0.72f, 1.0f, 1.0f));
    AddLine(FText::FromString(TEXT("EQUIPPED")), 13, FLinearColor(0.9f, 0.68f, 0.25f, 1.0f));
    const FString Weapon = Snapshot.EquippedWeapon.ItemId.IsNone() ? TEXT("None") : Snapshot.EquippedWeapon.DisplayName.ToString();
    const FString Armor = Snapshot.EquippedArmor.ItemId.IsNone() ? TEXT("None") : Snapshot.EquippedArmor.DisplayName.ToString();
    AddLine(FText::FromString(TEXT("Weapon: ") + Weapon), 12, FLinearColor(0.88f, 0.9f, 0.95f, 1.0f));
    AddLine(FText::FromString(TEXT("Armor: ") + Armor), 12, FLinearColor(0.88f, 0.9f, 0.95f, 1.0f));
    AddLine(FText::FromString(TEXT("ITEMS")), 13, FLinearColor(0.9f, 0.68f, 0.25f, 1.0f));
    if (Snapshot.Entries.IsEmpty())
    {
        AddLine(FText::FromString(TEXT("Your inventory is empty.")), 13, FLinearColor(0.65f, 0.7f, 0.78f, 1.0f));
    }
    else
    {
        for (const FHunterInventoryDisplayEntry& Entry : Snapshot.Entries)
        {
            const FString ItemName = Entry.Item.DisplayName.IsEmpty()
                ? Entry.Item.ItemId.ToString() : Entry.Item.DisplayName.ToString();
            const FString ItemLine = FString::Printf(TEXT("%s   x%d"), *ItemName, Entry.Item.Quantity);
            AddLine(FText::FromString(ItemLine), 13, FLinearColor(0.88f, 0.91f, 0.97f, 1.0f));
        }
    }
    AddLine(FText::FromString(TEXT("Press I to close")), 11, FLinearColor(0.45f, 0.65f, 0.82f, 1.0f));
}

bool UHunterInventoryWidget::EquipItemById(FName ItemId)
{
    if (!EquipmentComponent || !EquipmentComponent->EquipInventoryItem(ItemId))
    {
        return false;
    }

    RefreshInventory();
    return true;
}

bool UHunterInventoryWidget::UnequipSlot(EHunterEquipmentSlot Slot)
{
    if (!EquipmentComponent || !EquipmentComponent->UnequipSlot(Slot))
    {
        return false;
    }

    RefreshInventory();
    return true;
}

void UHunterInventoryWidget::HandleInventoryChanged(FName ItemId)
{
    RefreshInventory();
}

void UHunterInventoryWidget::HandleEquipmentChanged()
{
    RefreshInventory();
}

bool UHunterInventoryWidget::UseConsumableById(FName ItemId)
{
    if (ItemId.IsNone() || !InventoryComponent || !ArinCharacter)
    {
        return false;
    }

    const TArray<FHunterInventoryItem> Items = InventoryComponent->GetItems();
    const FHunterInventoryItem* FoundItem = Items.FindByPredicate([ItemId](const FHunterInventoryItem& Item)
    {
        return Item.ItemId == ItemId && Item.ItemType == EHunterItemType::Consumable && Item.Quantity > 0;
    });
    if (!FoundItem)
    {
        return false;
    }

    UHunterHealthComponent* Health = ArinCharacter->GetHealthComponent();
    UHunterAbilityComponent* Abilities = ArinCharacter->GetAbilityComponent();
    const bool bCanHeal = Health && FoundItem->HealthRestoreAmount > 0.0f
        && Health->GetCurrentHealth() < Health->GetMaxHealth();
    const bool bCanRestoreEnergy = Abilities && FoundItem->EnergyRestoreAmount > 0.0f
        && Abilities->GetCurrentEnergy() < Abilities->GetMaxEnergy();

    if ((!bCanHeal && !bCanRestoreEnergy) || (Health && Health->IsDead()))
    {
        return false;
    }

    const float HealthRestore = FoundItem->HealthRestoreAmount;
    const float EnergyRestore = FoundItem->EnergyRestoreAmount;
    if (!InventoryComponent->RemoveItem(ItemId, 1))
    {
        return false;
    }

    if (bCanHeal) Health->Heal(HealthRestore);
    if (bCanRestoreEnergy) Abilities->RestoreEnergy(EnergyRestore);
    RefreshInventory();
    return true;
}
