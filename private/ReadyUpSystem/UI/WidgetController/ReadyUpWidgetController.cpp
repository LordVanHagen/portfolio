// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/ReadyUpWidgetController.h"
#include "Player/SDPlayerController.h"
#include "Kismet/GameplayStatics.h"

void UReadyUpWidgetController::BindCallbacksToDependencies()
{
	if (ASDPlayerController* SDPlayerController = Cast<ASDPlayerController>(PlayerController))
	{
		SDPlayerController->OnClientsDataChanged.AddLambda(
			[this](const TArray<FConnectedClientData>& ConnectedClientsData)
			{
				OnConnectedClientsDataChanged.Broadcast(ConnectedClientsData);
			}
		);
	}
}
