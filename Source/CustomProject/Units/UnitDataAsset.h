#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "UnitDataAsset.generated.h"


USTRUCT(BlueprintType)
struct FUnitStarTier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 StarLevel = 1;

	// --- Stats ---
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

UCLASS()
class CUSTOMPROJECT_API UUnitDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// --- Identity ---
	UPROPERTY(EditAnywhere, Category="Identity")
	FName UnitName;

	UPROPERTY(EditAnywhere, Category="Identity")
	int32 Cost = 1; // 1-5 like TFT
	
	UPROPERTY(EditAnywhere, Category="Stats")
	float MoveSpeed = 300.f;
	
	// Stats for each star level — index 0 = 1 star, index 1 = 2 star etc
	UPROPERTY(EditAnywhere, Category="StarLevels")
	TArray<FUnitStarTier> StarTiers;
	
	// Helper — returns stats for a given star level
	const FUnitStarTier* GetStarTier(int32 StarLevel) const
	{
		for (const FUnitStarTier& Tier : StarTiers)
		{
			if (Tier.StarLevel == StarLevel)
				return &Tier;
		}
		return nullptr;
	}
	
	// --- Identity Tags ---
	// A unit has exactly one race
	UPROPERTY(EditAnywhere, Category="Traits")
	FGameplayTag Race; // e.g. Race.Elf

	// A unit has exactly one class
	UPROPERTY(EditAnywhere, Category="Traits")
	FGameplayTag Class; // e.g. Class.Warrior
	
	// Helper — returns both tags together for trait counting
	FGameplayTagContainer GetAllTraitTags() const
	{
		FGameplayTagContainer Tags;
		Tags.AddTag(Race);
		Tags.AddTag(Class);
		return Tags;
	}

	// --- Visuals ---
	UPROPERTY(EditAnywhere, Category="Visuals")
	TObjectPtr<USkeletalMesh> Mesh;
};