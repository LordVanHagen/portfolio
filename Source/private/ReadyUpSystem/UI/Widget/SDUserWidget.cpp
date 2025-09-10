// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/SDUserWidget.h"

void USDUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}
