#include "Player/TFTPlayerState.h"
#include "TFTGameMode.h"
#include "Traits/TraitSubsystem.h"
#include "Units/Unit.h"

ATFTPlayerState::ATFTPlayerState()
{
    // Starting values set inline in the header
}

// -------------------------------------------------------
// Level & XP
// -------------------------------------------------------

int32 ATFTPlayerState::GetBoardCapacity() const
{
    return FMath::Min(PlayerLevel, MaxLevel);
}

int32 ATFTPlayerState::GetXPToNextLevel() const
{
    if (PlayerLevel == MaxLevel) return INT32_MAX;
    return XPThresholds[PlayerLevel - 1];
}

void ATFTPlayerState::AddXP(int32 Amount)
{
    if (PlayerLevel == MaxLevel) return;
    CurrentXP += Amount;
    CheckLevelUp();
}

void ATFTPlayerState::CheckLevelUp()
{
    // Loop in case one XP grant causes multiple level ups
    while (PlayerLevel < MaxLevel)
    {
        int32 XPToNext = GetXPToNextLevel();
        if (CurrentXP >= XPToNext)
        {
            CurrentXP -= XPToNext; // carry over excess XP
            PlayerLevel++;
            OnLevelUp.Broadcast(PlayerLevel, GetBoardCapacity());
        }
        else break;
    }
}

bool ATFTPlayerState::BuyXP()
{
    if (!SpendGold(GoldForXP)) return false;
    AddXP(4);
    return true;
}

// -------------------------------------------------------
// Board & Bench
// -------------------------------------------------------

bool ATFTPlayerState::CanPlaceOnBoard() const
{
    if (BoardUnits.Num() < GetBoardCapacity()) return true;
    TFTGameMode->ShowMessage(TEXT("Board full — level up to place more units"),
        2.f, FLinearColor::Red);
    return false;
}

bool ATFTPlayerState::MoveToBoard(AUnit* Unit)
{
    if (!Unit) return false;
    if (BoardUnits.Contains(Unit)) return true;
    if (!CanPlaceOnBoard()) return false;

    // Free bench slot if unit is coming from bench
    if (Unit->BenchSlotIndex >= 0)
    {
        TFTGameMode->FreeBenchSlot(Unit->BenchSlotIndex);
        Unit->BenchSlotIndex = -1;
    }

    BenchUnits.Remove(Unit);
    BoardUnits.Add(Unit);
    return true;
}

void ATFTPlayerState::MoveToBench(AUnit* Unit)
{
    if (!Unit) return;

    // Free battlefield cell if coming from board
    if (TFTGameMode->Battlefield && Unit->GridCol >= 0)
    {
        TFTGameMode->Battlefield->FreePlayerCell(Unit->GridCol, Unit->GridRow);
        Unit->GridCol = -1;
        Unit->GridRow = -1;
    }

    // Free old bench slot if unit was already on bench
    if (Unit->BenchSlotIndex >= 0)
    {
        TFTGameMode->FreeBenchSlot(Unit->BenchSlotIndex);
        Unit->BenchSlotIndex = -1;
    }

    // Place unit in next available bench slot
    int32 BenchIndex = TFTGameMode->GetNextFreeBenchSlot();
    if (BenchIndex == -1) return; // bench full

    Unit->SetActorLocation(TFTGameMode->GetBenchPosition(BenchIndex));
    Unit->BenchSlotIndex = BenchIndex;
    TFTGameMode->OccupyBenchSlot(BenchIndex);

    BoardUnits.Remove(Unit);
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
    if (Gold < Amount)
    {
        TFTGameMode->ShowMessage(TEXT("Not enough gold"), 2.f, FLinearColor::Yellow);
        return false;
    }
    Gold -= Amount;
    return true;
}

// -------------------------------------------------------
// Merging
// -------------------------------------------------------

void ATFTPlayerState::CheckForMerge(FName UnitName)
{
    // Merges only happen during prep — not mid combat
    if (TFTGameMode->CombatSS->CurrentPhase != EGamePhase::Prep) return;

    // Check for 3 one-star copies first
    TArray<AUnit*> Copies = FindAllCopies(UnitName);
    TArray<AUnit*> OneStars = Copies.FilterByPredicate([](AUnit* Unit)
    {
        return Unit && Unit->StarLevel == 1;
    });
    if (OneStars.Num() >= 3) MergeUnits(OneStars);

    // Then check for 3 two-star copies (in case a merge just created one)
    Copies = FindAllCopies(UnitName);
    TArray<AUnit*> TwoStars = Copies.FilterByPredicate([](AUnit* Unit)
    {
        return Unit && Unit->StarLevel == 2;
    });
    if (TwoStars.Num() >= 3) MergeUnits(TwoStars);
}

TArray<AUnit*> ATFTPlayerState::FindAllCopies(FName UnitName)
{
    TArray<AUnit*> Copies;

    for (AUnit* Unit : BoardUnits)
        if (Unit && Unit->UnitName == UnitName)
            Copies.Add(Unit);

    for (AUnit* Unit : BenchUnits)
        if (Unit && Unit->UnitName == UnitName)
            Copies.Add(Unit);

    return Copies;
}

void ATFTPlayerState::MergeUnits(TArray<AUnit*>& Copies)
{
    while (Copies.Num() >= 3)
    {
        AUnit* Merged    = Copies[0];
        AUnit* ToRemove1 = Copies[1];
        AUnit* ToRemove2 = Copies[2];

        // Free bench slots before destroying
        if (ToRemove1->BenchSlotIndex >= 0)
        {
            TFTGameMode->FreeBenchSlot(ToRemove1->BenchSlotIndex);
            ToRemove1->BenchSlotIndex = -1;
        }
        if (ToRemove2->BenchSlotIndex >= 0)
        {
            TFTGameMode->FreeBenchSlot(ToRemove2->BenchSlotIndex);
            ToRemove2->BenchSlotIndex = -1;
        }

        // Notify UI to clear board cells before destroying
        BoardUnits.Remove(ToRemove1);
        BenchUnits.Remove(ToRemove1);
        if (ToRemove1->GridCol >= 0)
            OnUnitsMerged.Broadcast(ToRemove1->GridCol, ToRemove1->GridRow);
        ToRemove1->Destroy();

        BoardUnits.Remove(ToRemove2);
        BenchUnits.Remove(ToRemove2);
        if (ToRemove2->GridCol >= 0)
            OnUnitsMerged.Broadcast(ToRemove2->GridCol, ToRemove2->GridRow);
        ToRemove2->Destroy();

        Copies.RemoveAt(2);
        Copies.RemoveAt(1);
        Copies.RemoveAt(0);

        // Upgrade survivor to next star level
        Merged->StarLevel++;
        Merged->InitFromDataAsset();

        TFTGameMode->ShowMessage(
            FString::Printf(TEXT("%s upgraded to ★%d"), 
                *Merged->UnitName.ToString(), Merged->StarLevel),
            2.f, FLinearColor::Yellow);
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

        Unit->GridCol = Col;
        Unit->GridRow = Row;

        MoveToBoard(Unit);

        // Recalculate synergies since board composition changed
        if (TFTGameMode->TraitSS)
            TFTGameMode->TraitSS->RecalculateTraits(BoardUnits);

        OnUnitPlaced.Broadcast(Unit);
    }
}