#include "Player/TFTPlayerState.h"

#include "TFTGameMode.h"
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
    
    if (TFTGameMode->Battlefield  && Unit->GridCol >= 0)
    {
        TFTGameMode->Battlefield->FreePlayerCell(Unit->GridCol, Unit->GridRow);
        Unit->GridCol = -1;
        Unit->GridRow = -1;
    }

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

void ATFTPlayerState::CheckForMerge(FName UnitName)
{
    // TODO: Find all copies of this unit across board and bench
    // HINT: Call FindAllCopies
    
    TArray<AUnit*> Copies1 = FindAllCopies(UnitName);
    
    TArray<AUnit*> OneStars = Copies1.FilterByPredicate([](AUnit* Unit)
    {
        return Unit && Unit->StarLevel == 1; 
    });
    
    if (OneStars.Num()>=3)
    {
        MergeUnits(OneStars);
    }
    
    TArray<AUnit*> Copies2 = FindAllCopies(UnitName);
    
    TArray<AUnit*> TwoStars = Copies2.FilterByPredicate([](AUnit* Unit)
    {
        return Unit && Unit->StarLevel == 2; 
    });
    
    if (TwoStars.Num()>=3)
    {
        MergeUnits(TwoStars);
    }
    
}

TArray<AUnit*> ATFTPlayerState::FindAllCopies(FName UnitName)
{
    TArray<AUnit*> Copies;
    
    
    for (AUnit* Unit: BoardUnits)
    {
        if (Unit && Unit->UnitName == UnitName)
        {
            Copies.Add(Unit);
        }
    }
    
    for (AUnit* Unit: BenchUnits)
    {
        if (Unit && Unit->UnitName == UnitName)
        {
            Copies.Add(Unit);
        }
    }

    return Copies;
}

void ATFTPlayerState::MergeUnits(TArray<AUnit*>& Copies)
{
    
    
    while (Copies.Num() >= 3)
    {
        AUnit* Merged = Copies[0];
        AUnit* ToRemove1 = Copies[1];
        AUnit* ToRemove2 = Copies[2];
        
        BoardUnits.Remove(ToRemove1);
        BenchUnits.Remove(ToRemove1);
        ToRemove1->Destroy();
        
        BoardUnits.Remove(ToRemove2);
        BenchUnits.Remove(ToRemove2);
        ToRemove2->Destroy();
        
        Copies.RemoveAt(2);
        Copies.RemoveAt(1);
        Copies.RemoveAt(0);
    
        Merged->StarLevel++;
        
        Merged->InitFromDataAsset();

        UE_LOG(LogTemp, Log, TEXT("%s merged to %d star"),
            *Merged->UnitName.ToString(), Merged->StarLevel);
    }

}

void ATFTPlayerState::TryAutoPlace(AUnit* Unit)
{
    if (!Unit || !TFTGameMode->Battlefield) return;
    if (BoardUnits.Contains(Unit)) return;
    if (!CanPlaceOnBoard()) return;

    int32 Col, Row;
    if (TFTGameMode->Battlefield->GetNextFreePlayerCell(Col, Row))
    {
        FVector Position = TFTGameMode->Battlefield->GetPlayerCellPosition(Col, Row);
        Unit->SetActorLocation(Position);
        TFTGameMode->Battlefield->OccupyPlayerCell(Col, Row);

        // Store cell on unit so we can free it later
        Unit->GridCol = Col;
        Unit->GridRow = Row;

        MoveToBoard(Unit);

        UE_LOG(LogTemp, Log, TEXT("Auto-placed %s at cell [%d,%d]"),
            *Unit->UnitName.ToString(), Col, Row);
    }
}