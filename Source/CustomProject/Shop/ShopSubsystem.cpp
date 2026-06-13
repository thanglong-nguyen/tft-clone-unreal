#include "Shop/ShopSubsystem.h"
#include "Units/Unit.h"
#include "Units/UnitDataAsset.h"
#include "Player/TFTPlayerState.h"
#include "TFTGameMode.h"
#include "Engine/World.h"
#include "UI/BoardWidget.h"

void UShopSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    CurrentShop.SetNum(ShopSize); // pre-allocate so array is always ShopSize long
}

// -------------------------------------------------------
// Pool Management
// -------------------------------------------------------

void UShopSubsystem::InitPool()
{
    UnitPool.Empty();
    for (UUnitDataAsset* UnitData : AllUnits)
    {
        if (!UnitData) continue;
        UnitPool.Add(UnitData->UnitName, CopiesPerUnit);
    }
}

void UShopSubsystem::ReturnToPool(FName UnitName)
{
    int32* Count = UnitPool.Find(UnitName);
    if (Count && *Count < CopiesPerUnit)
        (*Count)++;
}

// -------------------------------------------------------
// Rarity and Drawing
// -------------------------------------------------------

int32 UShopSubsystem::PickCostTier(int32 PlayerLevel) const
{
    int32 LevelIndex = FMath::Clamp(PlayerLevel - 1, 0, 7);
    float Luck       = FMath::FRand();
    const float* Rarity = RarityTable[LevelIndex];

    // Walk cumulative weights until the roll is covered
    // Higher weight = larger slice of the 0-1 range = more likely
    float Cumulative = 0.f;
    int32 Cost = 1;
    for (int Tier = 0; Tier < 5; ++Tier)
    {
        Cumulative += Rarity[Tier];
        if (Cumulative >= Luck)
            return Tier + 1; // tiers are 0-indexed, costs are 1-5
    }
    return Cost;
}

UUnitDataAsset* UShopSubsystem::DrawFromPool(int32 PlayerLevel)
{
    // Bail early if the pool is completely empty
    TArray<int32> Counts;
    UnitPool.GenerateValueArray(Counts);
    bool bAnyAvailable = Counts.ContainsByPredicate([](int32 C){ return C > 0; });
    if (!bAnyAvailable) return nullptr;

    int32 Tier = PickCostTier(PlayerLevel);

    // Find all units at this cost tier with copies remaining
    TArray<UUnitDataAsset*> Candidates;
    for (UUnitDataAsset* UnitData : AllUnits)
    {
        if (!UnitData) continue;
        int32* Count = UnitPool.Find(UnitData->UnitName);
        if (UnitData->Cost == Tier && Count && *Count > 0)
            Candidates.Add(UnitData);
    }

    // Nothing at this tier — re-roll (safe because pool exhaustion is caught above)
    if (Candidates.IsEmpty())
        return DrawFromPool(PlayerLevel);

    UUnitDataAsset* Picked = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
    UnitPool.FindOrAdd(Picked->UnitName)--;
    return Picked;
}

// -------------------------------------------------------
// Shop Generation
// -------------------------------------------------------

void UShopSubsystem::GenerateShop(int32 PlayerLevel)
{
    if (UnitPool.IsEmpty()) InitPool();

    // Return unsold units from previous shop back to the pool
    for (const FShopSlot& ShopSlot : CurrentShop)
        if (!ShopSlot.bIsPurchased && ShopSlot.Data)
            ReturnToPool(ShopSlot.Data->UnitName);

    // Fill each slot with a freshly drawn unit
    for (int i = 0; i < ShopSize; ++i)
    {
        CurrentShop[i].Data        = DrawFromPool(PlayerLevel);
        CurrentShop[i].bIsPurchased = false;
    }

    OnShopRefresh.Broadcast();
}

void UShopSubsystem::FreeRefresh(int32 PlayerLevel)
{
    if (UnitPool.IsEmpty()) InitPool();
    GenerateShop(PlayerLevel);
}

