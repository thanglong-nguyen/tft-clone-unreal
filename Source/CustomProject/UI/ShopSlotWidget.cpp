#include "UI/ShopSlotWidget.h"
#include "Shop/ShopSubsystem.h"

void UShopSlotWidget::OnBuyClicked()
{
	UShopSubsystem* Shop = GetGameInstance()->GetSubsystem<UShopSubsystem>();
	if (!Shop) return;

	if (Shop->BuyUnit(SlotIndex))
	{
		// Slot data will update via OnShopUpdated in HUD
		UE_LOG(LogTemp, Log, TEXT("ShopSlot %d: buy clicked"), SlotIndex);
	}
}