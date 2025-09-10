// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/AGHUD.h"
#include "Public/UI/Widget/ActionUserWidget.h"
#include "Public/UI/WidgetController/OverlayWidgetController.h"
#include "Public/UI/WidgetController/InventoryWidgetController.h"

UOverlayWidgetController* AAGHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();
	}

	return OverlayWidgetController;
}

UInventoryWidgetController* AAGHUD::GetInventoryWidgetController(const FWidgetControllerParams& WCParams)
{
	if (InventoryWidgetController == nullptr)
	{
		InventoryWidgetController = NewObject<UInventoryWidgetController>(this, InventoryWidgetControllerClass);
		InventoryWidgetController->SetWidgetControllerParams(WCParams);
		InventoryWidgetController->BindCallbacksToDependencies();
	}

	return InventoryWidgetController;
}

void AAGHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS, UInventoryComponent* IC)
{
	checkf(ActionWidgetClass, TEXT("Overlay Widget Class unitialized, please fill out BP_SDHUD"));
	checkf(OverlayWidgetControllerClass, TEXT("Overlay Widget Controller Class unitialized, please fill out BP_SDHUD"));

	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), ActionWidgetClass);
	OverlayWidget = Cast<UActionUserWidget>(Widget);

	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS, IC);
	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

	OverlayWidget->SetWidgetController(WidgetController);
	WidgetController->BroadcastInitialValues();
	Widget->AddToViewport();
}
