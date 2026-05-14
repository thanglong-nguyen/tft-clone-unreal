#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BenchUnitCard.generated.h"

class UTextBlock;
class AUnit;

UCLASS()
class CUSTOMPROJECT_API UBenchUnitCard : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY()
	AUnit* Unit = nullptr;

	void SetUnit(AUnit* InUnit);

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* UnitNameText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* StarText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* CostText;

protected:
	// Called when player starts dragging this card
	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation) override;
};