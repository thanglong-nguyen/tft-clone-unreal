#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TraitDefinition.h"
#include "TraitDataAsset.generated.h"

UCLASS()
class CUSTOMPROJECT_API UTraitDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// All trait definitions live here — races and classes together
	UPROPERTY(EditAnywhere, Category="Traits")
	TArray<FTraitDefinition> Traits;

	// Quick lookup by tag
	const FTraitDefinition* FindTrait(FGameplayTag Tag) const
	{
		for (const FTraitDefinition& Def : Traits)
		{
			if (Def.TraitTag == Tag)
				return &Def;
		}
		return nullptr;
	}
};