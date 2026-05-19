#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnitCard.generated.h"

class UTextBlock;
class UImage;
class AUnit;

UCLASS()
class CUSTOMPROJECT_API UUnitCard : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY()
    AUnit* Unit = nullptr;

    // Whether this card is on the board or bench
    // Affects drag behaviour
    UPROPERTY(BlueprintReadWrite)
    bool bOnBoard = false;

    // Populate card with unit data
    void SetUnit(AUnit* InUnit, bool bIsOnBoard);

    // -------------------------------------------------------
    // Widget Bindings — match WBP_UnitCard exactly
    // -------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* UnitNameText;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* StarText;

    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UTextBlock* CostText;

    // Card background — color changes by rarity/star level
    UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    UImage* CardBackground;

protected:
    // Draggable from both bench and board
    virtual FReply NativeOnMouseButtonDown(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent) override;

    virtual void NativeOnDragDetected(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent,
        UDragDropOperation*& OutOperation) override;
};