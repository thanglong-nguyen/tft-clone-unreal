#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopSlotWidget.generated.h"

UCLASS()
class CUSTOMPROJECT_API UShopSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Set this before adding to viewport
	UPROPERTY(BlueprintReadWrite, Category="Shop")
	int32 SlotIndex = 0;

	// Call from Blueprint to populate the slot visually
	UFUNCTION(BlueprintImplementableEvent, Category="Shop")
	void SetSlotData(const FString& UnitName, int32 Cost, bool bPurchased);

	// Called when buy button is clicked
	UFUNCTION(BlueprintCallable, Category="Shop")
	void OnBuyClicked();
};