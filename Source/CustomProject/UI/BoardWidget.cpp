#include "UI/BoardWidget.h"
#include "UI/SynergyCard.h"
#include "UI/BoardCellWidget.h"
#include "Components/UniformGridPanel.h"
#include "TFTGameMode.h"
#include "UnitCard.h"
#include "Traits/TraitSubsystem.h"
#include "Shop/ShopSubsystem.h"
#include "Player/TFTPlayerState.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/UniformGridSlot.h"
#include "Components/Button.h"
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

            // // If a unit already occupies this cell show it
            // if (TFTGameMode->PS)
            // {
            //     for (AUnit* Unit : TFTGameMode->PS->BoardUnits)
            //     {
            //         if (Unit && Unit->GridCol == Col && Unit->GridRow == Row)
            //         {
            //             Cell->PlaceUnit(Unit);
            //             break;
            //         }
            //     }
            // }
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
        TFTGameMode->ShopSS->OnUnitPurchased.AddDynamic(
            this, &UBoardWidget::HandleUnitPurchased);
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
    if (!BenchContainer || !TFTGameMode->PS) return;
    BenchContainer->ClearChildren();

    for (AUnit* Unit : TFTGameMode->PS->BenchUnits)
    {
        if (!Unit) continue;

        UUnitCard* Card = CreateWidget<UUnitCard>(this, UnitCardClass);
        if (!Card) continue;

        Card->SetUnit(Unit, false); // false = on bench
        BenchContainer->AddChild(Card);
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

void UBoardWidget::HandleTraitsUpdated()
{
    RefreshSynergies();
}

void UBoardWidget::HandleUnitPurchased(AUnit* NewUnit)
{
    RefreshBench();
    RefreshSynergies();
}