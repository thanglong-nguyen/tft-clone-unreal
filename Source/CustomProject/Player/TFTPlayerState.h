#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Units/Unit.h"
#include "TFTPlayerState.generated.h"

// Fired when the player levels up
// UI listens to this to update the level display
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelUp, int32, NewLevel, int32, BoardCapacity);

UCLASS()
class CUSTOMPROJECT_API ATFTPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    ATFTPlayerState();

    // -------------------------------------------------------
    // Level & XP
    // -------------------------------------------------------

    // Current player level — determines board capacity
    UPROPERTY(BlueprintReadOnly, Category="Level")
    int32 PlayerLevel = 1;

    // XP accumulated toward the next level
    UPROPERTY(BlueprintReadOnly, Category="Level")
    int32 CurrentXP = 0;

    // XP required to reach each level
    // Index 0 = XP to go from level 1 to 2, index 1 = level 2 to 3 etc
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
    TArray<int32> XPThresholds = {2, 6, 10, 16, 22, 34, 46, 60};

    // Returns how many units the player can place on the board
    // Equals PlayerLevel, capped at MaxLevel
    UFUNCTION(BlueprintCallable, Category="Level")
    int32 GetBoardCapacity() const;

    // Returns total XP needed to reach the next level
    UFUNCTION(BlueprintCallable, Category="Level")
    int32 GetXPToNextLevel() const;

    // Adds XP and triggers level up if threshold met
    void AddXP(int32 Amount);

    // Spends 4 gold to gain 4 XP — returns false if not enough gold
    bool BuyXP();

    // -------------------------------------------------------
    // Board & Bench
    // -------------------------------------------------------

    // Units currently placed on the battlefield
    UPROPERTY(BlueprintReadOnly, Category="Board")
    TArray<AUnit*> BoardUnits;

    // Units owned but not on the battlefield
    UPROPERTY(BlueprintReadOnly, Category="Board")
    TArray<AUnit*> BenchUnits;

    // Returns true if board has room for another unit
    bool CanPlaceOnBoard() const;

    // Moves a unit from bench to board — returns false if board is full
    bool MoveToBoard(AUnit* Unit);

    // Moves a unit from board to bench
    void MoveToBench(AUnit* Unit);
    
    // Checks if 3 copies of a unit exist across board and bench
    // Call this after any unit is added to board or bench
    void CheckForMerge(FName UnitName);

    // -------------------------------------------------------
    // Gold
    // -------------------------------------------------------

    // Current gold amount
    UPROPERTY(BlueprintReadOnly, Category="Economy")
    int32 Gold = 0;

    // Adds gold to the player's total
    void AddGold(int32 Amount);

    // Spends gold — returns false if player can't afford it
    bool SpendGold(int32 Amount);

    // -------------------------------------------------------
    // Delegates
    // -------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category="Level")
    FOnLevelUp OnLevelUp;

private:
    // Maximum level a player can reach
    static constexpr int32 MaxLevel  = 8;

    // Gold cost to manually buy XP
    static constexpr int32 GoldForXP = 4;

    // Checks if accumulated XP meets the next level threshold
    // Called automatically by AddXP — not called directly
    void CheckLevelUp();
    
    // Finds all units with a matching name across board and bench
    TArray<AUnit*> FindAllCopies(FName UnitName);

    // Performs the actual merge — upgrades one unit to next star level
    void MergeUnits(TArray<AUnit*>& Copies);
};