#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "TraitSubsystem.generated.h"

class ATFTGameMode;
class AUnit;

// Snapshot of one trait's current state
// Built every time the board changes — read by UI to draw synergy panel
USTRUCT(BlueprintType)
struct FActiveTraitStatus
{
    GENERATED_BODY()

    // The gameplay tag identifying this trait e.g. Race.Elf, Class.Warrior
    UPROPERTY(BlueprintReadOnly)
    FGameplayTag TraitTag;
    
    UPROPERTY(BlueprintReadOnly)
    FString DisplayName;
    
    UPROPERTY(BlueprintReadOnly)
    int32 CurrentCount = 0;

    // True if CurrentCount meets at least the lowest threshold
    UPROPERTY(BlueprintReadOnly)
    bool bIsActive = false;

    // The next threshold the player is working toward
    // 0 means already at the highest tier
    UPROPERTY(BlueprintReadOnly)
    int32 NextThreshold = 0;
};

// Fired whenever RecalculateTraits runs — UI listens to refresh synergy panel
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTraitsUpdated);

UCLASS()
class CUSTOMPROJECT_API UTraitSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    
    UPROPERTY()
    ATFTGameMode* TFTGameMode;
    
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // -------------------------------------------------------
    // Data
    // -------------------------------------------------------
    
    
    // Contains all trait definitions and their tier thresholds
    UPROPERTY(BlueprintReadWrite, Category="Traits")
    TObjectPtr<class UTraitDataAsset> TraitData;

    // -------------------------------------------------------
    // Public Interface
    // -------------------------------------------------------

    // Call this whenever the board changes — places, removes, sells
    // Strips old buffs, recounts traits, reapplies fresh buffs
    UFUNCTION(BlueprintCallable, Category="Traits")
    void RecalculateTraits(const TArray<AUnit*>& BoardUnits);

    // Returns how many unique units on the board have a specific trait
    UFUNCTION(BlueprintCallable, Category="Traits")
    int32 GetTraitCount(FGameplayTag TraitTag) const;

    // Returns the full status of every trait — used by UI synergy panel
    UFUNCTION(BlueprintCallable, Category="Traits")
    TArray<FActiveTraitStatus> GetAllTraitStatuses() const;

    // -------------------------------------------------------
    // Delegates
    // -------------------------------------------------------

    // Broadcast after every RecalculateTraits — bind to update UI
    UPROPERTY(BlueprintAssignable, Category="Traits")
    FOnTraitsUpdated OnTraitsUpdated;

private:

    TMap<FGameplayTag, int32> TraitCounts;

    // Counts unique trait contributions from board units
    // Uses a TSet to prevent duplicate unit types from counting twice
    void CountTraits(const TArray<AUnit*>& BoardUnits);

    // Applies stat bonuses to units whose trait meets a threshold
    void ApplyTraitBuffs(const TArray<AUnit*>& BoardUnits);

    // Resets all board units to base stats 
    // Called before reapplying buffs to prevent stacking
    void StripTraitBuffs(const TArray<AUnit*>& BoardUnits);
};