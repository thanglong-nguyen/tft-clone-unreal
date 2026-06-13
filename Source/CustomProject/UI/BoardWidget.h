#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BoardWidget.generated.h"

class AUnit;
class USellZoneWidget;
class UButton;
class ATFTGameMode;
class UVerticalBox;
class UUniformGridPanel;
class UBoardCellWidget;

// Full-screen overlay for managing units during prep phase.
// Shows the board grid, bench, synergy panel, and sell zone.
// Toggled by the player via the HUD button.
UCLASS()
class CUSTOMPROJECT_API UBoardWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    
    UPROPERTY()
    ATFTGameMode* TFTGameMode = nullptr;

    // -------------------------------------------------------
    // Widget Bindings 
    // -------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UVerticalBox* SynergyContainer;  // left panel — synergy cards

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UUniformGridPanel* BoardGrid;    // center — 4x4 board cells

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UUniformGridPanel* BenchGrid;    // bottom — bench cells

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    USellZoneWidget* SellZone;       // drop target for selling units

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UButton* ToggleHUDButton;        // closes board, shows HUD

    // -------------------------------------------------------
    // Classes to spawn dynamically — set in WBP_BoardWidget details
    // -------------------------------------------------------

    UPROPERTY(EditAnywhere, Category="Board")
    TSubclassOf<class USynergyCard> SynergyCardClass;

    UPROPERTY(EditAnywhere, Category="Board")
    TSubclassOf<UBoardCellWidget> BoardCellClass;

    UPROPERTY(EditAnywhere, Category="Board")
    TSubclassOf<class UUnitCard> UnitCardClass;

    // -------------------------------------------------------
    // Public Functions
    // -------------------------------------------------------

    // Populates BoardGrid with cell widgets — called once on construct
    void BuildBoardGrid();

    // Rebuilds synergy cards from current trait data
    UFUNCTION(BlueprintCallable)
    void RefreshSynergies();

    // Rebuilds bench grid from player bench units
    UFUNCTION(BlueprintCallable)
    void RefreshBench();

    // Clears the unit card from a specific board cell
    void ClearCell(int32 Col, int32 Row);

    // Returns the cell widget at the given grid coordinates
    UBoardCellWidget* GetCellWidget(int32 Col, int32 Row);

    // Replaces unit cards for all currently occupied board cells
    void RebuildOccupiedCells();

    UFUNCTION()
    void HandleUnitsMerged(int32 Col, int32 Row);

    UFUNCTION()
    void HandleUnitPlaced(AUnit* PlacedUnit);

    UFUNCTION()
    void OnToggleHUDClicked();

private:
    // Binds to shop, trait, and player state delegates
    void BindDelegates();

    UFUNCTION()
    void HandleTraitsUpdated();

    UFUNCTION()
    void HandleUnitTransaction(AUnit* NewUnit);
};