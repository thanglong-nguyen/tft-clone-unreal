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

    // Clear any existing card
    CardSlot->ClearChildren();

    // Create and add the unit card
    UUnitCard* Card = CreateWidget<UUnitCard>(this, UnitCardClass);
    if (!Card) return;

    Card->SetUnit(Unit, true); // true = on board
    CardSlot->AddChild(Card);

    // Blue tint when occupied
    if (CellBackground)
        CellBackground->SetColorAndOpacity(
            FLinearColor(0.2f, 0.4f, 1.f, 0.3f));
}

void UBoardCellWidget::ClearUnit()
{
    if (CardSlot) CardSlot->ClearChildren();

    if (CellBackground)
        CellBackground->SetColorAndOpacity(
            FLinearColor(1.f, 1.f, 1.f, 0.1f));
}

void UBoardCellWidget::SetHighlight(bool bHighlight)
{
    if (!CellBackground) return;

    CellBackground->SetColorAndOpacity(bHighlight
        ? FLinearColor(0.f, 1.f, 0.f, 0.4f)
        : FLinearColor(1.f, 1.f, 1.f, 0.1f));
}

bool UBoardCellWidget::NativeOnDragOver(
    const FGeometry& InGeometry,
    const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation)
{
    if (!Cast<UUnitDragDrop>(InOperation)) return false;

    // Red highlight during combat — can't place
    if (TFTGameMode && 
        TFTGameMode->CombatSS->GetCurrentPhase() != EGamePhase::Prep)
    {
        if (CellBackground)
            CellBackground->SetColorAndOpacity(
                FLinearColor(1.f, 0.f, 0.f, 0.3f));
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
    
    AUnit* Unit             = DragOp->DraggedUnit;
    ATFTPlayerState* PS     = TFTGameMode->PS;
    ABattlefieldActor* BF   = TFTGameMode->Battlefield;

    if (!PS || !BF) return false;
    if (!BF->IsPlayerCellFree(Col, Row)) return false;
    

    // Free old cell if coming from board
    if (DragOp->bFromBoard && Unit->GridCol >= 0)
    {
        BF->FreePlayerCell(Unit->GridCol, Unit->GridRow);
        PS->BoardUnits.Remove(Unit);

        if (TFTGameMode->BoardWidget)
            TFTGameMode->BoardWidget->ClearCell(Unit->GridCol, Unit->GridRow);
    }
    else
    {
        // Coming from bench
        if (!PS->CanPlaceOnBoard()) return false;
        PS->BenchUnits.Remove(Unit);
        if (TFTGameMode->BoardWidget)
            TFTGameMode->BoardWidget->RefreshBench();
    }

    // Place unit on new cell
    FVector Position = BF->GetPlayerCellPosition(Col, Row);
    Unit->SetActorLocation(Position);
    BF->OccupyPlayerCell(Col, Row);
    Unit->GridCol = Col;
    Unit->GridRow = Row;
    PS->MoveToBoard(Unit);

    // Show card in this cell
    PlaceUnit(Unit);

    // Update synergies
    if (TFTGameMode->ShopSS)
    {
        UTraitSubsystem* Traits = GetGameInstance()
            ->GetSubsystem<UTraitSubsystem>();
        if (Traits)
            Traits->RecalculateTraits(PS->BoardUnits);
    }

    SetHighlight(false);
    return true;
}