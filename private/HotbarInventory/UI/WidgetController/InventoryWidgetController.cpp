// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/InventoryWidgetController.h"
#include "ActionGameCharacter.h"
#include "ActorComponents/InventoryComponent.h"

void UInventoryWidgetController::BroadcastInitialValues()
{
	if (InventoryComponent)
	{
		FInventoryItemsPayload CurrentItems;
		CurrentItems.InventoryList = InventoryComponent->GetInventoryList();
		CurrentItems.CurrentItemIndex = InventoryComponent->GetCurrentItemIndex();
		OnUpdateInventory.Broadcast(CurrentItems);
	}
}

void UInventoryWidgetController::BindCallbacksToDependencies()
{
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryChanged.AddLambda(
			[this](const FInventoryItemsPayload& CurrentItems)
			{
				OnUpdateInventory.Broadcast(CurrentItems);
			});
	}
	else
		UE_LOG(LogTemp, Error, TEXT("InventoryComponent not set on %s"), *PlayerController->GetPawn()->GetName());
}
