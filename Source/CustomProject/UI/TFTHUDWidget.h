#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Combat/CombatSubsystem.h"
#include "TFTHUDWidget.generated.h"

class UButton;
class UCombatSubsystem;
class UShopSubsystem;
class ATFTPlayerState;
class UTextBlock;
class UProgressBar;
class UHorizontalBox;
class UShopSlotWidget;
class ATFTGameMode;

// Shows round, phase, timer, gold, level, XP, and the shop.
UCLASS()
class CUSTOMPROJECT_API UTFTHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;
    
    UPROPERTY()
    ATFTGameMode* TFTGameMode;

    // -------------------------------------------------------
    // Widget Bindings 
    // -------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* RoundText;      

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* PhaseText;      // PREP / COMBAT / RESULT

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* GoldText;       

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* LevelText;      

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* XPText;         

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UProgressBar* TimerBar;     

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UHorizontalBox* ShopContainer; 

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UButton* RerollButton;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UButton* BuyXPButton;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UButton* ToggleBoardButton;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* RerollText;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* BuyXPText;

    // -------------------------------------------------------
    // Update Functions — read from TFTGameMode and set widget text
    // -------------------------------------------------------

    void UpdateGold();
    void UpdateLevel();
    void UpdateRound();
    void UpdateTimer();
    void UpdatePhase();

    // Loops ShopContainer children and refreshes each slot widget
    void RefreshShopSlots();

    UFUNCTION()
    void OnRerollClicked();

    UFUNCTION()
    void OnBuyXPClicked();

    UFUNCTION()
    void OnToggleBoardClicked();

private:
    // Subscribes to subsystem delegates so UI auto-updates on events
    void BindDelegates();

    // Pushes current game state to all widgets on first load
    void PushInitialState();

    // -------------------------------------------------------
    // Delegate Handlers — bound in BindDelegates
    // -------------------------------------------------------

    UFUNCTION()
    void HandlePhaseChanged(EGamePhase NewPhase);

    UFUNCTION()
    void HandleRoundChanged(int32 NewRound);

    UFUNCTION()
    void HandleShopRefreshed();

    UFUNCTION()
    void HandleLevelUp(int32 NewLevel, int32 BoardCapacity);
};