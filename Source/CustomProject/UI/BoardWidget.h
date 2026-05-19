#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BoardWidget.generated.h"

class ATFTGameMode;
class UVerticalBox;
class UHorizontalBox;

UCLASS()
class CUSTOMPROJECT_API UBoardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY()
	ATFTGameMode* TFTGameMode = nullptr;

	// -------------------------------------------------------
	// Widget Bindings
	// -------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UVerticalBox* SynergyContainer;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UHorizontalBox* BenchContainer;

	// -------------------------------------------------------
	// Classes to spawn dynamically — set in WBP_BoardWidget
	// -------------------------------------------------------

	UPROPERTY(EditAnywhere, Category="Board")
	TSubclassOf<class USynergyCard> SynergyCardClass;

	UPROPERTY(EditAnywhere, Category="Board")
	TSubclassOf<class UUnitCard> UnitCardClass;

	// -------------------------------------------------------
	// Public Refresh Functions
	// -------------------------------------------------------

	// Rebuilds synergy cards from current trait data
	UFUNCTION(BlueprintCallable)
	void RefreshSynergies();

	// Rebuilds bench cards from player bench units
	UFUNCTION(BlueprintCallable)
	void RefreshBench();

private:
	void BindDelegates();

	UFUNCTION()
	void HandleTraitsUpdated();

	UFUNCTION()
	void HandleUnitPurchased(AUnit* NewUnit);
};