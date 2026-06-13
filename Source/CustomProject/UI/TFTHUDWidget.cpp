#include "UI/TFTHUDWidget.h"
#include "TFTGameMode.h"
#include "UI/ShopSlotWidget.h"
#include "Player/TFTPlayerState.h"
#include "Combat/CombatSubsystem.h"
#include "Shop/ShopSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"
#include "UI/BoardWidget.h"

void UTFTHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    BindDelegates();
    PushInitialState();

    if (RerollButton)
    {
        RerollButton->OnClicked.AddDynamic(this, &UTFTHUDWidget::OnRerollClicked);
        RerollText->SetText(FText::FromString("Reroll: 2g"));
    }

    if (BuyXPButton)
    {
        BuyXPButton->OnClicked.AddDynamic(this, &UTFTHUDWidget::OnBuyXPClicked);
        BuyXPText->SetText(FText::FromString("Buy XP: 4g"));
    }

    if (ToggleBoardButton)
        ToggleBoardButton->OnClicked.AddDynamic(
            this, &UTFTHUDWidget::OnToggleBoardClicked);
}

void UTFTHUDWidget::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
    Super::NativeTick(MyGeometry, DeltaTime);

    // Update every frame for smooth timer and gold display
    UpdateGold();
    UpdateTimer();
}

void UTFTHUDWidget::BindDelegates()
{
    if (TFTGameMode->CombatSS)
    {
        TFTGameMode->CombatSS->OnPhaseChanged.AddDynamic(
            this, &UTFTHUDWidget::HandlePhaseChanged);
        TFTGameMode->CombatSS->OnRoundChanged.AddDynamic(
            this, &UTFTHUDWidget::HandleRoundChanged);
    }

    if (TFTGameMode->ShopSS)
        TFTGameMode->ShopSS->OnShopRefresh.AddDynamic(
            this, &UTFTHUDWidget::HandleShopRefreshed);

    if (TFTGameMode->PS)
        TFTGameMode->PS->OnLevelUp.AddDynamic(
            this, &UTFTHUDWidget::HandleLevelUp);
}

void UTFTHUDWidget::PushInitialState()
{
    // Push current values immediately so UI is correct before any events fire
    UpdateGold();
    UpdateLevel();
    UpdateRound();
    UpdatePhase();
    UpdateTimer();
    RefreshShopSlots();
}

// -------------------------------------------------------
// Update Functions
// -------------------------------------------------------

void UTFTHUDWidget::UpdateGold()
{
    if (!TFTGameMode->PS || !GoldText) return;
    GoldText->SetText(FText::FromString(
        FString::Printf(TEXT("Gold: %d"), TFTGameMode->PS->Gold)));
}

void UTFTHUDWidget::UpdateLevel()
{
    if (!TFTGameMode->PS || !LevelText || !XPText) return;

    LevelText->SetText(FText::FromString(
        FString::Printf(TEXT("Level: %d"), TFTGameMode->PS->PlayerLevel)));

    // Show MAX when player reaches level cap
    if (TFTGameMode->PS->PlayerLevel >= 8)
        XPText->SetText(FText::FromString(TEXT("MAX")));
    else
        XPText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"),
            TFTGameMode->PS->CurrentXP,
            TFTGameMode->PS->GetXPToNextLevel())));
}

void UTFTHUDWidget::UpdateRound()
{
    if (!TFTGameMode->CombatSS || !RoundText) return;
    RoundText->SetText(FText::FromString(
        FString::Printf(TEXT("Round: %d"), TFTGameMode->CombatSS->GetCurrentRound())));
}

void UTFTHUDWidget::UpdateTimer()
{
    if (!TFTGameMode->CombatSS || !TimerBar) return;

    float Percent = 0.f;
    switch (TFTGameMode->CombatSS->GetCurrentPhase())
    {
    case EGamePhase::Prep:
        Percent = TFTGameMode->CombatSS->GetPrepTimeRemaining()
                / TFTGameMode->CombatSS->PrepDuration;
        break;
    case EGamePhase::Combat:
        Percent = TFTGameMode->CombatSS->GetCombatTimeRemaining()
                / TFTGameMode->CombatSS->CombatDuration;
        break;
    case EGamePhase::Result:
        Percent = TFTGameMode->CombatSS->GetResultTimeRemaining()
                / TFTGameMode->CombatSS->ResultDuration;
        break;
    }

    TimerBar->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
}

void UTFTHUDWidget::UpdatePhase()
{
    if (!TFTGameMode->CombatSS || !PhaseText) return;

    FString PhaseStr;
    switch (TFTGameMode->CombatSS->GetCurrentPhase())
    {
    case EGamePhase::Prep:   PhaseStr = TEXT("PREP");   break;
    case EGamePhase::Combat: PhaseStr = TEXT("COMBAT"); break;
    case EGamePhase::Result: PhaseStr = TEXT("RESULT"); break;
    }
    PhaseText->SetText(FText::FromString(PhaseStr));
}

void UTFTHUDWidget::RefreshShopSlots()
{
    if (!TFTGameMode->ShopSS || !ShopContainer) return;

    int32 SlotCount = ShopContainer->GetChildrenCount();
    const TArray<FShopSlot>& CurrentShop = TFTGameMode->ShopSS->CurrentShop;

    for (int32 i = 0; i < SlotCount; ++i)
    {
        UShopSlotWidget* SlotWidget = Cast<UShopSlotWidget>(ShopContainer->GetChildAt(i));
        if (!SlotWidget) continue;

        // Pass empty data if slot has no unit
        FName  UnitName  = CurrentShop.IsValidIndex(i) && CurrentShop[i].Data
                         ? CurrentShop[i].Data->UnitName : NAME_None;
        int32  Cost      = CurrentShop.IsValidIndex(i) && CurrentShop[i].Data
                         ? CurrentShop[i].Data->Cost : 0;
        bool   Purchased = CurrentShop.IsValidIndex(i)
                         ? CurrentShop[i].bIsPurchased : false;

        SlotWidget->RefreshSlot(UnitName, Cost, Purchased);
    }
}

// -------------------------------------------------------
// Button Handlers
// -------------------------------------------------------

void UTFTHUDWidget::OnRerollClicked()
{
    if (!TFTGameMode->ShopSS || !TFTGameMode->PS) return;
    TFTGameMode->ShopSS->RefreshShop(TFTGameMode->PS->PlayerLevel);
}

void UTFTHUDWidget::OnBuyXPClicked()
{
    if (!TFTGameMode->PS) return;
    TFTGameMode->PS->BuyXP();
    UpdateLevel();
    UpdateGold();
}

void UTFTHUDWidget::OnToggleBoardClicked()
{
    if (!TFTGameMode || !TFTGameMode->BoardWidget) return;

    // Hide HUD and show board widget
    this->SetVisibility(ESlateVisibility::Hidden);
    TFTGameMode->BoardWidget->SetVisibility(ESlateVisibility::Visible);
}

// -------------------------------------------------------
// Delegate Handlers
// -------------------------------------------------------

void UTFTHUDWidget::HandlePhaseChanged(EGamePhase NewPhase)
{
    UpdatePhase();
    UpdateTimer();
    if (ToggleBoardButton)
        ToggleBoardButton->SetIsEnabled(true);
}

void UTFTHUDWidget::HandleRoundChanged(int32 NewRound)
{
    UpdateRound();
}

void UTFTHUDWidget::HandleShopRefreshed()
{
    RefreshShopSlots();
}

void UTFTHUDWidget::HandleLevelUp(int32 NewLevel, int32 BoardCapacity)
{
    UpdateLevel();
}