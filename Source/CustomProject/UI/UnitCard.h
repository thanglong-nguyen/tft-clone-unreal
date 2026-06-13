#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnitCard.generated.h"

class UTextBlock;
class UImage;
class AUnit;

// Draggable card widget representing a single unit.
// Used in both board cells and bench cells.
// Initiates a UUnitDragDrop operation on left mouse drag.
UCLASS()
class CUSTOMPROJECT_API UUnitCard : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY()
    AUnit* Unit = nullptr;

    // True if this card is sitting on the board — affects drag drop behaviour
    UPROPERTY(BlueprintReadWrite)
    bool bOnBoard = false;

    // Populates all text and tints the background based on star level
    void SetUnit(AUnit* InUnit, bool bIsOnBoard);

    // -------------------------------------------------------
    // Widget Bindings 
    // -------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* UnitNameText;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* StarText;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* CostText;   

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UImage* CardBackground; // grey=1★, blue=2★, gold=3★

protected:
    virtual FReply NativeOnMouseButtonDown(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent) override;

    virtual void NativeOnDragDetected(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent,
        UDragDropOperation*& OutOperation) override;
};