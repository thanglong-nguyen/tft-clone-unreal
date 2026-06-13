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
	if (!Cast<UUnitDragDrop>(InOperation)) return false;

	// Brighten to signal this is a valid drop target
	if (SellBackground)
		SellBackground->SetColorAndOpacity(FLinearColor(1.f, 0.f, 0.f, 0.6f));
	return true;
}

void USellZoneWidget::NativeOnDragLeave(
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	// Dim back to resting state
	if (SellBackground)
		SellBackground->SetColorAndOpacity(FLinearColor(1.f, 0.f, 0.f, 0.2f));
}

bool USellZoneWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	UUnitDragDrop* DragOp = Cast<UUnitDragDrop>(InOperation);
	if (!DragOp || !DragOp->DraggedUnit || !TFTGameMode) return false;

	// Prevent selling units that are actively fighting on the battlefield
	if (TFTGameMode->CombatSS->GetCurrentPhase() != EGamePhase::Prep
		&& DragOp->DraggedUnit->GridCol >= 0)
	{
		TFTGameMode->ShowMessage(TEXT("Can't sell combating units"),
			2.f, FLinearColor::Yellow);
		return false;
	}

	UShopSubsystem* Shop = TFTGameMode->ShopSS;
	if (!Shop) return false;

	Shop->SellUnit(DragOp->DraggedUnit);
	return true;
}