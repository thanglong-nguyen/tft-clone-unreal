#include "Shop/ShopSubsystem.h"
#include "Units/Unit.h"
#include "Units/UnitDataAsset.h"
#include "Player/TFTPlayerState.h"
#include "Engine/World.h"

void UShopSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Pre-allocate shop slots so array is always ShopSize long
    CurrentShop.SetNum(ShopSize);
}

// -------------------------------------------------------
// Pool Management
// -------------------------------------------------------

void UShopSubsystem::InitPool()
{
    UnitPool.Empty();

    // Give every unit the same number of copies in the shared pool
    for (UUnitDataAsset* UnitData : AllUnits)
    {
        if (!UnitData) continue;
        UnitPool.Add(UnitData->UnitName, CopiesPerUnit);
    }

    UE_LOG(LogTemp, Log, TEXT("Pool initialised with %d unit types"), UnitPool.Num());
}

void UShopSubsystem::ReturnToPool(FName UnitName)
{
    // Increment pool count up to the original maximum
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
    float Luck       = FMath::FRand(); // random float 0.0 - 1.0

    const float* Rarity = RarityTable[LevelIndex];

    // Walk through each cost tier adding weights until the roll is covered
    // Higher weight = larger slice of the 0-1 range = more likely to roll
    float CumulativeTotal = 0.f;
    int   Cost            = 1;

    for (int Tier = 0; Tier < 5; ++Tier)
    {
        CumulativeTotal += Rarity[Tier];
        if (CumulativeTotal >= Luck)
        {
            Cost = Tier;
            return Cost + 1; // array is 0-indexed, costs are 1-5
        }
    }

    return Cost;
}

UUnitDataAsset* UShopSubsystem::DrawFromPool(int32 PlayerLevel)
{
    // Check if anything is available before rolling
    TArray<int32> Counts;
    UnitPool.GenerateValueArray(Counts);
    bool bAnyAvailable = Counts.ContainsByPredicate([](int32 C){ return C > 0; });
    if (!bAnyAvailable) return nullptr;

    // Roll a cost tier based on player level rarity
    int32 Tier = PickCostTier(PlayerLevel);

    // Find all units of that tier with copies remaining
    TArray<UUnitDataAsset*> Candidates;
    for (UUnitDataAsset* UnitData : AllUnits)
    {
        if (!UnitData) continue;
        int32* Count = UnitPool.Find(UnitData->UnitName);
        if (UnitData->Cost == Tier && Count && *Count > 0)
            Candidates.Add(UnitData);
    }

    // If nothing available at this tier re-roll
    // Safe because pool exhaustion is caught above
    if (Candidates.IsEmpty())
        return DrawFromPool(PlayerLevel);

    // Pick a random candidate and remove one copy from the pool
    UUnitDataAsset* Picked = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
    UnitPool.FindOrAdd(Picked->UnitName)--;
    return Picked;
}

// -------------------------------------------------------
// Shop Generation
// -------------------------------------------------------

void UShopSubsystem::GenerateShop(int32 PlayerLevel)
{
    // Return any unsold units from the previous shop back to the pool
    for (const FShopSlot& ShopSlot : CurrentShop)
    {
        if (!ShopSlot.bIsPurchased && ShopSlot.Data)
            ReturnToPool(ShopSlot.Data->UnitName);
    }

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
    // Init pool on first call
    if (UnitPool.IsEmpty()) InitPool();
    GenerateShop(PlayerLevel);
}

bool UShopSubsystem::RefreshShop(int32 PlayerLevel)
{
    // Costs 2 gold to manually reroll
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

    // Slot must have a unit and not already be purchased
    if (Slot.bIsPurchased || !Slot.Data) return false;

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (ATFTPlayerState* PS = PC->GetPlayerState<ATFTPlayerState>())
        {
            // Try to spend gold equal to unit cost
            if (!PS->SpendGold(Slot.Data->Cost)) return false;

            // Spawn the unit at the bench position
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride =
                ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

            FVector BenchLocation = FVector(-800.f, SlotIndex * 200.f, 0.f);

            AUnit* PurchasedUnit = GetWorld()->SpawnActor<AUnit>(
                AUnit::StaticClass(), BenchLocation, FRotator::ZeroRotator, Params);

            if (!PurchasedUnit) return false;

            // Load stats and mesh from the data asset
            PurchasedUnit->DataAsset = Slot.Data;
            PurchasedUnit->InitFromDataAsset();

            // Add to bench and mark slot as sold
            PS->BenchUnits.Add(PurchasedUnit);
            CurrentShop[SlotIndex].bIsPurchased = true;

            OnUnitPurchased.Broadcast(PurchasedUnit);

            UE_LOG(LogTemp, Log, TEXT("Bought %s for %d gold | Gold remaining: %d"),
                *Slot.Data->UnitName.ToString(), Slot.Data->Cost, PS->Gold);

            return true;
        }
    }
    return false;
}

void UShopSubsystem::SellUnit(AUnit* Unit)
{
    if (!Unit) return;

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (ATFTPlayerState* PS = PC->GetPlayerState<ATFTPlayerState>())
        {
        
            // How many original copies went into this unit
            // 1 star = 1, 2 star = 3, 3 star = 9
            int32 Copies = FMath::Pow(3.f, Unit->StarLevel - 1);
           
            // Refund full cost — fallback to 1 if no data asset
            int32 Refund = Unit->DataAsset ? Unit->DataAsset->Cost* Copies  : 1;
            
            PS->AddGold(Refund);
            
            // Return all copies back to the pool
            for (int32 i = 0; i < Copies; i++)
            {
                ReturnToPool(Unit->UnitName);
            }

            // Remove from wherever the unit currently is
            PS->BenchUnits.Remove(Unit);
            PS->BoardUnits.Remove(Unit);

            Unit->Destroy();

            UE_LOG(LogTemp, Log, TEXT("Sold %s (Star %d) for %d gold | Returned %d copies to pool"),
                *Unit->UnitName.ToString(), Unit->StarLevel, Refund, Copies);
        }
    }
}