// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/SDHUD.h"
#include "UI/Widget/SDUserWidget.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "UI/WidgetController/GameEndWidgetController.h"
#include "UI/WidgetController/ReadyUpWidgetController.h"
#include "AbilitySystemComponent.h"

UOverlayWidgetController* ASDHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();
	}

	return OverlayWidgetController;
}

UReadyUpWidgetController* ASDHUD::GetReadyUpWidgetController(const FWidgetControllerParams& WCParams)
{
	if (ReadyUpWidgetController == nullptr)
	{
		ReadyUpWidgetController = NewObject<UReadyUpWidgetController>(this, ReadyUpWidgetControllerClass);
		ReadyUpWidgetController->SetWidgetControllerParams(WCParams);
		ReadyUpWidgetController->BindCallbacksToDependencies();
	}

	return ReadyUpWidgetController;
}

void ASDHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class unitialized, please fill out BP_SDHUD"));
	checkf(OverlayWidgetControllerClass, TEXT("Overlay Widget Controller Class unitialized, please fill out BP_SDHUD"));

	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<USDUserWidget>(Widget);

	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

	OverlayWidget->SetWidgetController(WidgetController);
	WidgetController->BroadcastInitialValues();
	Widget->AddToViewport();
}
