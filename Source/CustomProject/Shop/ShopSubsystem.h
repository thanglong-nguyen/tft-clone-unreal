#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Units/Unit.h"
#include "ShopSubsystem.generated.h"

class ATFTGameMode;
class UUnitDataAsset;

// Represents one slot in the shop
USTRUCT(BlueprintType)
struct FShopSlot
{
    GENERATED_BODY();

    // The unit available in this slot — null if slot is empty
    UPROPERTY(BlueprintReadOnly)
    UUnitDataAsset* Data = nullptr;

    // True once the player buys this slot
    UPROPERTY(BlueprintReadOnly)
    bool bIsPurchased = false;
};

// Fired when the shop refreshes — UI listens to redraw slots
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShopRefresh);

// Fired when a unit is purchased — passes the spawned unit
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitPurchased, AUnit*, NewUnit);

UCLASS()
class CUSTOMPROJECT_API UShopSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPROPERTY()
    ATFTGameMode* TFTGameMode;
    
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Number of slots shown in the shop each round
    static constexpr int32 ShopSize = 2;

    // Current shop slots — read by UI to display available units
    UPROPERTY(BlueprintReadOnly, Category="Shop")
    TArray<FShopSlot> CurrentShop;

    // Costs 2 gold — rerolls the shop mid round
    UFUNCTION(BlueprintCallable, Category="Shop")
    bool RefreshShop(int32 PlayerLevel);

    // Free reroll — called automatically at the start of each round
    UFUNCTION(BlueprintCallable, Category="Shop")
    void FreeRefresh(int32 PlayerLevel);

    // Buy a unit from a shop slot by index — spawns it to the bench
    UFUNCTION(BlueprintCallable, Category="Shop")
    bool BuyUnit(int32 SlotIndex);

    // Sell a unit — removes it from board or bench and refunds gold
    UFUNCTION(BlueprintCallable, Category="Shop")
    void SellUnit(AUnit* Unit);

    // All unit data assets in the game
    // Populate this in the editor or via CombatTestActor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shop")
    TArray<UUnitDataAsset*> AllUnits;

    // Broadcast when shop slots change
    UPROPERTY(BlueprintAssignable)
    FOnShopRefresh OnShopRefresh;

    // Broadcast when a unit is purchased
    UPROPERTY(BlueprintAssignable)
    FOnUnitPurchased OnUnitPurchased;

private:
    // Tracks how many copies of each unit are left in the shared pool
    TMap<FName, int32> UnitPool;

    // How many copies of each unit exist in the pool
    static constexpr int32 CopiesPerUnit = 15;

    // Probability of rolling each cost tier per player level
    // Rows = levels 1-8, Columns = cost tiers 1-5
    float RarityTable[8][5] = {
        {1.00f, 0.00f, 0.00f, 0.00f, 0.00f}, // Level 1
        {0.70f, 0.30f, 0.00f, 0.00f, 0.00f}, // Level 2
        {0.55f, 0.30f, 0.15f, 0.00f, 0.00f}, // Level 3
        {0.40f, 0.35f, 0.20f, 0.05f, 0.00f}, // Level 4
        {0.30f, 0.35f, 0.25f, 0.10f, 0.00f}, // Level 5
        {0.20f, 0.30f, 0.30f, 0.15f, 0.05f}, // Level 6
        {0.15f, 0.20f, 0.30f, 0.25f, 0.10f}, // Level 7
        {0.10f, 0.15f, 0.25f, 0.30f, 0.20f}, // Level 8
    };

    // Fills the pool with CopiesPerUnit copies of every unit in AllUnits
    void InitPool();

    // Picks a random unit from the pool weighted by PlayerLevel rarity
    UUnitDataAsset* DrawFromPool(int32 PlayerLevel);

    // Uses the rarity table to pick a cost tier based on a random roll
    int32 PickCostTier(int32 PlayerLevel) const;

    // Returns a unit copy back to the pool when unsold or sold
    void ReturnToPool(FName UnitName);

    // Generates fresh shop slots — called by both refresh functions
    void GenerateShop(int32 PlayerLevel);
};