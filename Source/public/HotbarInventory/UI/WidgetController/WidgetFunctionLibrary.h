// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WidgetFunctionLibrary.generated.h"

class UInventoryWidgetController;

/**
 * 
 */
UCLASS()
class ACTIONGAME_API UWidgetFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "WidgetFunctionLibrary|InventoryWidgetController")
	static UInventoryWidgetController* GetInventoryWidgetController(const UObject* WorldContextObject);
};
