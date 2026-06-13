#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "UnitDataAsset.generated.h"

// Stats for one star level of a unit.
// Each unit has an array of these — one per star level (1, 2, 3).
USTRUCT(BlueprintType)
struct FUnitStarTier
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 StarLevel = 1;

    UPROPERTY(EditAnywhere, Category="Stats")
    float BaseHP = 500.f;

    UPROPERTY(EditAnywhere, Category="Stats")
    float BaseAttack = 50.f;

    UPROPERTY(EditAnywhere, Category="Stats")
    float BaseArmor = 20.f;

    UPROPERTY(EditAnywhere, Category="Stats")
    float AttackRange = 200.f;

    UPROPERTY(EditAnywhere, Category="Stats")
    float AttackSpeed = 1.0f;
};

// Data asset defining a unit type — stats, traits, mesh, and cost.
// One asset per unit type e.g. DA_HumanWarrior, DA_ElfArcher.
// Assign to a unit actor's DataAsset property to configure it.
UCLASS()
class CUSTOMPROJECT_API UUnitDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // -------------------------------------------------------
    // Identity
    // -------------------------------------------------------

    UPROPERTY(EditAnywhere, Category="Identity")
    FName UnitName;

    UPROPERTY(EditAnywhere, Category="Identity")
    int32 Cost = 1; // shop cost tier 1-5

    UPROPERTY(EditAnywhere, Category="Stats")
    float MoveSpeed = 300.f;

    // -------------------------------------------------------
    // Star Tiers
    // One entry per star level — index by StarLevel value
    // -------------------------------------------------------

    UPROPERTY(EditAnywhere, Category="StarLevels")
    TArray<FUnitStarTier> StarTiers;

    // Returns stats for the given star level — null if not found
    const FUnitStarTier* GetStarTier(int32 StarLevel) const
    {
        for (const FUnitStarTier& Tier : StarTiers)
            if (Tier.StarLevel == StarLevel)
                return &Tier;
        return nullptr;
    }

    // -------------------------------------------------------
    // Traits — used by TraitSubsystem for synergy calculation
    // -------------------------------------------------------

    UPROPERTY(EditAnywhere, Category="Traits")
    FGameplayTag Race;  // e.g. Race.Elf

    UPROPERTY(EditAnywhere, Category="Traits")
    FGameplayTag Class; // e.g. Class.Warrior

    // Returns both trait tags together for counting in TraitSubsystem
    FGameplayTagContainer GetAllTraitTags() const
    {
        FGameplayTagContainer Tags;
        Tags.AddTag(Race);
        Tags.AddTag(Class);
        return Tags;
    }

    // -------------------------------------------------------
    // Visuals
    // -------------------------------------------------------

    UPROPERTY(EditAnywhere, Category="Visuals")
    USkeletalMesh* Mesh;
};