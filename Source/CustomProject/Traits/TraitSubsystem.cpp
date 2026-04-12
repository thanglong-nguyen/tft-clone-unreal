#include "Traits/TraitSubsystem.h"
#include "Traits/TraitDataAsset.h"
#include "Units/Unit.h"
#include "Units/UnitDataAsset.h"

void UTraitSubsystem::RecalculateTraits(const TArray<AUnit*>& BoardUnits)
{
    if (!TraitData) 
    {
        UE_LOG(LogTemp, Warning, TEXT("TraitSubsystem: No TraitData asset assigned!"));
        return;
    }

    // Strip old buffs before recalculating
    StripAllTraitBuffs(BoardUnits);

    // Recount from scratch
    TraitCounts.Empty();
    CountTraits(BoardUnits);

    // Apply new buffs based on fresh counts
    ApplyTraitBuffs(BoardUnits);

    // Tell UI something changed
    OnTraitsUpdated.Broadcast();
}

void UTraitSubsystem::CountTraits(const TArray<AUnit*>& BoardUnits)
{
    // Track which unit NAMES we've already counted
    // so duplicates don't contribute extra trait counts
    TSet<FName> CountedUnitNames;

    for (AUnit* Unit : BoardUnits)
    {
        if (!Unit || Unit->IsDead() || !Unit->DataAsset) continue;

        FName UnitName = Unit->DataAsset->UnitName;

        // Skip if we already counted a unit with this name
        if (CountedUnitNames.Contains(UnitName)) continue;
        CountedUnitNames.Add(UnitName);

        // Count race
        FGameplayTag RaceTag = Unit->DataAsset->Race;
        if (RaceTag.IsValid())
        {
            int32& Count = TraitCounts.FindOrAdd(RaceTag);
            Count++;
        }

        // Count class
        FGameplayTag ClassTag = Unit->DataAsset->Class;
        if (ClassTag.IsValid())
        {
            int32& Count = TraitCounts.FindOrAdd(ClassTag);
            Count++;
        }
    }
}

void UTraitSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Auto load DA_Traits on startup
    TraitData = Cast<UTraitDataAsset>(StaticLoadObject(
        UTraitDataAsset::StaticClass(),
        nullptr,
        TEXT("/Script/CustomProject.TraitDataAsset'/Game/DA_Traits.DA_Traits'")
    ));

    if (TraitData)
        UE_LOG(LogTemp, Log, TEXT("TraitSubsystem: DA_Traits loaded successfully"))
    else
        UE_LOG(LogTemp, Error, TEXT("TraitSubsystem: Could not find DA_Traits — check the path"));
}   

void UTraitSubsystem::ApplyTraitBuffs(const TArray<AUnit*>& BoardUnits)
{
    for (auto& Pair : TraitCounts)
    {
        FGameplayTag Tag       = Pair.Key;
        int32        Count     = Pair.Value;

        const FTraitDefinition* Def = TraitData->FindTrait(Tag);
        if (!Def) continue;

        const FTraitTier* ActiveTier = Def->GetActiveTier(Count);
        if (!ActiveTier) continue; // threshold not met

        // Apply buff to every board unit that has this trait
        for (AUnit* Unit : BoardUnits)
        {
            if (!Unit || Unit->IsDead() || !Unit->DataAsset) continue;

            FGameplayTagContainer UnitTags = Unit->DataAsset->GetAllTraitTags();
            if (!UnitTags.HasTag(Tag)) continue;

            // Apply the tier bonuses on top of base stats
            Unit->MaxHP        += ActiveTier->BonusHP;
            Unit->CurrentHP    += ActiveTier->BonusHP;
            Unit->AttackDamage += ActiveTier->BonusAttack;
            Unit->Armor        += ActiveTier->BonusArmor;
            Unit->AttackSpeed  += ActiveTier->BonusAttackSpeed;

            UE_LOG(LogTemp, Log, TEXT("Applied %s buff to %s"), 
                *Tag.ToString(), *Unit->UnitName.ToString());
        }
    }
}

void UTraitSubsystem::StripAllTraitBuffs(const TArray<AUnit*>& BoardUnits)
{
    // Easiest approach — just reinitialise from data asset
    // This resets stats to base, then buffs get reapplied fresh
    for (AUnit* Unit : BoardUnits)
    {
        if (!Unit || !Unit->DataAsset) continue;
        Unit->InitFromDataAsset();
    }
}

int32 UTraitSubsystem::GetTraitCount(FGameplayTag TraitTag) const
{
    const int32* Count = TraitCounts.Find(TraitTag);
    return Count ? *Count : 0;
}

TArray<FActiveTraitStatus> UTraitSubsystem::GetAllTraitStatuses() const
{
    TArray<FActiveTraitStatus> Result;
    if (!TraitData) return Result;

    for (const FTraitDefinition& Def : TraitData->Traits)
    {
        FActiveTraitStatus Status;
        Status.TraitTag    = Def.TraitTag;
        Status.DisplayName = Def.DisplayName;
        Status.CurrentCount= GetTraitCount(Def.TraitTag);
        Status.bIsActive   = Def.GetActiveTier(Status.CurrentCount) != nullptr;

        // Find next threshold
        Status.NextThreshold = 0;
        for (const FTraitTier& Tier : Def.Tiers)
        {
            if (Tier.RequiredCount > Status.CurrentCount)
            {
                Status.NextThreshold = Tier.RequiredCount;
                break;
            }
        }

        Result.Add(Status);
    }

    return Result;
}