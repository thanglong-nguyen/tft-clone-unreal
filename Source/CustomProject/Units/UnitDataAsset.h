#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "UnitDataAsset.generated.h"

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

	UPROPERTY(EditAnywhere, Category="Stats")
	float MoveSpeed = 300.f;

	// --- Traits ---
	// e.g. "Trait.Warrior", "Trait.Mage"
	UPROPERTY(EditAnywhere, Category="Traits")
	FGameplayTagContainer Traits;

	// --- Visuals ---
	UPROPERTY(EditAnywhere, Category="Visuals")
	TObjectPtr<USkeletalMesh> Mesh;
};