#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFTHUDWidget.generated.h"

// Forward declarations
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
    // Names must match exactly in WBP_HUD
    // -------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* RoundText;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* PhaseText;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* TimerText;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* GoldText;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* LevelText;
    
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* XPText;

    // Keep timer as a progress bar:
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UProgressBar* TimerBar;

    // Container holding the two shop slot widgets
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UHorizontalBox* ShopContainer;

    // -------------------------------------------------------
    // Public update functions
    // Called internally and by delegates
    // -------------------------------------------------------

    void UpdateGold();
    void UpdateLevel();
    void UpdateRound();
    void UpdateTimer();
    void UpdatePhase();
    void RefreshShopSlots();

private:
    void BindDelegates();
    void PushInitialState();

    UFUNCTION()
    void HandlePhaseChanged(EGamePhase NewPhase);

    UFUNCTION()
    void HandleRoundChanged(int32 NewRound);

    UFUNCTION()
    void HandleShopRefreshed();

    UFUNCTION()
    void HandleLevelUp(int32 NewLevel, int32 BoardCapacity);
    
    UPROPERTY()
    UCombatSubsystem* CombatSS;

    UPROPERTY()
    UShopSubsystem* ShopSS;

    UPROPERTY()
    ATFTPlayerState* PS;
};