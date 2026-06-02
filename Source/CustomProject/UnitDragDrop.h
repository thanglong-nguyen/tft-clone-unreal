#pragma once
#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "UnitDragDrop.generated.h"

class AUnit;

UCLASS()
class CUSTOMPROJECT_API UUnitDragDrop : public UDragDropOperation
{
    GENERATED_BODY()

public:
    // The unit being dragged
    UPROPERTY()
    AUnit* DraggedUnit = nullptr;

    // Whether unit came from board or bench
    UPROPERTY()
    bool bFromBoard = false;
};