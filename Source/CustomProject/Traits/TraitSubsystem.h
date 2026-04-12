#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "TraitDefinition.h"
#include "Units/Unit.h"
#include "TraitSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FActiveTraitStatus
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag TraitTag;

	UPROPERTY(BlueprintReadOnly)
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 NextThreshold = 0; // 0 means max tier reached

	UPROPERTY(BlueprintReadOnly)
	bool bIsActive = false; // true if at least one threshold met
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTraitsUpdated);

UCLASS()
class CUSTOMPROJECT_API UTraitSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Call this whenever board composition changes
	// Pass in all units currently on the board
	UFUNCTION(BlueprintCallable, Category="Traits")
	void RecalculateTraits(const TArray<AUnit*>& BoardUnits);

	// Returns count of units with a specific trait tag
	UFUNCTION(BlueprintCallable, Category="Traits")
	int32 GetTraitCount(FGameplayTag TraitTag) const;

	// Returns all trait statuses — used by UI to display synergies
	UFUNCTION(BlueprintCallable, Category="Traits")
	TArray<FActiveTraitStatus> GetAllTraitStatuses() const;

	// The data asset containing all trait definitions
	// Set this in your GameMode or GameInstance Blueprint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Traits")
	TObjectPtr<class UTraitDataAsset> TraitData;

	// Fires whenever traits change — UI listens to this
	UPROPERTY(BlueprintAssignable)
	FOnTraitsUpdated OnTraitsUpdated;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	// Tag → how many board units have it
	TMap<FGameplayTag, int32> TraitCounts;

	void CountTraits(const TArray<AUnit*>& BoardUnits);
	void ApplyTraitBuffs(const TArray<AUnit*>& BoardUnits);
	void StripAllTraitBuffs(const TArray<AUnit*>& BoardUnits);
};