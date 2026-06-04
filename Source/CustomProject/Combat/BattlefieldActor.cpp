#include "Combat/BattlefieldActor.h"

ABattlefieldActor::ABattlefieldActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ABattlefieldActor::BeginPlay()
{
    Super::BeginPlay();
    InitGrids();
    DrawBorder();
}

void ABattlefieldActor::InitGrids()
{
    // Initialise both grids as fully empty
    PlayerGrid.SetNum(Columns);
    EnemyGrid.SetNum(Columns);

    for (int32 Col = 0; Col < Columns; Col++)
    {
        PlayerGrid[Col].Init(false, Rows);
        EnemyGrid[Col].Init(false, Rows);
    }
}


void ABattlefieldActor::DrawBorder()
{
    // Player side grid lines
    for (int32 Row = 0; Row <= Rows/2; Row++)
    {
        // Horizontal lines — run along Y axis (separating rows)
        FVector Start = GetPlayerCellPosition(0, Row) + FVector(CellSize * 0.5f, -CellSize * 0.5f, 1.f);
        FVector End   = GetPlayerCellPosition(Columns - 1, Row) + FVector(CellSize * 0.5f, CellSize * 0.5f, 1.f);
        DrawDebugLine(GetWorld(), Start, End, FColor::Green, true, -1.f, 0, 2.f);
    }

    for (int32 Col = 0; Col <= Columns; Col++)
    {
        // Vertical lines — run along X axis (separating columns)
        FVector Start = GetPlayerCellPosition(Col, 0) + FVector(CellSize * 0.5f, -CellSize * 0.5f, 1.f);
        FVector End   = GetPlayerCellPosition(Col, Rows/2 - 1) + FVector(-CellSize * 0.5f, -CellSize * 0.5f, 1.f);
        DrawDebugLine(GetWorld(), Start, End, FColor::Green, true, -1.f, 0, 2.f);
    }
    
    // Enemy side grid lines — red tint
    for (int32 Row = 0; Row <= Rows/2; Row++)
    {
        FVector Start = GetEnemyCellPosition(0, Row) + FVector(-CellSize * 0.5f, -CellSize * 0.5f, 1.f);
        FVector End   = GetEnemyCellPosition(Columns - 1, Row) + FVector(-CellSize * 0.5f, CellSize * 0.5f, 1.f);
        DrawDebugLine(GetWorld(), Start, End, FColor::Red, true, -1.f, 0, 2.f);
    }

    for (int32 Col = 0; Col <= Columns; Col++)
    {
        FVector Start = GetEnemyCellPosition(Col, 0) + FVector(-CellSize * 0.5f, -CellSize * 0.5f, 1.f);
        FVector End   = GetEnemyCellPosition(Col, Rows/2 - 1) + FVector(CellSize * 0.5f, -CellSize * 0.5f, 1.f);
        DrawDebugLine(GetWorld(), Start, End, FColor::Red, true, -1.f, 0, 2.f);
    }
}

FVector ABattlefieldActor::GetPlayerCellPosition(int32 Col, int32 Row) const
{
    FVector Origin = GetActorLocation();
    return Origin + FVector(
        - (Row + 1 )* CellSize,                          // rows go forward/back (X)
        (Col - Columns / 2.f) * CellSize,        // cols go left/right (Y)
        0.f
    );
}

FVector ABattlefieldActor::GetEnemyCellPosition(int32 Col, int32 Row) const
{
    FVector Origin = GetActorLocation();
    return Origin + FVector(
        (Row + 1) * CellSize,                       // enemy side negative X
        (Col - Columns / 2.f) * CellSize,        // cols go left/right (Y)
        0.f
    );
}

bool ABattlefieldActor::GetNextFreePlayerCell(int32& OutCol, int32& OutRow) const
{
    for (int32 Row = 0; Row < Rows; Row++)
    {
        for (int32 Col = 0; Col < Columns; Col++)
        {
            if (!PlayerGrid[Col][Row])
            {
                OutCol = Col;
                OutRow = Row;
                return true;
            }
        }
    }
    return false; // no free cells
}

bool ABattlefieldActor::GetNextFreeEnemyCell(int32& OutCol, int32& OutRow) const
{
    for (int32 Row = 0; Row < Rows; Row++)
    {
        for (int32 Col = 0; Col < Columns; Col++)
        {
            if (!EnemyGrid[Col][Row])
            {
                OutCol = Col;
                OutRow = Row;
                return true;
            }
        }
    }
    return false;
}

void ABattlefieldActor::OccupyPlayerCell(int32 Col, int32 Row)
{
    if (PlayerGrid.IsValidIndex(Col) && PlayerGrid[Col].IsValidIndex(Row))
        PlayerGrid[Col][Row] = true;
}

void ABattlefieldActor::FreePlayerCell(int32 Col, int32 Row)
{
    if (PlayerGrid.IsValidIndex(Col) && PlayerGrid[Col].IsValidIndex(Row))
        PlayerGrid[Col][Row] = false;
}

void ABattlefieldActor::OccupyEnemyCell(int32 Col, int32 Row)
{
    if (EnemyGrid.IsValidIndex(Col) && EnemyGrid[Col].IsValidIndex(Row))
        EnemyGrid[Col][Row] = true;
}

void ABattlefieldActor::FreeEnemyCell(int32 Col, int32 Row)
{
    if (EnemyGrid.IsValidIndex(Col) && EnemyGrid[Col].IsValidIndex(Row))
        EnemyGrid[Col][Row] = false;
}

bool ABattlefieldActor::IsPlayerCellFree(int32 Col, int32 Row) const
{
    if (PlayerGrid.IsValidIndex(Col) && PlayerGrid[Col].IsValidIndex(Row))
        return !PlayerGrid[Col][Row];
    return false;
}

bool ABattlefieldActor::IsEnemyCellFree(int32 Col, int32 Row) const
{
    if (EnemyGrid.IsValidIndex(Col) && EnemyGrid[Col].IsValidIndex(Row))
        return !EnemyGrid[Col][Row];
    return false;
}