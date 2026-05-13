// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UnitStateWidget.h"
#include "Units/Unit.h"


void UUnitStateWidget::UpdateValues()
{
	if (Owner)
	{
		HealthBar->SetPercent(Owner->CurrentHP /
		Owner->MaxHP);
	}
}
