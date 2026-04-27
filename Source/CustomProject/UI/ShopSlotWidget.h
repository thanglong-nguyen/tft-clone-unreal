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

	// Index into ShopSubsystem::CurrentShop
	// Set this on each slot widget in UMG before it can buy correctly
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Shop")
	int32 SlotIndex = 0;

	// Called by TFTHUDWidget::RefreshShopSlots every time shop changes
	void RefreshSlot(const FName& UnitName, int32 Cost, bool bPurchased);

	// -------------------------------------------------------
	// Widget Bindings
	// Names must match exactly in WBP_ShopSlot
	// -------------------------------------------------------

	// Shows the unit name
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* UnitNameText;

	// Shows the gold cost e.g. "2g"
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* CostText;

	// Shown when slot is already purchased — hidden otherwise
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* SoldText;

	// Disabled after purchase
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UButton* BuyButton;

private:
	// Bound to BuyButton.OnClicked in NativeConstruct
	UFUNCTION()
	void OnBuyClicked();
};