#include "UI/BoardCellWidget.h"
#include "UnitDragDrop.h"
#include "TFTGameMode.h"
#include "Player/TFTPlayerState.h"
#include "Combat/BattlefieldActor.h"
#include "Components/Image.h"

void UBoardCellWidget::SetHighlight(bool bHighlight)
{
	if (!CellBackground) return;
	CellBackground->SetColorAndOpacity(bHighlight
		? FLinearColor(0.f, 1.f, 0.f, 0.4f)  // green highlight
		: FLinearColor(1.f, 1.f, 1.f, 0.1f)); // default subtle
}

bool UBoardCellWidget::NativeOnDragOver(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (Cast<UUnitDragDrop>(InOperation))
	{
		SetHighlight(true);
		return true;
	}
	return false;
}

void UBoardCellWidget::NativeOnDragLeave(
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	SetHighlight(false);
}

bool UBoardCellWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	UUnitDragDrop* DragOp = Cast<UUnitDragDrop>(InOperation);
	if (!DragOp || !DragOp->DraggedUnit || !TFTGameMode) return false;

	AUnit* Unit = DragOp->DraggedUnit;
	ATFTPlayerState* PS = TFTGameMode->PS;
	ABattlefieldActor* Battlefield = TFTGameMode->Battlefield;

	if (!PS || !Battlefield) return false;

	// Free old cell if coming from board
	if (DragOp->bFromBoard && Unit->GridCol >= 0)
	{
		Battlefield->FreePlayerCell(Unit->GridCol, Unit->GridRow);
		PS->BoardUnits.Remove(Unit);
	}

	// Place on new cell
	if (Battlefield->IsPlayerCellFree(Col, Row))
	{
		FVector Position = Battlefield->GetPlayerCellPosition(Col, Row);
		Unit->SetActorLocation(Position);
		Battlefield->OccupyPlayerCell(Col, Row);
		Unit->GridCol = Col;
		Unit->GridRow = Row;
		PS->MoveToBoard(Unit);

		SetHighlight(false);
		return true;
	}

	return false;
}