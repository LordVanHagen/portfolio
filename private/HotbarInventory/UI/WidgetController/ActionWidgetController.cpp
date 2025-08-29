// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/ActionWidgetController.h"

void UActionWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& Params)
{
	PlayerController = Params.PlayerController;
	PlayerState = Params.PlayerState;
	AbilitySystemComponent = Params.AbilitySystemComponent;
	AttributeSet = Params.AttributeSet;
	InventoryComponent = Params.InventoryComponent;
}

void UActionWidgetController::BroadcastInitialValues()
{
}

void UActionWidgetController::BindCallbacksToDependencies()
{
}
