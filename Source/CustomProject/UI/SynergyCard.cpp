#include "UI/SynergyCard.h"
#include "Components/TextBlock.h"

void USynergyCard::SetData(const FString& TraitName, int32 Current,
							int32 NextThreshold, bool bActive)
{
	if (TraitNameText)
		TraitNameText->SetText(FText::FromString(TraitName));

	if (CountText)
	{
		// Show "X / MAX" when player has reached the highest tier
		FString CountStr = NextThreshold > 0
			? FString::Printf(TEXT("%d / %d"), Current, NextThreshold)
			: FString::Printf(TEXT("%d / MAX"), Current);
		CountText->SetText(FText::FromString(CountStr));
	}

	if (StatusText)
	{
		FString StatusStr = bActive
			? TEXT("ACTIVE")
			: FString::Printf(TEXT("%d more"), NextThreshold - Current);
		StatusText->SetText(FText::FromString(StatusStr));

		// Gold if active, grey if threshold not yet met
		StatusText->SetColorAndOpacity(bActive
			? FSlateColor(FLinearColor(1.f, 0.85f, 0.f))
			: FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)));
	}
}