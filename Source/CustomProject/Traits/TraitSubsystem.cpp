#include "Traits/TraitSubsystem.h"
#include "Traits/TraitDataAsset.h"
#include "Traits/TraitDefinition.h"
#include "Units/Unit.h"
#include "Units/UnitDataAsset.h"


void UTraitSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    TraitData = Cast<UTraitDataAsset>(StaticLoadObject(
        UTraitDataAsset::StaticClass(),
        nullptr,
        TEXT("/Game/DA_Traits.DA_Traits")
));

    if (TraitData)
        UE_LOG(LogTemp, Log, TEXT("TraitSubsystem: DA_Traits loaded"))
    else
        UE_LOG(LogTemp, Error, TEXT("TraitSubsystem: DA_Traits not found — check path"))
}

void UTraitSubsystem::RecalculateTraits(const TArray<AUnit*>& BoardUnits)
{

    if (!TraitData)
    {
        return;
    }
    
    StripTraitBuffs(BoardUnits); 

    TraitCounts.Empty();

    CountTraits(BoardUnits);
    
    ApplyTraitBuffs(BoardUnits);
    
    OnTraitsUpdated.Broadcast();
}

void UTraitSubsystem::CountTraits(const TArray<AUnit*>& BoardUnits)
{
    
    TSet<FName> CountedNames;
    
    for (AUnit* Unit : BoardUnits)
    {
        if (!Unit || Unit->IsDead() || CountedNames.Contains(Unit->UnitName))
        {
            continue;
        }
        CountedNames.Add(Unit->UnitName);
        
        if (!Unit->DataAsset) continue; // add this check

        FGameplayTag RaceTag = Unit->DataAsset->Race;
        FGameplayTag ClassTag = Unit->DataAsset->Class;

        TraitCounts.FindOrAdd(RaceTag)++;
        TraitCounts.FindOrAdd(ClassTag)++;
    }
    
    
}

void UTraitSubsystem::ApplyTraitBuffs(const TArray<AUnit*>& BoardUnits)
{
    
    for (auto& Trait : TraitCounts)
    {
        FGameplayTag Tag = Trait.Key;
        int32 Count = Trait.Value;
        const FTraitDefinition* TraitDef = TraitData->FindTrait(Tag);
        if (!TraitDef) continue;
        
        const FTraitTier* Tier = TraitDef->GetActiveTier(Count);
        
        if (Tier == nullptr)
        {
            continue;
        }
        
        for (AUnit* Unit : BoardUnits)
        {
            if (!Unit || !Unit->DataAsset) continue;
            if (Unit->DataAsset->GetAllTraitTags().HasTag(Tag))
            {
                Unit->MaxHP += Tier->BonusHP;
                Unit->Armor += Tier->BonusArmor;
                Unit->AttackDamage += Tier->BonusAttack;
                Unit->AttackSpeed += Tier->BonusAttackSpeed;
            }
        }
    }
}

void UTraitSubsystem::StripTraitBuffs(const TArray<AUnit*>& BoardUnits)
{
    
    for (AUnit* Unit : BoardUnits)
    {
        if (Unit) Unit->InitFromDataAsset();
    }
}

int32 UTraitSubsystem::GetTraitCount(FGameplayTag TraitTag) const
{
    
    const int32* Count = TraitCounts.Find(TraitTag);
    return Count ? *Count : 0;
}

TArray<FActiveTraitStatus> UTraitSubsystem::GetAllTraitStatuses() const
{
    // TODO: Return early with empty array if TraitData is null
    
    TArray<FActiveTraitStatus> Result;
    
    if (!TraitData)
    {
        return Result;
    }

    // TODO: Loop through all trait definitions in TraitData->Traits
    //   → Create an FActiveTraitStatus for each
    //   → Set TraitTag, DisplayName, CurrentCount
    //   → Set bIsActive based on whether GetActiveTier returns non-null
    //   → Set NextThreshold by finding the first tier above CurrentCount
    //   → Add to result array
    
    for (const FTraitDefinition& TraitDef: TraitData->Traits)
    {
        FActiveTraitStatus Status;
        Status.TraitTag = TraitDef.TraitTag;
        Status.DisplayName  = TraitDef.DisplayName; 
        Status.CurrentCount = GetTraitCount(TraitDef.TraitTag);
        
        const FTraitTier* Tier = TraitDef.GetActiveTier(Status.CurrentCount);
        
        Status.bIsActive = (Tier != nullptr);
        Status.NextThreshold = 0;
        
        for (const FTraitTier& ATier :TraitDef.Tiers)
        {
            if (Status.CurrentCount < ATier.RequiredCount)
            {
                Status.NextThreshold  = ATier.RequiredCount;
                break;
            } 
        }
        
        Result.Add(Status);
    }
    
    return Result;
}

