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
        ToggleHUDButton->OnClicked.AddDynamic(
            this, &UBoardWidget::OnToggleHUDClicked);
    
    if (SellZone)
    {
        SellZone->TFTGameMode = TFTGameMode;
    }
}

void UBoardWidget::BuildBoardGrid()
{
    if (!BoardGrid || !BoardCellClass || !TFTGameMode) return;

    BoardGrid->ClearChildren();

    // Match battlefield dimensions
    int32 Columns = 4;
    int32 Rows    = 4;

    for (int32 Row = 0; Row < Rows; Row++)
    {
        for (int32 Col = 0; Col < Columns; Col++)
        {
            UBoardCellWidget* Cell = CreateWidget<UBoardCellWidget>(
                this, BoardCellClass);
            
            if (!Cell) continue;

            Cell->TFTGameMode = TFTGameMode;
            Cell->Col         = Col;
            Cell->Row         = Row;

            // Add to grid at correct position
            // BoardGrid->AddChildToUniformGrid(Cell, Row, Col);
            
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

    // Refresh synergies when traits change
    if (UTraitSubsystem* Traits = GetGameInstance()->GetSubsystem<UTraitSubsystem>())
        Traits->OnTraitsUpdated.AddDynamic(this, &UBoardWidget::HandleTraitsUpdated);

    // Refresh bench when a unit is bought
    if (TFTGameMode->ShopSS)
        TFTGameMode->ShopSS->OnUnitTransaction.AddDynamic(
            this, &UBoardWidget::HandleUnitTransaction);
    
    if (TFTGameMode->PS)
    {
        TFTGameMode->PS->OnUnitsMerged.AddDynamic(
            this, &UBoardWidget::HandleUnitsMerged);
        TFTGameMode->PS->OnUnitPlaced.AddDynamic(
            this, &UBoardWidget::HandleUnitPlaced);
    }
}

void UBoardWidget::RefreshSynergies()
{
    if (!SynergyContainer || !SynergyCardClass) return;

    UTraitSubsystem* Traits = GetGameInstance()->GetSubsystem<UTraitSubsystem>();
    if (!Traits) return;

    // Clear existing cards
    SynergyContainer->ClearChildren();

    // Add a card for every trait the player has at least 1 unit in
    for (const FActiveTraitStatus& Status : Traits->GetAllTraitStatuses())
    {
        if (Status.CurrentCount == 0) continue; // skip traits with no units

        USynergyCard* Card = CreateWidget<USynergyCard>(
            this, SynergyCardClass);
        if (!Card) continue;

        Card->SetData(
            Status.DisplayName,
            Status.CurrentCount,
            Status.NextThreshold,
            Status.bIsActive
        );

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
        UBoardCellWidget* Cell = CreateWidget<UBoardCellWidget>(
            this, BoardCellClass);
        if (!Cell) continue;

        Cell->TFTGameMode   = TFTGameMode;
        Cell->bIsBenchCell  = true;
        Cell->UnitCardClass = UnitCardClass;

        // Place unit if one exists at this bench index
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

    // Cells are added in Row-major order: index = Col + Row * Columns
    int32 Columns = 4;
    int32 Index   = Col + Row * Columns;

    return Cast<UBoardCellWidget>(BoardGrid->GetChildAt(Index));
}

void UBoardWidget::OnToggleHUDClicked()
{
    if (!TFTGameMode || !TFTGameMode->HUDWidget) return;

    UTFTHUDWidget* HUD = TFTGameMode->HUDWidget;
    
    this->SetVisibility(ESlateVisibility::Hidden);
    HUD->SetVisibility(ESlateVisibility::Visible);
}

void UBoardWidget::ClearCell(int32 Col, int32 Row)
{
    UBoardCellWidget* Cell = GetCellWidget(Col, Row);
    if (!Cell) return;

    Cell->ClearUnit();
}

void UBoardWidget::HandleUnitsMerged(int32 Col, int32 Row)
{
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

void UBoardWidget::RebuildOccupiedCells()
{
    if (!TFTGameMode->PS) return;
    
    for (AUnit* Unit : TFTGameMode->PS->BoardUnits)
    {
        if (!Unit || Unit->GridCol < 0) continue;
        
        UBoardCellWidget* Cell = GetCellWidget(Unit->GridCol, Unit->GridRow);
        if (Cell) Cell->PlaceUnit(Unit);
    }
}

void UBoardWidget::HandleTraitsUpdated()
{
    RefreshSynergies();
}

void UBoardWidget::HandleUnitTransaction(AUnit* NewUnit)
{
    RefreshBench();
    RefreshSynergies();
    RebuildOccupiedCells();
}