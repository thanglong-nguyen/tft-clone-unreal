// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "UnitStateWidget.generated.h"

class AUnit;
/**
 * 
 */
UCLASS()
class CUSTOMPROJECT_API UUnitStateWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
	
	UPROPERTY()
	AUnit* Owner;
	
	void UpdateValues(); 
	
};