bool UShopSubsystem::RefreshShop(int32 PlayerLevel)
{
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (ATFTPlayerState* PS = PC->GetPlayerState<ATFTPlayerState>())
        {
            if (!PS->SpendGold(2)) return false;
            GenerateShop(PlayerLevel);
            return true;
        }
    }
    return false;
}

// -------------------------------------------------------
// Buy and Sell
// -------------------------------------------------------

bool UShopSubsystem::BuyUnit(int32 SlotIndex)
{
    FShopSlot Slot = CurrentShop[SlotIndex];
    if (Slot.bIsPurchased || !Slot.Data) return false;

    // Check bench has space before spending gold
    int32 BenchIndex = TFTGameMode->GetNextFreeBenchSlot();
    if (BenchIndex == -1)
    {
        TFTGameMode->ShowMessage(TEXT("Bench is full"), 2.f, FLinearColor::Yellow);
        return false;
    }

    if (!TFTGameMode->PS) return false;
    ATFTPlayerState* PS = TFTGameMode->PS;

    if (!PS->SpendGold(Slot.Data->Cost)) return false;

    // Spawn off screen — MoveToBench will place it at the correct bench position
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AUnit* PurchasedUnit = GetWorld()->SpawnActor<AUnit>(
        AUnit::StaticClass(), FVector(-9999.f, -9999.f, -9999.f),
        FRotator::ZeroRotator, Params);

    if (!PurchasedUnit) return false;

    // Initialise stats and widget from data asset
    PurchasedUnit->DataAsset        = Slot.Data;
    PurchasedUnit->StateWidgetClass = TFTGameMode->StateWidgetClass;
    PurchasedUnit->InitFromDataAsset();
    PurchasedUnit->SetStateWidget();
    PS->MoveToBench(PurchasedUnit);

    // Green health bar to distinguish player units from enemies
    if (PurchasedUnit->StateWidget)
        PurchasedUnit->StateWidget->HealthBar->SetFillColorAndOpacity(FLinearColor::Green);

    CurrentShop[SlotIndex].bIsPurchased = true;

    PS->CheckForMerge(PurchasedUnit->UnitName);
    OnShopRefresh.Broadcast();
    OnUnitTransaction.Broadcast(PurchasedUnit);

    return true;
}

void UShopSubsystem::SellUnit(AUnit* Unit)
{
    if (!Unit || !TFTGameMode->PS) return;

    ATFTPlayerState* PS = TFTGameMode->PS;

    // Refund scales with star level — 1★=1x, 2★=3x, 3★=9x base cost
    int32 Copies = FMath::Pow(3.f, Unit->StarLevel - 1);
    int32 Refund = Unit->DataAsset ? Unit->DataAsset->Cost * Copies : 1;
    PS->AddGold(Refund);

    // Return copies to the shared pool
    for (int32 i = 0; i < Copies; i++)
        ReturnToPool(Unit->UnitName);

    // Free bench slot if unit was on bench
    if (Unit->BenchSlotIndex >= 0)
    {
        TFTGameMode->FreeBenchSlot(Unit->BenchSlotIndex);
        Unit->BenchSlotIndex = -1;
    }

    // Free board cell and clear UI card if unit was on board
    if (Unit->GridCol >= 0)
    {
        TFTGameMode->BoardWidget->ClearCell(Unit->GridCol, Unit->GridRow);
        TFTGameMode->Battlefield->FreePlayerCell(Unit->GridCol, Unit->GridRow);
        Unit->GridCol = -1;
        Unit->GridRow = -1;
    }

    PS->BenchUnits.Remove(Unit);
    PS->BoardUnits.Remove(Unit);

    OnUnitTransaction.Broadcast(Unit);
    Unit->Destroy();

    TFTGameMode->ShowMessage(
        FString::Printf(TEXT("Sold %s ★%d for %dg"),
            *Unit->UnitName.ToString(), Unit->StarLevel, Refund),
        2.f, FLinearColor::Yellow);
}