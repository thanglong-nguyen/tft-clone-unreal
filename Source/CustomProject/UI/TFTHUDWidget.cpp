#include "UI/TFTHUDWidget.h"
#include "UI/ShopSlotWidget.h"
#include "Player/TFTPlayerState.h"
#include "Combat/CombatSubsystem.h"
#include "Shop/ShopSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"

void UTFTHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Cache subsystem and player state references once
    // All update functions use these cached pointers
    UGameInstance* GI = GetGameInstance();
    CombatSS = GI->GetSubsystem<UCombatSubsystem>();
    ShopSS   = GI->GetSubsystem<UShopSubsystem>();

    if (APlayerController* PC = GetOwningPlayer())
        PS = PC->GetPlayerState<ATFTPlayerState>();

    // Bind after caching so delegates have valid pointers
    BindDelegates();

    // Push current game state to all widgets immediately
    PushInitialState();
    
    if (RerollButton)
        RerollButton->OnClicked.AddDynamic(this, &UTFTHUDWidget::OnRerollClicked);

    if (BuyXPButton)
        BuyXPButton->OnClicked.AddDynamic(this, &UTFTHUDWidget::OnBuyXPClicked);
}

void UTFTHUDWidget::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
    Super::NativeTick(MyGeometry, DeltaTime);

    // Gold and timer update every frame for smooth display
    UpdateGold();
    UpdateTimer();
}

void UTFTHUDWidget::BindDelegates()
{
    // Subscribe to combat events
    if (CombatSS)
    {
        CombatSS->OnPhaseChanged.AddDynamic(this, &UTFTHUDWidget::HandlePhaseChanged);
        CombatSS->OnRoundChanged.AddDynamic(this, &UTFTHUDWidget::HandleRoundChanged);
    }

    // Subscribe to shop events
    if (ShopSS)
        ShopSS->OnShopRefresh.AddDynamic(this, &UTFTHUDWidget::HandleShopRefreshed);

    // Subscribe to player level up events
    if (PS)
        PS->OnLevelUp.AddDynamic(this, &UTFTHUDWidget::HandleLevelUp);
}

void UTFTHUDWidget::PushInitialState()
{
    // Push all current values to widgets on startup
    // Ensures UI shows correct state before any events fire
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
    if (!PS || !GoldText) return;
    GoldText->SetText(FText::FromString(
        FString::Printf(TEXT("Gold: %d"), PS->Gold)));
}

void UTFTHUDWidget::UpdateLevel()
{
    if (!PS || !LevelText || !XPText) return;

    LevelText->SetText(FText::FromString(
        FString::Printf(TEXT("Level: %d"), PS->PlayerLevel)));

    // Show MAX when player reaches level cap
    if (PS->PlayerLevel >= 8)
        XPText->SetText(FText::FromString(TEXT("MAX")));
    else
        XPText->SetText(FText::FromString(
            FString::Printf(TEXT("%d / %d"), PS->CurrentXP, PS->GetXPToNextLevel())));
}

void UTFTHUDWidget::UpdateRound()
{
    if (!CombatSS || !RoundText) return;
    RoundText->SetText(FText::FromString(
        FString::Printf(TEXT("Round: %d"), CombatSS->GetCurrentRound())));
}

void UTFTHUDWidget::UpdateTimer()
{
    if (!CombatSS || !TimerBar) return;

    float Percent = 0.f;

    // Each phase has its own duration — bar fills or drains accordingly
    switch (CombatSS->GetCurrentPhase())
    {
    case EGamePhase::Prep:
        Percent = CombatSS->GetPrepTimeRemaining() / CombatSS->PrepDuration;
        break;
    case EGamePhase::Combat:
        Percent = CombatSS->GetCombatTimeRemaining() / CombatSS->CombatDuration;
        break;
    case EGamePhase::Result:
        Percent = CombatSS->GetResultTimeRemaining() / CombatSS->ResultDuration;
        break;
    }

    TimerBar->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
}

void UTFTHUDWidget::UpdatePhase()
{
    if (!CombatSS || !PhaseText) return;

    FString PhaseStr;
    switch (CombatSS->GetCurrentPhase())
    {
    case EGamePhase::Prep:   PhaseStr = TEXT("PREP");   break;
    case EGamePhase::Combat: PhaseStr = TEXT("COMBAT"); break;
    case EGamePhase::Result: PhaseStr = TEXT("RESULT"); break;
    }

    PhaseText->SetText(FText::FromString(PhaseStr));
}

void UTFTHUDWidget::RefreshShopSlots()
{
    if (!ShopSS || !ShopContainer) return;

    int32 SlotCount = ShopContainer->GetChildrenCount();
    const TArray<FShopSlot>& CurrentShop = ShopSS->CurrentShop;

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

void UTFTHUDWidget::OnRerollClicked()
{
    if (!ShopSS || !PS) return;
    ShopSS->RefreshShop(PS->PlayerLevel);
}

void UTFTHUDWidget::OnBuyXPClicked()
{
    if (!PS) return;
    PS->BuyXP();
    UpdateLevel();
    UpdateGold();
}

// -------------------------------------------------------
// Delegate Handlers
// -------------------------------------------------------

void UTFTHUDWidget::HandlePhaseChanged(EGamePhase NewPhase)
{
    UpdatePhase();
    UpdateTimer();
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