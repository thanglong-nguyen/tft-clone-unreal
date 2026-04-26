#include "UI/ShopSlotWidget.h"
#include "Shop/ShopSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UShopSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// TODO: Bind BuyButton OnClicked to OnBuyClicked
	// HINT: BuyButton->OnClicked.AddDynamic(this, &UShopSlotWidget::OnBuyClicked)
	
	BuyButton->OnClicked.AddDynamic(this, &UShopSlotWidget::OnBuyClicked);
}

void UShopSlotWidget::RefreshSlot(const FName& UnitName, int32 Cost, bool bPurchased)
{
	// TODO: Set UnitNameText to UnitName
	// TODO: Set CostText — format as "{Cost}g"
	// TODO: If bPurchased — show SoldText, disable BuyButton
	// TODO: If not purchased — hide SoldText, enable BuyButton
	// HINT: SetVisibility(ESlateVisibility::Visible/Hidden)
	// HINT: BuyButton->SetIsEnabled(bool)
	
	FString Name = FString::Printf(TEXT("%s"), UnitName); 
	UnitNameText->SetText(FText::FromString(Name));
	
	FString ShopCost = FString::Printf(TEXT("%dg"), Cost); 
	CostText->SetText(FText::FromString(ShopCost));
	
	if (bPurchased)
	{
		SoldText->SetVisibility(ESlateVisibility::Visible);
		BuyButton->SetIsEnabled(false);
	}
	
	SoldText->SetVisibility(ESlateVisibility::Hidden);
	BuyButton->SetIsEnabled(true);
	
}

void UShopSlotWidget::OnBuyClicked()
{
	// TODO: Get ShopSubsystem
	// TODO: Call BuyUnit(SlotIndex)
	// HINT: After buying the HUD will refresh via OnShopRefresh delegate
	//       so you don't need to manually update visuals here
	
	UGameInstance* GI = GetGameInstance();
	UShopSubsystem* ShopSS   = GI->GetSubsystem<UShopSubsystem>();
	
	ShopSS->BuyUnit(SlotIndex);
	
}