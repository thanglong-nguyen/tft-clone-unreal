#include "Traits/TraitSubsystem.h"
#include "Traits/TraitDataAsset.h"
#include "Traits/TraitDefinition.h"
#include "Units/Unit.h"
#include "Units/UnitDataAsset.h"

// -------------------------------------------------------
// Initialization
// -------------------------------------------------------

void UTraitSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Auto load the trait definitions asset on startup
    TraitData = Cast<UTraitDataAsset>(StaticLoadObject(
        UTraitDataAsset::StaticClass(),
        nullptr,
        TEXT("/Game/DA_Traits.DA_Traits")
    ));
}

// -------------------------------------------------------
// Core Flow
// -------------------------------------------------------

void UTraitSubsystem::RecalculateTraits(const TArray<AUnit*>& BoardUnits)
{
    if (!TraitData) return;
    
    // 1. Reset stats to base so old buffs don't stack
    // 2. Rebuild counts from scratch
    // 3. Apply fresh buffs based on new counts
    
    StripTraitBuffs(BoardUnits);
    
    TraitCounts.Empty();
    
    CountTraits(BoardUnits);
    
    ApplyTraitBuffs(BoardUnits);

    // Tell UI and any other listeners to refresh
    OnTraitsUpdated.Broadcast();
}

// -------------------------------------------------------
// Private Helpers
// -------------------------------------------------------

void UTraitSubsystem::CountTraits(const TArray<AUnit*>& BoardUnits)
{
    // Track which unit names have already been counted
    TSet<FName> CountedNames;

    for (AUnit* Unit : BoardUnits)
    {
        // Skip invalid, dead, or already counted units
        if (!Unit || Unit->IsDead()) continue;
        if (CountedNames.Contains(Unit->UnitName)) continue;
        if (!Unit->DataAsset) continue;
        
        
        CountedNames.Add(Unit->UnitName);

        // Increment count for race and class tags
        FGameplayTag RaceTag  = Unit->DataAsset->Race;
        FGameplayTag ClassTag = Unit->DataAsset->Class;

        TraitCounts.FindOrAdd(RaceTag)++;
        TraitCounts.FindOrAdd(ClassTag)++;
    }
}

void UTraitSubsystem::ApplyTraitBuffs(const TArray<AUnit*>& BoardUnits)
{
    for (auto& Trait : TraitCounts)
    {
        FGameplayTag Tag  = Trait.Key;
        int32        Count = Trait.Value;

        // Look up this trait's definition in the data asset
        const FTraitDefinition* TraitDef = TraitData->FindTrait(Tag);
        if (!TraitDef) continue;

        // Get the highest tier this count qualifies for
        const FTraitTier* Tier = TraitDef->GetActiveTier(Count);
        if (!Tier) continue; // threshold not met — no buff to apply

        // Apply bonuses to every board unit that has this trait
        for (AUnit* Unit : BoardUnits)
        {
            if (!Unit || !Unit->DataAsset) continue;

            if (Unit->DataAsset->GetAllTraitTags().HasTag(Tag))
            {
                Unit->MaxHP        += Tier->BonusHP;
                Unit->Armor        += Tier->BonusArmor;
                Unit->AttackDamage += Tier->BonusAttack;
                Unit->AttackSpeed  += Tier->BonusAttackSpeed;
            }
        }
    }
}

void UTraitSubsystem::StripTraitBuffs(const TArray<AUnit*>& BoardUnits)
{
    // Reset every unit to base stats from their data asset
    for (AUnit* Unit : BoardUnits)
    {
        if (Unit) Unit->InitFromDataAsset();
    }
}

// -------------------------------------------------------
// Queries
// -------------------------------------------------------

int32 UTraitSubsystem::GetTraitCount(FGameplayTag TraitTag) const
{
    const int32* Count = TraitCounts.Find(TraitTag);
    return Count ? *Count : 0;
}

TArray<FActiveTraitStatus> UTraitSubsystem::GetAllTraitStatuses() const
{
    TArray<FActiveTraitStatus> Result;
    if (!TraitData) return Result;

    for (const FTraitDefinition& TraitDef : TraitData->Traits)
    {
        FActiveTraitStatus Status;
        Status.TraitTag     = TraitDef.TraitTag;
        Status.DisplayName  = TraitDef.DisplayName;
        Status.CurrentCount = GetTraitCount(TraitDef.TraitTag);

        // Active if at least the lowest tier threshold is met
        Status.bIsActive = (TraitDef.GetActiveTier(Status.CurrentCount) != nullptr);

        // Find the next threshold the player hasn't reached yet
        Status.NextThreshold = 0; // 0 means already at max tier
        for (const FTraitTier& Tier : TraitDef.Tiers)
        {
            if (Status.CurrentCount < Tier.RequiredCount)
            {
                Status.NextThreshold = Tier.RequiredCount;
                break; 
            }
        }

        Result.Add(Status);
    }

    return Result;
}