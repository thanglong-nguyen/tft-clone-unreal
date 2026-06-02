#include "UI/SellZoneWidget.h"
#include "UnitDragDrop.h"
#include "TFTGameMode.h"
#include "Shop/ShopSubsystem.h"
#include "Components/Image.h"

bool USellZoneWidget::NativeOnDragOver(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (Cast<UUnitDragDrop>(InOperation))
	{
		// Turn red when hovering with a unit
		if (SellBackground)
			SellBackground->SetColorAndOpacity(
				FLinearColor(1.f, 0.f, 0.f, 0.6f));
		return true;
	}
	return false;
}

void USellZoneWidget::NativeOnDragLeave(
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (SellBackground)
		SellBackground->SetColorAndOpacity(
			FLinearColor(1.f, 0.f, 0.f, 0.2f));
}

bool USellZoneWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	UUnitDragDrop* DragOp = Cast<UUnitDragDrop>(InOperation);
	if (!DragOp || !DragOp->DraggedUnit || !TFTGameMode) return false;
	
	if (TFTGameMode->CombatSS->GetCurrentPhase() != EGamePhase::Prep) return false;

	UShopSubsystem* Shop = TFTGameMode->ShopSS;
	if (!Shop) return false;

	Shop->SellUnit(DragOp->DraggedUnit);
	return true;
}