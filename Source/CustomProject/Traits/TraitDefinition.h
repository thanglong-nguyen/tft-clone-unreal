#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "TraitDefinition.generated.h"

// One threshold tier e.g. "3 Elves = +15% dodge"
USTRUCT(BlueprintType)
struct FTraitTier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RequiredCount = 2;

	// Stat bonuses applied to ALL units with this trait
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BonusHP = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BonusAttack = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BonusArmor = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BonusAttackSpeed = 0.f;

	// Human readable description for UI
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Description = "";
};

// Full definition of one trait e.g. "Warrior"
USTRUCT(BlueprintType)
struct FTraitDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag TraitTag; // e.g. Class.Warrior

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString DisplayName = "";

	// Sorted ascending by RequiredCount
	// e.g. [{2, +20 attack}, {4, +50 attack}, {6, +100 attack}]
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FTraitTier> Tiers;

	// Returns the highest active tier for a given unit count
	// Returns nullptr if count doesn't meet any threshold
	const FTraitTier* GetActiveTier(int32 UnitCount) const
	{
		const FTraitTier* Active = nullptr;
		for (const FTraitTier& Tier : Tiers)
		{
			if (UnitCount >= Tier.RequiredCount)
				Active = &Tier;
		}
		return Active;
	}
};