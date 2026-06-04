#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattlefieldActor.generated.h"

UCLASS()
class CUSTOMPROJECT_API ABattlefieldActor : public AActor
{
    GENERATED_BODY()

public:
    ABattlefieldActor();
    virtual void BeginPlay() override;

    // -------------------------------------------------------
    // Grid Config
    // -------------------------------------------------------

    // Number of columns per side
    UPROPERTY(EditAnywhere, Category="Battlefield")
    int32 Columns = 4;

    // Number of rows (shared between both sides)
    UPROPERTY(EditAnywhere, Category="Battlefield")
    int32 Rows = 8;

    // Distance between each cell in cm
    UPROPERTY(EditAnywhere, Category="Battlefield")
    float CellSize = 200.f;

    // -------------------------------------------------------
    // Grid Queries
    // -------------------------------------------------------

    // Returns world position of a player grid cell
    // Col 0-3, Row 0-7
    UFUNCTION(BlueprintCallable, Category="Battlefield")
    FVector GetPlayerCellPosition(int32 Col, int32 Row) const;

    // Returns world position of an enemy grid cell
    UFUNCTION(BlueprintCallable, Category="Battlefield")
    FVector GetEnemyCellPosition(int32 Col, int32 Row) const;

    // Returns the next available player cell
    // Used for auto-placing units
    UFUNCTION(BlueprintCallable, Category="Battlefield")
    bool GetNextFreePlayerCell(int32& OutCol, int32& OutRow) const;

    // Returns the next available enemy cell
    UFUNCTION(BlueprintCallable, Category="Battlefield")
    bool GetNextFreeEnemyCell(int32& OutCol, int32& OutRow) const;

    // Mark a cell as occupied or free
    void OccupyPlayerCell(int32 Col, int32 Row);
    void FreePlayerCell(int32 Col, int32 Row);
    void OccupyEnemyCell(int32 Col, int32 Row);
    void FreeEnemyCell(int32 Col, int32 Row);

    // Check if a cell is free
    bool IsPlayerCellFree(int32 Col, int32 Row) const;
    bool IsEnemyCellFree(int32 Col, int32 Row) const;

private:
    // Tracks which cells are occupied
    // true = occupied, false = free
    TArray<TArray<bool>> PlayerGrid;
    TArray<TArray<bool>> EnemyGrid;

    void InitGrids();
    
    void DrawBorder();
};