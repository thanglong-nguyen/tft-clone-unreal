#include "Combat/BattlefieldActor.h"

ABattlefieldActor::ABattlefieldActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ABattlefieldActor::BeginPlay()
{
    Super::BeginPlay();
    InitGrids();
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

FVector ABattlefieldActor::GetPlayerCellPosition(int32 Col, int32 Row) const
{
    // Player side — positive X
    FVector Origin = GetActorLocation();
    return Origin + FVector(
        (Col + 1) * CellSize,           // positive X = player side
        (Row - Rows / 2.f) * CellSize,  // centered on Y axis
        0.f
    );
}

FVector ABattlefieldActor::GetEnemyCellPosition(int32 Col, int32 Row) const
{
    // Enemy side — negative X
    FVector Origin = GetActorLocation();
    return Origin + FVector(
        -(Col + 1) * CellSize,          // negative X = enemy side
        (Row - Rows / 2.f) * CellSize,  // centered on Y axis
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