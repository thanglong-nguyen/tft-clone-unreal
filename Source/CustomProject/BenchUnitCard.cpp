#include "BenchUnitCard.h"
#include "UnitDragDrop.h"
#include "Units/Unit.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UBenchUnitCard::SetUnit(AUnit* InUnit)
{
	Unit = InUnit;
	if (!Unit) return;

	if (UnitNameText)
		UnitNameText->SetText(FText::FromName(Unit->UnitName));

	if (StarText)
		StarText->SetText(FText::FromString(
			FString::Printf(TEXT("★%d"), Unit->StarLevel)));

	if (CostText && Unit->DataAsset)
		CostText->SetText(FText::FromString(
			FString::Printf(TEXT("%dg"), Unit->DataAsset->Cost)));
}

FReply UBenchUnitCard::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	// Detect drag on left mouse button
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(
			InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}
	return FReply::Unhandled();
}

void UBenchUnitCard::NativeOnDragDetected(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	if (!Unit) return;

	UUnitDragDrop* DragOp = NewObject<UUnitDragDrop>();
	DragOp->DraggedUnit = Unit;
	DragOp->bFromBoard  = false;
	DragOp->DefaultDragVisual = this; // widget follows cursor

	OutOperation = DragOp;
}