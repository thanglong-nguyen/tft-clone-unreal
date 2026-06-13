#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SellZoneWidget.generated.h"

class ATFTGameMode;
class UImage;

// Drop target widget — dragging a unit onto this sells it for gold.
UCLASS()
class CUSTOMPROJECT_API USellZoneWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY()
	ATFTGameMode* TFTGameMode = nullptr;

	// Background image — tints brighter red when a unit is dragged over
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UImage* SellBackground;

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