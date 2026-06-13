#include "UI/BoardCellWidget.h"
#include "UI/UnitCard.h"
#include "UI/BoardWidget.h"
#include "UnitDragDrop.h"
#include "TFTGameMode.h"
#include "Player/TFTPlayerState.h"
#include "Combat/BattlefieldActor.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Traits/TraitSubsystem.h"

void UBoardCellWidget::PlaceUnit(AUnit* Unit)
{
    if (!Unit || !CardSlot || !UnitCardClass) return;

    CardSlot->ClearChildren();

    UUnitCard* Card = CreateWidget<UUnitCard>(this, UnitCardClass);
    if (!Card) return;

    Card->SetUnit(Unit, !bIsBenchCell); // bOnBoard = true for board cells
    CardSlot->AddChild(Card);

    // Blue tint indicates occupied board cell
    if (CellBackground)
        CellBackground->SetColorAndOpacity(FLinearColor(0.2f, 0.4f, 1.f, 0.3f));
}

void UBoardCellWidget::ClearUnit()
{
    if (CardSlot) CardSlot->ClearChildren();

    if (CellBackground)
        CellBackground->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.1f));
}

void UBoardCellWidget::SetHighlight(bool bHighlight)
{
    if (!CellBackground) return;
    CellBackground->SetColorAndOpacity(bHighlight
        ? FLinearColor(0.f, 1.f, 0.f, 0.4f)  // green = valid drop target
        : FLinearColor(1.f, 1.f, 1.f, 0.1f)); // default subtle tint
}

bool UBoardCellWidget::NativeOnDragOver(
    const FGeometry& InGeometry,
    const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation)
{
    if (!Cast<UUnitDragDrop>(InOperation)) return false;

    // Red highlight on board cells during combat — placement is blocked
    if (TFTGameMode &&
        TFTGameMode->CombatSS->GetCurrentPhase() != EGamePhase::Prep &&
        !bIsBenchCell)
    {
        if (CellBackground)
            CellBackground->SetColorAndOpacity(FLinearColor(1.f, 0.f, 0.f, 0.3f));
        return true;
    }

    SetHighlight(true);
    return true;
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

    AUnit* Unit           = DragOp->DraggedUnit;
    ATFTPlayerState* PS   = TFTGameMode->PS;
    ABattlefieldActor* BF = TFTGameMode->Battlefield;

    if (!PS || !BF) return false;

    // Block placement changes outside of prep phase
    if (TFTGameMode->CombatSS->GetCurrentPhase() != EGamePhase::Prep)
    {
        TFTGameMode->ShowMessage(TEXT("Can't move units during Combat"),
            2.f, FLinearColor::Yellow);
        return false;
    }

    // -------------------------------------------------------
    // Bench drop 
    // -------------------------------------------------------
    if (bIsBenchCell)
    {
        // Check bench has a free slot before proceeding
        if (TFTGameMode->GetNextFreeBenchSlot() == -1) return false;

        // Free the board cell if unit came from the board
        if (DragOp->bFromBoard && Unit->GridCol >= 0)
        {
            BF->FreePlayerCell(Unit->GridCol, Unit->GridRow);
            if (TFTGameMode->BoardWidget)
                TFTGameMode->BoardWidget->ClearCell(Unit->GridCol, Unit->GridRow);
        }

        PS->MoveToBench(Unit);
        PlaceUnit(Unit);
        SetHighlight(false);

        if (TFTGameMode->TraitSS)
            TFTGameMode->TraitSS->RecalculateTraits(PS->BoardUnits);

        if (TFTGameMode->BoardWidget)
            TFTGameMode->BoardWidget->RefreshBench();

        return true;
    }

    // -------------------------------------------------------
    // Board drop 
    // -------------------------------------------------------

    if (!BF->IsPlayerCellFree(Col, Row)) return false;

    if (DragOp->bFromBoard && Unit->GridCol >= 0)
    {
        // Moving from one board cell to another — free the old cell
        BF->FreePlayerCell(Unit->GridCol, Unit->GridRow);
        PS->BoardUnits.Remove(Unit);
        if (TFTGameMode->BoardWidget)
            TFTGameMode->BoardWidget->ClearCell(Unit->GridCol, Unit->GridRow);
    }
    else
    {
        // Coming from bench — check board capacity before placing
        if (!PS->CanPlaceOnBoard()) return false;
        PS->BenchUnits.Remove(Unit);
        if (TFTGameMode->BoardWidget)
            TFTGameMode->BoardWidget->RefreshBench();
    }

    // Move unit actor to the cell's world position
    FVector Position = BF->GetPlayerCellPosition(Col, Row);
    Unit->SetActorLocation(Position);
    BF->OccupyPlayerCell(Col, Row);
    Unit->GridCol = Col;
    Unit->GridRow = Row;
    PS->MoveToBoard(Unit);
    PlaceUnit(Unit);
    SetHighlight(false);

    if (TFTGameMode->TraitSS)
        TFTGameMode->TraitSS->RecalculateTraits(PS->BoardUnits);

    return true;
}