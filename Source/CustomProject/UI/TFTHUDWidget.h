#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Combat/CombatSubsystem.h"
#include "TFTHUDWidget.generated.h"

class UCombatSubsystem;
class UShopSubsystem;
class ATFTPlayerState;
class UTextBlock;
class UProgressBar;
class UHorizontalBox;
class UShopSlotWidget;

UCLASS()
class CUSTOMPROJECT_API UTFTHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;

    // -------------------------------------------------------
    // Widget Bindings
    // Variable names must match widget names exactly in WBP_HUD
    // -------------------------------------------------------

    // Shows current round number
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* RoundText;

    // Shows current phase — PREP, COMBAT, RESULT
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* PhaseText;

    // Shows current gold amount
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* GoldText;

    // Shows current player level
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* LevelText;

    // Shows current XP progress e.g. "4 / 10" or "MAX"
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* XPText;

    // Decreasing bar showing time remaining in current phase
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UProgressBar* TimerBar;

    // Horizontal container holding the shop slot widgets
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UHorizontalBox* ShopContainer;

    // -------------------------------------------------------
    // Update Functions
    // Each one reads from cached subsystems and sets widget text
    // -------------------------------------------------------

    void UpdateGold();
    void UpdateLevel();
    void UpdateRound();
    void UpdateTimer();
    void UpdatePhase();

    // Loops ShopContainer children and refreshes each slot widget
    void RefreshShopSlots();

private:
    // Subscribes to subsystem delegates so UI auto-updates on events
    void BindDelegates();

    // Pushes current game state to all widgets on startup
    void PushInitialState();

    // -------------------------------------------------------
    // Delegate Handlers
    // Bound in BindDelegates — fire when subsystems broadcast
    // -------------------------------------------------------

    UFUNCTION()
    void HandlePhaseChanged(EGamePhase NewPhase);

    UFUNCTION()
    void HandleRoundChanged(int32 NewRound);

    UFUNCTION()
    void HandleShopRefreshed();

    UFUNCTION()
    void HandleLevelUp(int32 NewLevel, int32 BoardCapacity);

    // -------------------------------------------------------
    // Cached References
    // Set once in NativeConstruct — avoids repeated lookups
    // -------------------------------------------------------

    UPROPERTY()
    UCombatSubsystem* CombatSS;

    UPROPERTY()
    UShopSubsystem* ShopSS;

    UPROPERTY()
    ATFTPlayerState* PS;
};