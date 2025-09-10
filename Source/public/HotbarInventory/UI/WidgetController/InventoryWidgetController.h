// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/ActionWidgetController.h"
#include "../../../Inventory/InventoryList.h"
#include "InventoryWidgetController.generated.h"


USTRUCT(BlueprintType)
struct FInventoryItemsPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInventoryList InventoryList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 CurrentItemIndex;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateInventorySignature, FInventoryItemsPayload, CurrentInventory);

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class ACTIONGAME_API UInventoryWidgetController : public UActionWidgetController
{
	GENERATED_BODY()
public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnUpdateInventorySignature OnUpdateInventory;
};
