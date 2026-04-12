#include "Player/TFTPlayerState.h"
#include "Units/Unit.h"

ATFTPlayerState::ATFTPlayerState()
{
    PlayerLevel = 3;
}

void ATFTPlayerState::AddXP(int32 Amount)
{
    if (PlayerLevel >= 8) return; // already max level

    CurrentXP += Amount;
    CheckLevelUp();

    UE_LOG(LogTemp, Log, TEXT("XP: %d / %d | Level: %d"),
        CurrentXP, GetXPToNextLevel(), PlayerLevel);
}

void ATFTPlayerState::CheckLevelUp()
{
    // Keep leveling up while XP meets threshold
    while (PlayerLevel < 8)
    {
        int32 Required = GetXPToNextLevel();
        if (CurrentXP < Required) break;

        CurrentXP   -= Required;
        PlayerLevel++;

        OnLevelUp.Broadcast(PlayerLevel, GetBoardCapacity());

        UE_LOG(LogTemp, Log, TEXT("LEVEL UP → Level %d | Board Capacity: %d"),
            PlayerLevel, GetBoardCapacity());
    }
}

int32 ATFTPlayerState::GetXPToNextLevel() const
{
    int32 Index = PlayerLevel - 1; // level 1 needs index 0
    if (XPThresholds.IsValidIndex(Index))
        return XPThresholds[Index];
    return INT32_MAX; // max level
}

bool ATFTPlayerState::BuyXP()
{
    if (!SpendGold(BuyXPCost)) return false;
    AddXP(BuyXPAmount);
    return true;
}

bool ATFTPlayerState::CanPlaceOnBoard() const
{
    return BoardUnits.Num() < GetBoardCapacity();
}

bool ATFTPlayerState::MoveToBoard(AUnit* Unit)
{
    if (!Unit) return false;

    // Already on board
    if (BoardUnits.Contains(Unit)) return true;

    // Board is full
    if (!CanPlaceOnBoard())
    {
        UE_LOG(LogTemp, Warning, TEXT("Board full — level up to place more units"));
        return false;
    }

    BenchUnits.Remove(Unit);
    BoardUnits.Add(Unit);
    return true;
}

void ATFTPlayerState::MoveToBench(AUnit* Unit)
{
    if (!Unit) return;
    BoardUnits.Remove(Unit);
    if (!BenchUnits.Contains(Unit))
        BenchUnits.Add(Unit);
}

void ATFTPlayerState::AddGold(int32 Amount)
{
    Gold = FMath::Min(Gold + Amount, 50); // cap at 50 like TFT
}

bool ATFTPlayerState::SpendGold(int32 Amount)
{
    if (Gold < Amount) return false;
    Gold -= Amount;
    return true;
}