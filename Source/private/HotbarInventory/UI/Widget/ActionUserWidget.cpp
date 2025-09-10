// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/ActionUserWidget.h"

void UActionUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}
