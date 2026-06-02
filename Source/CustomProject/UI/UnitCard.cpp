#include "UI/UnitCard.h"
#include "UnitDragDrop.h"
#include "Units/Unit.h"
#include "Units/UnitDataAsset.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UUnitCard::SetUnit(AUnit* InUnit, bool bIsOnBoard)
{
    Unit     = InUnit;
    bOnBoard = bIsOnBoard;

    if (!Unit) return;

    if (UnitNameText)
        UnitNameText->SetText(FText::FromName(Unit->UnitName));

    if (StarText)
        StarText->SetText(FText::FromString(
            FString::Printf(TEXT("★%d"), Unit->StarLevel)));

    if (CostText && Unit->DataAsset)
    {
        // Cost scales with star level — same formula as sell refund
        int32 Copies = FMath::RoundToInt(FMath::Pow(3.f, Unit->StarLevel - 1));
        
        int32 TotalCost = Unit->DataAsset->Cost * Copies;
        
        CostText->SetText(FText::FromString(
            FString::Printf(TEXT("%dg"), TotalCost)));
    }

    // Tint card background by star level
    if (CardBackground)
    {
        FLinearColor Color;
        switch (Unit->StarLevel)
        {
        case 1: Color = FLinearColor(0.8f, 0.8f, 0.8f, 1.f); break; // grey
        case 2: Color = FLinearColor(0.2f, 0.6f, 1.f,  1.f); break; // blue
        case 3: Color = FLinearColor(1.f,  0.8f, 0.f,  1.f); break; // gold
        default: Color = FLinearColor::White; break;
        }
        CardBackground->SetColorAndOpacity(Color);
    }
}

FReply UUnitCard::NativeOnMouseButtonDown(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        return UWidgetBlueprintLibrary::DetectDragIfPressed(
            InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
    }
    return FReply::Unhandled();
}

void UUnitCard::NativeOnDragDetected(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent,
    UDragDropOperation*& OutOperation)
{
    if (!Unit) return;

    UUnitDragDrop* DragOp = NewObject<UUnitDragDrop>();
    DragOp->DraggedUnit     = Unit;
    DragOp->bFromBoard      = bOnBoard;
    DragOp->DefaultDragVisual = this; // card follows cursor while dragging

    OutOperation = DragOp;
}