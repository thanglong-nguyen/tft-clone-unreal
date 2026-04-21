#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "TraitSubsystem.generated.h"

class AUnit;
// Represents the current status of one trait
// Used by UI to display synergy panel
USTRUCT(BlueprintType)
struct FActiveTraitStatus
{
    GENERATED_BODY()

    // The trait tag e.g. Race.Elf or Class.Warrior
    UPROPERTY(BlueprintReadOnly)
    FGameplayTag TraitTag;

    // Human readable name e.g. "Elf", "Warrior"
    UPROPERTY(BlueprintReadOnly)
    FString DisplayName;

    // How many unique units on board have this trait
    UPROPERTY(BlueprintReadOnly)
    int32 CurrentCount = 0;

    // True if at least one threshold is met
    UPROPERTY(BlueprintReadOnly)
    bool bIsActive = false;

    // Next threshold the player is working toward
    // 0 means already at max tier
    UPROPERTY(BlueprintReadOnly)
    int32 NextThreshold = 0;
};

// Fired whenever board composition changes and traits are recalculated
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTraitsUpdated);

UCLASS()
class CUSTOMPROJECT_API UTraitSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    
    // The data asset containing all trait definitions
    // TODO: Load this in Initialize using StaticLoadObject
    // HINT: Similar to how TraitSubsystem loaded DA_Traits before
    UPROPERTY(BlueprintReadWrite, Category="Traits")
    TObjectPtr<class UTraitDataAsset> TraitData;

    // Call this whenever a unit is placed or removed from the board
    // Pass in ALL units currently on the board
    UFUNCTION(BlueprintCallable, Category="Traits")
    void RecalculateTraits(const TArray<AUnit*>& BoardUnits);

    // Returns how many unique units have a specific trait tag
    UFUNCTION(BlueprintCallable, Category="Traits")
    int32 GetTraitCount(FGameplayTag TraitTag) const;

    // Returns status of all traits — used by UI synergy panel
    UFUNCTION(BlueprintCallable, Category="Traits")
    TArray<FActiveTraitStatus> GetAllTraitStatuses() const;

    // Fired when traits change — UI binds to this
    UPROPERTY(BlueprintAssignable, Category="Traits")
    FOnTraitsUpdated OnTraitsUpdated;

private:

    TMap<FGameplayTag, int32> TraitCounts;
    
    
    void CountTraits(const TArray<AUnit*>& BoardUnits);
    
    
    void ApplyTraitBuffs(const TArray<AUnit*>& BoardUnits);

    void StripTraitBuffs(const TArray<AUnit*>& BoardUnits);
};