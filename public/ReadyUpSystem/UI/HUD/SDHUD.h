// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SDHUD.generated.h"

class USDUserWidget;
class UOverlayWidgetController;
struct FWidgetControllerParams;
class UGameEndWidgetController;
class UReadyUpWidgetController;
class UAbilitySystemComponent;
class UAttributeSet;

/**
 * 
 */
UCLASS()
class SUPERDEFENCE_API ASDHUD : public AHUD
{
	GENERATED_BODY()
public:
	UOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WCParams);

	UReadyUpWidgetController* GetReadyUpWidgetController(const FWidgetControllerParams& WCParams);

	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

private:
	/* Overlay Widget */
	UPROPERTY()
	TObjectPtr<USDUserWidget> OverlayWidget;
	UPROPERTY(EditAnywhere)
	TSubclassOf<USDUserWidget> OverlayWidgetClass;
	/* Overlay Widget */
	/* Overlay WidgetController */
	UPROPERTY()
	TObjectPtr<UOverlayWidgetController> OverlayWidgetController;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;
	/* Overlay WidgetController */

	/* ReadyUp WidgetController */
	UPROPERTY()
	TObjectPtr<UReadyUpWidgetController> ReadyUpWidgetController;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UReadyUpWidgetController> ReadyUpWidgetControllerClass;
	/* ReadyUp WidgetController */
};
