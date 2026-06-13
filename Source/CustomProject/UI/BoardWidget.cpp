#include "UI/BoardWidget.h"
#include "UI/SynergyCard.h"
#include "UI/BoardCellWidget.h"
#include "Components/UniformGridPanel.h"
#include "TFTGameMode.h"
#include "Traits/TraitSubsystem.h"
#include "Shop/ShopSubsystem.h"
#include "Player/TFTPlayerState.h"
#include "Components/VerticalBox.h"
#include "Components/UniformGridSlot.h"
#include "Components/Button.h"
#include "UI/SellZoneWidget.h"
#include "Components/HorizontalBoxSlot.h"

void UBoardWidget::NativeConstruct()
{
    Super::NativeConstruct();

    BindDelegates();
    BuildBoardGrid();
    RefreshBench();
    RefreshSynergies();

    if (ToggleHUDButton)
        ToggleHUDButton->OnClicked.AddDynamic(this, &UBoardWidget::OnToggleHUDClicked);
    
    if (SellZone)
        SellZone->TFTGameMode = TFTGameMode;
}

void UBoardWidget::BuildBoardGrid()
{
    if (!BoardGrid || !BoardCellClass || !TFTGameMode) return;

    BoardGrid->ClearChildren();

    int32 Columns = 4;
    int32 Rows    = 4;

    for (int32 Row = 0; Row < Rows; Row++)
    {
        for (int32 Col = 0; Col < Columns; Col++)
        {
            UBoardCellWidget* Cell = CreateWidget<UBoardCellWidget>(this, BoardCellClass);
            if (!Cell) continue;

            Cell->TFTGameMode = TFTGameMode;
            Cell->Col         = Col;
            Cell->Row         = Row;

            UUniformGridSlot* GridSlot = BoardGrid->AddChildToUniformGrid(Cell, Row, Col);
            if (GridSlot)
            {
                GridSlot->SetColumn(Col);
                GridSlot->SetRow(Row);
                GridSlot->SetHorizontalAlignment(HAlign_Fill);
                GridSlot->SetVerticalAlignment(VAlign_Fill);
            }
        }
    }
}

void UBoardWidget::BindDelegates()
{
    if (!TFTGameMode) return;

    if (UTraitSubsystem* Traits = GetGameInstance()->GetSubsystem<UTraitSubsystem>())
        Traits->OnTraitsUpdated.AddDynamic(this, &UBoardWidget::HandleTraitsUpdated);

    if (TFTGameMode->ShopSS)
        TFTGameMode->ShopSS->OnUnitTransaction.AddDynamic(
            this, &UBoardWidget::HandleUnitTransaction);

    if (TFTGameMode->PS)
    {
        TFTGameMode->PS->OnUnitsMerged.AddDynamic(this, &UBoardWidget::HandleUnitsMerged);
        TFTGameMode->PS->OnUnitPlaced.AddDynamic(this, &UBoardWidget::HandleUnitPlaced);
    }
}

void UBoardWidget::RefreshSynergies()
{
    if (!SynergyContainer || !SynergyCardClass) return;

    UTraitSubsystem* Traits = GetGameInstance()->GetSubsystem<UTraitSubsystem>();
    if (!Traits) return;

    SynergyContainer->ClearChildren();

    // Only show traits the player currently has units contributing to
    for (const FActiveTraitStatus& Status : Traits->GetAllTraitStatuses())
    {
        if (Status.CurrentCount == 0) continue;

        USynergyCard* Card = CreateWidget<USynergyCard>(this, SynergyCardClass);
        if (!Card) continue;

        Card->SetData(Status.DisplayName, Status.CurrentCount,
            Status.NextThreshold, Status.bIsActive);
        SynergyContainer->AddChild(Card);
    }
}

void UBoardWidget::RefreshBench()
{
    if (!BenchGrid || !BoardCellClass || !TFTGameMode->PS) return;

    BenchGrid->ClearChildren();

    int32 MaxBench = 5;

    for (int32 i = 0; i < MaxBench; i++)
    {
        UBoardCellWidget* Cell = CreateWidget<UBoardCellWidget>(this, BoardCellClass);
        if (!Cell) continue;

        Cell->TFTGameMode  = TFTGameMode;
        Cell->bIsBenchCell = true;
        Cell->UnitCardClass = UnitCardClass;

        // Show unit card if a unit occupies this bench index
        if (TFTGameMode->PS->BenchUnits.IsValidIndex(i))
        {
            AUnit* Unit = TFTGameMode->PS->BenchUnits[i];
            if (Unit) Cell->PlaceUnit(Unit);
        }

        UUniformGridSlot* Slot_ = BenchGrid->AddChildToUniformGrid(Cell, 0, i);
        if (Slot_)
        {
            Slot_->SetRow(0);
            Slot_->SetColumn(i);
            Slot_->SetHorizontalAlignment(HAlign_Fill);
            Slot_->SetVerticalAlignment(VAlign_Fill);
        }
    }
}

UBoardCellWidget* UBoardWidget::GetCellWidget(int32 Col, int32 Row)
{
    if (!BoardGrid) return nullptr;

    // Cells are added row-major: index = Col + Row * Columns
    int32 Index = Col + Row * 4;
    return Cast<UBoardCellWidget>(BoardGrid->GetChildAt(Index));
}

void UBoardWidget::ClearCell(int32 Col, int32 Row)
{
    UBoardCellWidget* Cell = GetCellWidget(Col, Row);
    if (Cell) Cell->ClearUnit();
}

void UBoardWidget::RebuildOccupiedCells()
{
    if (!TFTGameMode->PS) return;

    // Refresh unit cards for all units currently on the board
    for (AUnit* Unit : TFTGameMode->PS->BoardUnits)
    {
        if (!Unit || Unit->GridCol < 0) continue;
        UBoardCellWidget* Cell = GetCellWidget(Unit->GridCol, Unit->GridRow);
        if (Cell) Cell->PlaceUnit(Unit);
    }
}

void UBoardWidget::OnToggleHUDClicked()
{
    if (!TFTGameMode || !TFTGameMode->HUDWidget) return;
    this->SetVisibility(ESlateVisibility::Hidden);
    TFTGameMode->HUDWidget->SetVisibility(ESlateVisibility::Visible);
}

// -------------------------------------------------------
// Delegate Handlers
// -------------------------------------------------------

void UBoardWidget::HandleUnitsMerged(int32 Col, int32 Row)
{
    // Clear the ghost card left by the destroyed unit then rebuild
    ClearCell(Col, Row);
    RebuildOccupiedCells();
    RefreshBench();
}

void UBoardWidget::HandleUnitPlaced(AUnit* PlacedUnit)
{
    if (!PlacedUnit) return;
    UBoardCellWidget* Cell = GetCellWidget(PlacedUnit->GridCol, PlacedUnit->GridRow);
    if (Cell) Cell->PlaceUnit(PlacedUnit);
    RefreshBench();
    RefreshSynergies();
}

void UBoardWidget::HandleTraitsUpdated()
{
    RefreshSynergies();
}

void UBoardWidget::HandleUnitTransaction(AUnit* NewUnit)
{
    // Called on buy or sell — rebuild everything that could have changed
    RefreshBench();
    RefreshSynergies();
    RebuildOccupiedCells();
}