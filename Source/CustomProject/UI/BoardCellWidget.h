#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BoardCellWidget.generated.h"

class ATFTGameMode;
class UImage;

UCLASS()
class CUSTOMPROJECT_API UBoardCellWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY()
	ATFTGameMode* TFTGameMode = nullptr;

	// Which cell this widget represents
	UPROPERTY(BlueprintReadWrite)
	int32 Col = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 Row = 0;

	// Highlight when unit is dragged over
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UImage* CellBackground;

	void SetHighlight(bool bHighlight);

protected:
	virtual bool NativeOnDrop(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

	virtual bool NativeOnDragOver(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

	virtual void NativeOnDragLeave(
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;
};