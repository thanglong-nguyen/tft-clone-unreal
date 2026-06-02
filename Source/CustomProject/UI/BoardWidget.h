#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BoardWidget.generated.h"

class AUnit;
class USellZoneWidget;
class UButton;
class ATFTGameMode;
class UVerticalBox;
class UUniformGridPanel;
class UBoardCellWidget;

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
	UUniformGridPanel* BenchGrid;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UUniformGridPanel* BoardGrid;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	USellZoneWidget* SellZone;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UButton* ToggleHUDButton;

	// -------------------------------------------------------
	// Classes to spawn dynamically — set in WBP_BoardWidget
	// -------------------------------------------------------

	UPROPERTY(EditAnywhere, Category="Board")
	TSubclassOf<class USynergyCard> SynergyCardClass;
	
	UPROPERTY(EditAnywhere, Category="Board")
    TSubclassOf<UBoardCellWidget> BoardCellClass;

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
	
	void BuildBoardGrid();
	
	// Find a cell widget by grid coordinates and clear it
	void ClearCell(int32 Col, int32 Row);

	// Find a cell widget by grid coordinates  
	UBoardCellWidget* GetCellWidget(int32 Col, int32 Row);
	
	    
	UFUNCTION()
	void HandleUnitsMerged(int32 Col, int32 Row);
	
	UFUNCTION()
	void HandleUnitPlaced(AUnit* PlacedUnit);
	
	UFUNCTION()
	void OnToggleHUDClicked();
	
	void RebuildOccupiedCells();

private:
	void BindDelegates();

	UFUNCTION()
	void HandleTraitsUpdated();

	UFUNCTION()
	void HandleUnitTransaction(AUnit* NewUnit);
};