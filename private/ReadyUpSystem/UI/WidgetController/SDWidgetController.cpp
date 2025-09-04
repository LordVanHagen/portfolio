// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/SDWidgetController.h"

void USDWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WParams)
{
	PlayerController = WParams.PlayerController;
	PlayerState = WParams.PlayerState;
	AbilitySystemComponent = WParams.AbilitySystemComponent;
	AttributeSet = WParams.AttributeSet;
}

void USDWidgetController::BroadcastInitialValues()
{

}

void USDWidgetController::BindCallbacksToDependencies()
{

}
