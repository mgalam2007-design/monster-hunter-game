#include "HunterHUDWidget.h"

#include "ArinValeCharacter.h"
#include "Components/HunterHealthComponent.h"
#include "Components/HunterAbilityComponent.h"
#include "Components/HunterCurrencyComponent.h"
#include "ArinProgressionComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UHunterHUDWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BuildNativeLayout();
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

    ApplyNativeLayout();
    BP_OnHUDSnapshotUpdated(Snapshot);
}

void UHunterHUDWidget::BuildNativeLayout()
{
    if (!WidgetTree || WidgetTree->RootWidget)
    {
        return;
    }

    NativeRoot = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("NativeHUDRoot"));
    WidgetTree->RootWidget = NativeRoot;

    UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("HUDPanel"));
    Panel->SetBrushColor(FLinearColor(0.015f, 0.025f, 0.045f, 0.88f));
    Panel->SetPadding(FMargin(16.0f));
    UCanvasPanelSlot* PanelSlot = NativeRoot->AddChildToCanvas(Panel);
    PanelSlot->SetPosition(FVector2D(28.0f, 28.0f));
    PanelSlot->SetSize(FVector2D(340.0f, 220.0f));

    UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("HUDStack"));
    Panel->SetContent(Stack);

    auto AddText = [this, Stack](const FName Name, const FText& Initial, int32 FontSize)
    {
        UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
        Text->SetText(Initial);
        FSlateFontInfo Font = Text->GetFont();
        Font.Size = FontSize;
        Text->SetFont(Font);
        Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.93f, 1.0f, 1.0f)));
        Stack->AddChildToVerticalBox(Text);
        return Text;
    };
    auto AddBar = [this, Stack](const FName Name, const FLinearColor FillColor)
    {
        UProgressBar* Bar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), Name);
        Bar->SetFillColorAndOpacity(FillColor);
        Bar->SetPercent(1.0f);
        UVerticalBoxSlot* Slot = Stack->AddChildToVerticalBox(Bar);
        Slot->SetPadding(FMargin(0.0f, 3.0f, 0.0f, 7.0f));
        return Bar;
    };

    StageText = AddText(TEXT("EvolutionStage"), FText::FromString(TEXT("HUNTER // AWAKENING")), 18);
    HealthText = AddText(TEXT("HealthLabel"), FText::FromString(TEXT("HP 100 / 100")), 12);
    HealthBar = AddBar(TEXT("HealthBar"), FLinearColor(0.82f, 0.12f, 0.16f, 1.0f));
    EnergyText = AddText(TEXT("EnergyLabel"), FText::FromString(TEXT("ENERGY 100 / 100")), 12);
    EnergyBar = AddBar(TEXT("EnergyBar"), FLinearColor(0.08f, 0.48f, 0.95f, 1.0f));
    ExperienceText = AddText(TEXT("ExperienceLabel"), FText::FromString(TEXT("EXPERIENCE")), 12);
    ExperienceBar = AddBar(TEXT("ExperienceBar"), FLinearColor(0.9f, 0.62f, 0.12f, 1.0f));
    CurrencyText = AddText(TEXT("CurrencyLabel"), FText::FromString(TEXT("GOLD 0")), 13);
}

void UHunterHUDWidget::ApplyNativeLayout()
{
    if (StageText)
    {
        StageText->SetText(FText::Format(NSLOCTEXT("HunterHUD", "StageFormat", "HUNTER // {0}"), Snapshot.EvolutionStageName));
    }
    if (HealthText)
    {
        HealthText->SetText(FText::FromString(FString::Printf(TEXT("HP  %.0f / %.0f"), Snapshot.Health, Snapshot.MaxHealth)));
    }
    if (HealthBar) HealthBar->SetPercent(Snapshot.HealthPercent);
    if (EnergyText)
    {
        EnergyText->SetText(FText::FromString(FString::Printf(TEXT("ENERGY  %.0f / %.0f"), Snapshot.Energy, Snapshot.MaxEnergy)));
    }
    if (EnergyBar) EnergyBar->SetPercent(Snapshot.EnergyPercent);
    if (ExperienceText)
    {
        ExperienceText->SetText(FText::FromString(FString::Printf(TEXT("XP  %d / %d"), Snapshot.Experience, Snapshot.ExperienceForNextStage)));
    }
    if (ExperienceBar) ExperienceBar->SetPercent(Snapshot.ExperiencePercent);
    if (CurrencyText)
    {
        CurrencyText->SetText(FText::FromString(FString::Printf(TEXT("GOLD  %lld"), static_cast<long long>(Snapshot.Currency))));
    }
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
