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

UCLASS()
class CUSTOMPROJECT_API UBoardCellWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY()
    ATFTGameMode* TFTGameMode = nullptr;

    UPROPERTY(BlueprintReadWrite)
    int32 Col = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 Row = 0;

    // -------------------------------------------------------
    // Widget Bindings
    // -------------------------------------------------------

    // Cell background — tints on hover/occupied
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UImage* CellBackground;

    // Slot that holds the unit card when occupied
    // Empty overlay when no unit
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UOverlay* CardSlot;

    // Class to spawn for unit cards — set in WBP_BoardCell details
    UPROPERTY(EditAnywhere, Category="Board")
    TSubclassOf<class UUnitCard> UnitCardClass;

    // -------------------------------------------------------
    // Functions
    // -------------------------------------------------------

    // Place a unit card in this cell
    void PlaceUnit(AUnit* Unit);

    // Remove the unit card from this cell
    void ClearUnit();

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