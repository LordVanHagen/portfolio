// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AGHUD.generated.h"

class UOverlayWidgetController;
class UInventoryWidgetController;
struct FWidgetControllerParams;
class UAbilitySystemComponent;
class UAttributeSet;
class UInventoryComponent;
class UActionUserWidget;

/**
 * 
 */
UCLASS()
class ACTIONGAME_API AAGHUD : public AHUD
{
	GENERATED_BODY()
public:
	UOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WCParams);

	UInventoryWidgetController* GetInventoryWidgetController(const FWidgetControllerParams& WCParams);

	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS, UInventoryComponent* IC);

private:
	/* Overlay Widget */
	UPROPERTY()
	TObjectPtr<UActionUserWidget> OverlayWidget;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UActionUserWidget> ActionWidgetClass;

	UPROPERTY()
	TObjectPtr<UOverlayWidgetController> OverlayWidgetController;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;
	/* Overlay Widget */

	/* InventoryWidget */
	UPROPERTY()
	TObjectPtr<UInventoryWidgetController> InventoryWidgetController;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UInventoryWidgetController> InventoryWidgetControllerClass;
	/* InventoryWidget */

};
