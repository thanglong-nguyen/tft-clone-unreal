#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BoardCellWidget.generated.h"

class ATFTGameMode;
class UImage;
class UOverlay;
class UUnitCard;
class AUnit;
class UDragDropOperation;

// Represents a single cell on the board or bench grid.
// Handles drag and drop — units can be dragged in, out, or to the sell zone.
// bIsBenchCell determines whether drops move units to board or bench.
UCLASS()
class CUSTOMPROJECT_API UBoardCellWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY()
    ATFTGameMode* TFTGameMode;

    // Grid coordinates — used to map this widget to a battlefield cell
    UPROPERTY(BlueprintReadWrite)
    int32 Col = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 Row = 0;

    // If true dropping a unit here sends it to bench instead of board
    UPROPERTY(BlueprintReadWrite)
    bool bIsBenchCell = false;

    // -------------------------------------------------------
    // Widget Bindings — names must match WBP_BoardCell exactly
    // -------------------------------------------------------

    // Tints on hover and when occupied
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UImage* CellBackground;

    // Holds the unit card widget when a unit occupies this cell
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UOverlay* CardSlot;

    // Set in WBP_BoardCell details panel
    UPROPERTY(EditAnywhere, Category="Board")
    TSubclassOf<class UUnitCard> UnitCardClass;

    // -------------------------------------------------------
    // Functions
    // -------------------------------------------------------

    // Spawns a UnitCard into CardSlot and tints the background
    void PlaceUnit(AUnit* Unit);

    // Clears the CardSlot and resets background tint
    void ClearUnit();

    // Tints green on hover, resets on leave
    void SetHighlight(bool bHighlight);

protected:
    virtual bool NativeOnDrop(
        const FGeometry& InGeometry,
        const FDragDropEvent& InDragDropEvent,
        UDragDropOperation* InOperation) override;

    virtual bool NativeOnDragOver(
        const FGeometry& InGeometry,
        const FDragDropEvent& InDragDropEvent,
        UDragDropOperation* InOperation) override;

    virtual void NativeOnDragLeave(
        const FDragDropEvent& InDragDropEvent,
        UDragDropOperation* InOperation) override;
};