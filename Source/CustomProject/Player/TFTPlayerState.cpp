#include "Player/TFTPlayerState.h"
#include "Units/Unit.h"

ATFTPlayerState::ATFTPlayerState()
{
    // Starting values are set inline in the header
}

// -------------------------------------------------------
// Level & XP
// -------------------------------------------------------

int32 ATFTPlayerState::GetBoardCapacity() const
{
    // Board capacity equals player level, capped at MaxLevel
    return FMath::Min(PlayerLevel, MaxLevel);
}

int32 ATFTPlayerState::GetXPToNextLevel() const
{
    // At max level there is no next threshold
    if (PlayerLevel == MaxLevel) return INT32_MAX;

    // XPThresholds index maps directly to current level
    // Level 1 needs XPThresholds[0], level 2 needs XPThresholds[1] etc
    return XPThresholds[PlayerLevel - 1];
}

void ATFTPlayerState::AddXP(int32 Amount)
{
    // Already at max level — nothing to do
    if (PlayerLevel == MaxLevel) return;

    CurrentXP += Amount;

    // Check if the new XP total triggers a level up
    CheckLevelUp();
}

void ATFTPlayerState::CheckLevelUp()
{
    // Keep leveling up as long as XP meets the next threshold
    // Handles edge case where one XP grant causes multiple level ups
    while (PlayerLevel < MaxLevel)
    {
        int32 XPToNext = GetXPToNextLevel();

        if (CurrentXP >= XPToNext)
        {
            // Carry over excess XP to the next level
            CurrentXP -= XPToNext;
            PlayerLevel++;

            // Notify UI and any other listeners
            OnLevelUp.Broadcast(PlayerLevel, GetBoardCapacity());
        }
        else
        {
            // Not enough XP for another level — stop checking
            break;
        }
    }
}

bool ATFTPlayerState::BuyXP()
{
    // Attempt to spend gold first — fails silently if not enough
    if (!SpendGold(GoldForXP)) return false;

    AddXP(4);
    return true;
}

// -------------------------------------------------------
// Board & Bench
// -------------------------------------------------------

bool ATFTPlayerState::CanPlaceOnBoard() const
{
    return BoardUnits.Num() < GetBoardCapacity();
}

bool ATFTPlayerState::MoveToBoard(AUnit* Unit)
{
    if (!Unit) return false;

    // Already on the board — nothing to do
    if (BoardUnits.Contains(Unit)) return true;

    // Board is full — player needs to level up first
    if (!CanPlaceOnBoard())
    {
        UE_LOG(LogTemp, Warning, TEXT("Board full — level up to place more units"));
        return false;
    }

    // Move from bench to board
    BenchUnits.Remove(Unit);
    BoardUnits.Add(Unit);
    return true;
}

void ATFTPlayerState::MoveToBench(AUnit* Unit)
{
    if (!Unit) return;

    BoardUnits.Remove(Unit);

    // Avoid duplicates on the bench
    if (!BenchUnits.Contains(Unit))
        BenchUnits.Add(Unit);
}

// -------------------------------------------------------
// Gold
// -------------------------------------------------------

void ATFTPlayerState::AddGold(int32 Amount)
{
    Gold += Amount;
}

bool ATFTPlayerState::SpendGold(int32 Amount)
{
    // Can't spend what you don't have
    if (Gold < Amount) return false;

    Gold -= Amount;
    return true;
}