#include "UI/TFTHUDWidget.h"
#include "Player/TFTPlayerState.h"
#include "Combat/CombatSubsystem.h"
#include "Shop/ShopSubsystem.h"

void UTFTHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BindDelegates();

    // Push initial state on startup
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ATFTPlayerState* PS = PC->GetPlayerState<ATFTPlayerState>())
        {
            OnGoldUpdated(PS->Gold);
            OnLevelUpdated(PS->PlayerLevel, PS->CurrentXP, PS->GetXPToNextLevel());
        }
    }

    if (UCombatSubsystem* Combat = GetGameInstance()->GetSubsystem<UCombatSubsystem>())
    {
        OnRoundUpdated(Combat->GetCurrentRound());
        OnPhaseUpdated(Combat->GetCurrentPhase());
    }

    OnShopUpdated();
}

void UTFTHUDWidget::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
    Super::NativeTick(MyGeometry, DeltaTime);

    // Push timer update every frame during prep
    if (UCombatSubsystem* Combat = GetGameInstance()->GetSubsystem<UCombatSubsystem>())
    {
        if (Combat->GetCurrentPhase() == EGamePhase::Prep)
            OnTimerUpdated(Combat->GetPrepTimeRemaining());
    }

    // Push gold every frame so it stays current
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ATFTPlayerState* PS = PC->GetPlayerState<ATFTPlayerState>())
            OnGoldUpdated(PS->Gold);
    }
}

void UTFTHUDWidget::BindDelegates()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI) return;

    if (UCombatSubsystem* Combat = GI->GetSubsystem<UCombatSubsystem>())
    {
        Combat->OnPhaseChanged.AddDynamic(this, &UTFTHUDWidget::HandlePhaseChanged);
        Combat->OnRoundChanged.AddDynamic(this, &UTFTHUDWidget::HandleRoundChanged);
    }

    if (UShopSubsystem* Shop = GI->GetSubsystem<UShopSubsystem>())
        Shop->OnShopRefresh.AddDynamic(this, &UTFTHUDWidget::HandleShopRefreshed);

    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ATFTPlayerState* PS = PC->GetPlayerState<ATFTPlayerState>())
            PS->OnLevelUp.AddDynamic(this, &UTFTHUDWidget::HandleLevelUp);
    }
}

void UTFTHUDWidget::HandlePhaseChanged(EGamePhase NewPhase)
{
    OnPhaseUpdated(NewPhase);
}

void UTFTHUDWidget::HandleRoundChanged(int32 NewRound)
{
    OnRoundUpdated(NewRound);
}

void UTFTHUDWidget::HandleShopRefreshed()
{
    OnShopUpdated();
}

void UTFTHUDWidget::HandleLevelUp(int32 NewLevel, int32 BoardCapacity)
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ATFTPlayerState* PS = PC->GetPlayerState<ATFTPlayerState>())
            OnLevelUpdated(NewLevel, PS->CurrentXP, PS->GetXPToNextLevel());
    }
}