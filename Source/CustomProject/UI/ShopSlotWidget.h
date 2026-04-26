#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopSlotWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class CUSTOMPROJECT_API UShopSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// Which shop index this slot represents
	UPROPERTY(BlueprintReadWrite, Category="Shop")
	int32 SlotIndex = 0;

	// Called by HUD to update this slot's display
	// UnitName — name to show, empty if slot is empty
	// Cost     — gold cost to show
	// bPurchased — true if already bought this round
	void RefreshSlot(const FName& UnitName, int32 Cost, bool bPurchased);

	// -------------------------------------------------------
	// Widget bindings — names must match exactly in WBP_ShopSlot
	// -------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* UnitNameText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* CostText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* SoldText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UButton* BuyButton;

private:
	UFUNCTION()
	void OnBuyClicked();
};