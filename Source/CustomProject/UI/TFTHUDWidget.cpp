#include "UI/TFTHUDWidget.h"
#include "UI/ShopSlotWidget.h"
#include "Player/TFTPlayerState.h"
#include "Combat/CombatSubsystem.h"
#include "Shop/ShopSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/HorizontalBox.h"

void UTFTHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // TODO: Call BindDelegates
    // TODO: Call PushInitialState
    
    UGameInstance* GI = GetGameInstance();
    CombatSS = GI->GetSubsystem<UCombatSubsystem>();
    ShopSS   = GI->GetSubsystem<UShopSubsystem>();

    if (APlayerController* PC = GetOwningPlayer())
        PS = PC->GetPlayerState<ATFTPlayerState>(); 
    
    BindDelegates();
    PushInitialState();
}

void UTFTHUDWidget::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
    Super::NativeTick(MyGeometry, DeltaTime);
    // TODO: Call UpdateGold every tick
    // TODO: Call UpdateTimer only if phase is Prep
    UpdateGold();
    
    if (CombatSS && CombatSS->GetCurrentPhase() == EGamePhase::Prep)
    {
        UpdateTimer();
    }
}

void UTFTHUDWidget::BindDelegates()
{
    // TODO: Get GameInstance
    // TODO: Bind CombatSubsystem delegates
    // TODO: Bind ShopSubsystem delegates  
    // TODO: Bind PlayerState OnLevelUp
    
    if (CombatSS)
    {
        CombatSS->OnPhaseChanged.AddDynamic(this, &UTFTHUDWidget::HandlePhaseChanged);
        CombatSS->OnRoundChanged.AddDynamic(this, &UTFTHUDWidget::HandleRoundChanged);
    }
    
    if (ShopSS)
    {
        ShopSS->OnShopRefresh.AddDynamic(this, &UTFTHUDWidget::HandleShopRefreshed);
    }
    
    if (PS)
    {
        PS->OnLevelUp.AddDynamic(this, &UTFTHUDWidget::HandleLevelUp);
    }
        
    
}

void UTFTHUDWidget::PushInitialState()
{
    // TODO: Call all update functions once on startup
    
    UpdateGold();
    UpdateLevel();
    UpdateRound();
    UpdatePhase();
    UpdateTimer();
    RefreshShopSlots();
}

void UTFTHUDWidget::UpdateGold()
{
    // TODO: Get PlayerState
    // TODO: Set GoldText using FText::FromString
    // HINT: FString::Printf(TEXT("Gold: %d"), PS->Gold)
    // HINT: GoldText->SetText(FText::FromString(...))
    

    if (!PS || !GoldText) return; 
    
    FString Gold = FString::Printf(TEXT("Gold: %d"), PS->Gold);
    GoldText->SetText(FText::FromString(Gold));

     
    
}

void UTFTHUDWidget::UpdateLevel()
{
    // TODO: Get PlayerState
    // TODO: Set LevelText — format as "Level: {Level}"
    // TODO: Set XPText — format as "{CurrentXP} / {XPToNext}"
    // HINT: If PlayerLevel == MaxLevel show "MAX" instead
    
    if (!PS || !LevelText || !XPText) return; 
    

    FString Level = FString::Printf(TEXT("Level: %d"), PS->PlayerLevel);
    LevelText->SetText(FText::FromString(Level));
    
    if (PS->PlayerLevel >= 8)
    {
        XPText->SetText(FText::FromString(TEXT("MAX")));
    }
    else
    {
        FString XP = FString::Printf(TEXT("%d / %d"), PS->CurrentXP, PS->GetXPToNextLevel());
        XPText->SetText(FText::FromString(XP));
    } 

    
}

void UTFTHUDWidget::UpdateRound()
{
    // TODO: Get CombatSubsystem
    // TODO: Set RoundText
    
    if (!CombatSS || !RoundText) return; 
    
    FString Round = FString::Printf(TEXT("Round: %d"), CombatSS->GetCurrentRound());
    
    RoundText->SetText(FText::FromString(Round));
    
}

void UTFTHUDWidget::UpdateTimer()
{
    if (!CombatSS || !TimerBar) return;

    float Percent = 0.f;

    switch (CombatSS->GetCurrentPhase())
    {
    case EGamePhase::Prep:
        // Counts down from full to empty
        Percent = CombatSS->GetPrepTimeRemaining() / CombatSS->PrepDuration;
        break;

    case EGamePhase::Combat:
        // Keep bar full during combat — no timer
        Percent = CombatSS->GetCombatTimeRemaining() / CombatSS->CombatDuration;
        break;

    case EGamePhase::Result:
        // 3 second result screen — you'd need to track this too
        // For now just show empty
        Percent = CombatSS->GetResultTimeRemaining() / CombatSS->ResultDuration;
        break; 
    }

    TimerBar->SetPercent(Percent);
}

void UTFTHUDWidget::UpdatePhase()
{
    // TODO: Get CombatSubsystem
    // TODO: Switch on current phase
    // TODO: Set PhaseText to "PREP", "COMBAT" or "RESULT"
    
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
    // TODO: Get ShopSubsystem
    // TODO: Loop ShopContainer children
    // TODO: Cast each to UShopSlotWidget
    // TODO: Call RefreshSlot on each with matching data
    // HINT: ShopContainer->GetChildrenCount()
    // HINT: ShopContainer->GetChildAt(i)
    
    if (!ShopSS || !ShopContainer) return;
    
    int32 Slots = ShopContainer->GetChildrenCount();

    for (int i = 0; i < Slots; ++i)
    {
        UShopSlotWidget* ShopSlot = Cast<UShopSlotWidget>(ShopContainer->GetChildAt(i));
        if (ShopSlot)
        {
            // TODO: call RefreshSlot once ShopSlotWidget is implemented
        }
    }
    
    
}

void UTFTHUDWidget::HandlePhaseChanged(EGamePhase NewPhase)
{
    UpdatePhase();
    UpdateTimer(); 
}

void UTFTHUDWidget::HandleRoundChanged(int32 NewRound)
{
    // TODO: Call UpdateRound
    
    UpdateRound(); 
}

void UTFTHUDWidget::HandleShopRefreshed()
{
    // TODO: Call RefreshShopSlots
    
    RefreshShopSlots();
}

void UTFTHUDWidget::HandleLevelUp(int32 NewLevel, int32 BoardCapacity)
{
    // TODO: Call UpdateLevel
    
    UpdateLevel();
}