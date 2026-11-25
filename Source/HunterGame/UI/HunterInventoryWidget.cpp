#include "HunterInventoryWidget.h"

#include "ArinValeCharacter.h"
#include "ArinProgressionComponent.h"
#include "AI/HunterLoadoutRecommendationSubsystem.h"
#include "AI/HunterPlayerProfileSubsystem.h"
#include "Engine/GameInstance.h"
#include "Components/HunterHealthComponent.h"
#include "Components/HunterAbilityComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"

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

TArray<FHunterEquipmentRecommendation> UHunterInventoryWidget::GetRecommendedEquipment(int32 MaxResults) const
{
    UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    if (!ArinCharacter || !InventoryComponent || !GameInstance)
    {
        return {};
    }

    UHunterLoadoutRecommendationSubsystem* Recommender =
        GameInstance->GetSubsystem<UHunterLoadoutRecommendationSubsystem>();
    if (!Recommender)
    {
        return {};
    }

    const UArinProgressionComponent* Progression = ArinCharacter->GetProgressionComponent();
    const int32 EvolutionStage = Progression
        ? static_cast<int32>(Progression->GetEvolutionStage()) : 0;
    EHunterPlayerArchetype Archetype = EHunterPlayerArchetype::Balanced;
    if (UHunterPlayerProfileSubsystem* Profile =
        GameInstance->GetSubsystem<UHunterPlayerProfileSubsystem>())
    {
        Archetype = Profile->GetCurrentArchetype();
    }

    TArray<FHunterInventoryItem> EligibleCandidates = InventoryComponent->GetItems();
    // A duplicate stack can remain after equipping one copy. Do not recommend
    // the already-equipped item again when another copy remains in inventory.
    const FHunterEquipmentItem EquippedWeapon = EquipmentComponent
        ? EquipmentComponent->GetWeapon() : FHunterEquipmentItem();
    const FHunterEquipmentItem EquippedArmor = EquipmentComponent
        ? EquipmentComponent->GetArmor() : FHunterEquipmentItem();
    EligibleCandidates.RemoveAll([&EquippedWeapon, &EquippedArmor](const FHunterInventoryItem& Item)
    {
        return (!EquippedWeapon.ItemId.IsNone() && Item.ItemId == EquippedWeapon.ItemId) ||
            (!EquippedArmor.ItemId.IsNone() && Item.ItemId == EquippedArmor.ItemId);
    });

    return Recommender->RecommendEquipment(
        EligibleCandidates, EvolutionStage, Archetype, MaxResults);
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
            UHorizontalBox* ItemRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
            UVerticalBoxSlot* RowSlot = NativeStack->AddChildToVerticalBox(ItemRow);
            RowSlot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 4.0f));

            UTextBlock* ItemLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
            ItemLabel->SetText(FText::FromString(FString::Printf(TEXT("%s   x%d"), *ItemName, Entry.Item.Quantity)));
            FSlateFontInfo ItemFont = ItemLabel->GetFont();
            ItemFont.Size = 12;
            ItemLabel->SetFont(ItemFont);
            ItemLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.88f, 0.91f, 0.97f, 1.0f)));
            UHorizontalBoxSlot* LabelSlot = ItemRow->AddChildToHorizontalBox(ItemLabel);
            LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

            if (Entry.Item.ItemType == EHunterItemType::Weapon || Entry.Item.ItemType == EHunterItemType::Armor)
            {
                UButton* EquipButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
                UTextBlock* EquipLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                EquipLabel->SetText(FText::FromString(TEXT("EQUIP")));
                EquipButton->SetContent(EquipLabel);
                EquipButton->OnClicked.AddLambda([this, ItemId = Entry.Item.ItemId]() { EquipItemById(ItemId); });
                ItemRow->AddChildToHorizontalBox(EquipButton);
            }
            else if (Entry.Item.ItemType == EHunterItemType::Consumable)
            {
                UButton* UseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
                UTextBlock* UseLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                UseLabel->SetText(FText::FromString(TEXT("USE")));
                UseButton->SetContent(UseLabel);
                UseButton->SetIsEnabled(Entry.bCanUseConsumable);
                UseButton->OnClicked.AddLambda([this, ItemId = Entry.Item.ItemId]() { UseConsumableById(ItemId); });
                ItemRow->AddChildToHorizontalBox(UseButton);
            }
        }
    }
    AddLine(FText::FromString(TEXT("AI LOADOUT RECOMMENDATIONS")), 13, FLinearColor(0.35f, 0.72f, 1.0f, 1.0f));
    const TArray<FHunterEquipmentRecommendation> Recommendations = GetRecommendedEquipment(3);
    if (Recommendations.IsEmpty())
    {
        AddLine(FText::FromString(TEXT("No eligible equipment to recommend.")), 11,
            FLinearColor(0.65f, 0.7f, 0.78f, 1.0f));
    }
    else
    {
        for (const FHunterEquipmentRecommendation& Recommendation : Recommendations)
        {
            const FString RecommendedName = Recommendation.Item.DisplayName.IsEmpty()
                ? Recommendation.Item.ItemId.ToString() : Recommendation.Item.DisplayName.ToString();
            UHorizontalBox* RecommendationRow =
                WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
            NativeStack->AddChildToVerticalBox(RecommendationRow);
            UTextBlock* RecommendationLabel =
                WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
            RecommendationLabel->SetText(FText::FromString(FString::Printf(
                TEXT("%s  //  SCORE %.1f"), *RecommendedName, Recommendation.Score)));
            RecommendationRow->AddChildToHorizontalBox(RecommendationLabel);
            UButton* RecommendationEquip =
                WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
            UTextBlock* RecommendationEquipLabel =
                WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
            RecommendationEquipLabel->SetText(FText::FromString(TEXT("EQUIP")));
            RecommendationEquip->SetContent(RecommendationEquipLabel);
            RecommendationEquip->OnClicked.AddLambda(
                [this, ItemId = Recommendation.Item.ItemId]() { EquipItemById(ItemId); });
            RecommendationRow->AddChildToHorizontalBox(RecommendationEquip);
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
