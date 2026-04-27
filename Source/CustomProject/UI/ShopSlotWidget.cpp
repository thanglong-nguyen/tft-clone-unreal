#include "UI/ShopSlotWidget.h"
#include "Shop/ShopSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UShopSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Wire buy button click to purchase logic
	if (BuyButton)
		BuyButton->OnClicked.AddDynamic(this, &UShopSlotWidget::OnBuyClicked);
}

void UShopSlotWidget::RefreshSlot(const FName& UnitName, int32 Cost, bool bPurchased)
{
	// Update unit name display
	if (UnitNameText)
		UnitNameText->SetText(FText::FromString(UnitName.ToString()));

	// Update cost display
	if (CostText)
		CostText->SetText(FText::FromString(FString::Printf(TEXT("%dg"), Cost)));

	// Toggle sold state — show SOLD text and disable button if purchased
	if (SoldText)
		SoldText->SetVisibility(bPurchased
			? ESlateVisibility::Visible
			: ESlateVisibility::Hidden);

	if (BuyButton)
		BuyButton->SetIsEnabled(!bPurchased); 
}

void UShopSlotWidget::OnBuyClicked()
{
	// Purchase attempt — HUD refreshes automatically via OnShopRefresh delegate
	UShopSubsystem* Shop = GetGameInstance()->GetSubsystem<UShopSubsystem>();
	if (Shop) Shop->BuyUnit(SlotIndex);
}