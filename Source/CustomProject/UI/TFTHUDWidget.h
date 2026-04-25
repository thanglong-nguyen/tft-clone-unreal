#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Combat/CombatSubsystem.h"
#include "TFTHUDWidget.generated.h"

UCLASS()
class CUSTOMPROJECT_API UTFTHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;

protected:
	// -------------------------------------------------------
	// Blueprint Implementable Events
	// These are the functions you implement visually in UMG
	// -------------------------------------------------------

	// Called when gold changes — update gold text here
	UFUNCTION(BlueprintImplementableEvent, Category="HUD")
	void OnGoldUpdated(int32 NewGold);

	// Called when level or XP changes
	UFUNCTION(BlueprintImplementableEvent, Category="HUD")
	void OnLevelUpdated(int32 NewLevel, int32 XPCurrent, int32 XPNeeded);

	// Called when round number changes
	UFUNCTION(BlueprintImplementableEvent, Category="HUD")
	void OnRoundUpdated(int32 NewRound);

	// Called every tick during prep phase
	UFUNCTION(BlueprintImplementableEvent, Category="HUD")
	void OnTimerUpdated(float TimeRemaining);

	// Called when phase changes — e.g. show COMBAT banner
	UFUNCTION(BlueprintImplementableEvent, Category="HUD")
	void OnPhaseUpdated(EGamePhase NewPhase);

	// Called when shop refreshes — rebuild shop slot widgets
	UFUNCTION(BlueprintImplementableEvent, Category="HUD")
	void OnShopUpdated();

private:
	void BindDelegates();

	UFUNCTION()
	void HandlePhaseChanged(EGamePhase NewPhase);

	UFUNCTION()
	void HandleRoundChanged(int32 NewRound);

	UFUNCTION()
	void HandleShopRefreshed();

	UFUNCTION()
	void HandleLevelUp(int32 NewLevel, int32 BoardCapacity);
};