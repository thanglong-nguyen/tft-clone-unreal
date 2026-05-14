#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SynergyCard.generated.h"

class UTextBlock;
class UProgressBar;

UCLASS()
class CUSTOMPROJECT_API USynergyCard : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetData(const FString& TraitName, int32 Current, 
				 int32 NextThreshold, bool bActive);

	// -------------------------------------------------------
	// Widget Bindings — match names in WBP_SynergyCard
	// -------------------------------------------------------

	// e.g. "Warrior"
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* TraitNameText;

	// e.g. "2 / 4"
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* CountText;

	// "ACTIVE" or "2 more"
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* StatusText;
};