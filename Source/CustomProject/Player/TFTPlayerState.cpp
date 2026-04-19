#include "Player/TFTPlayerState.h"
#include "Units/Unit.h"

ATFTPlayerState::ATFTPlayerState()
{
    // Nothing needed here for now
}

int32 ATFTPlayerState::GetBoardCapacity() const
{
    
    return FMath::Min(PlayerLevel,MaxLevel);
}

int32 ATFTPlayerState::GetXPToNextLevel() const
{
    if (PlayerLevel == MaxLevel)
    {
        return  INT32_MAX;
    }
    
    int32 XPNeeded = XPThresholds[PlayerLevel-1];
    
    return XPNeeded;
    
}

void ATFTPlayerState::AddXP(int32 Amount)
{
    if (PlayerLevel == MaxLevel)
    {
        return  ;
    }
    
    CurrentXP += Amount;
    CheckLevelUp();
}

void ATFTPlayerState::CheckLevelUp()
{
    while (PlayerLevel < MaxLevel)
    {
        int32 XPToNext = GetXPToNextLevel();

        if (CurrentXP >= XPToNext)
        {
            CurrentXP -= XPToNext;
            PlayerLevel++;
            OnLevelUp.Broadcast(PlayerLevel, GetBoardCapacity());
        }
        else
        {
            break;
        }
    }
}

bool ATFTPlayerState::BuyXP()
{
    if (!SpendGold(GoldForXP)) return false;
    AddXP(4);
    return true;
}
    


bool ATFTPlayerState::CanPlaceOnBoard() const
{
    return BoardUnits.Num() < GetBoardCapacity();
    
}

bool ATFTPlayerState::MoveToBoard(AUnit* Unit)
{
    if (!Unit)
    {
        return false;
    }
    
    if (BoardUnits.Contains(Unit))
    {
        return true;
    }
    
    if (BoardUnits.Num() == GetBoardCapacity())
    {
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
    Gold+= Amount;
}

bool ATFTPlayerState::SpendGold(int32 Amount)
{
    if (Gold < Amount)
    {
        return false;
    } else
    {
        Gold -= Amount;
        return true;
    }
}