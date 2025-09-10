// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/SDWidgetController.h"
#include "SDTypes.h"
#include "ReadyUpWidgetController.generated.h"

struct FConnectedClientData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotifyConnectedClientsDataChangedSignature, const TArray<FConnectedClientData>&, NewConnectedClientsData);

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class SUPERDEFENCE_API UReadyUpWidgetController : public USDWidgetController
{
	GENERATED_BODY()
public:
	virtual void BindCallbacksToDependencies() override;

	UPROPERTY(BlueprintAssignable, Category = "ClientData")
	FOnNotifyConnectedClientsDataChangedSignature OnConnectedClientsDataChanged;
};
