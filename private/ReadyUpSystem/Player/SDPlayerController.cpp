// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SDPlayerController.h"
#include "Game/SDGameModeBase.h"

ASDPlayerController::ASDPlayerController()
{
	bReplicates = true;
}

void ASDPlayerController::Client_UpdateGameEnd(bool bGameWon)
{
	if(OnLateJoinGameOver.IsBound())
		OnLateJoinGameOver.Execute(bGameWon);
	else
		UE_LOG(LogTemp, Error, TEXT("OnLateJoinGameOver not bound on: %s"), *GetNameSafe(this));
}

void ASDPlayerController::Server_SetClientIsReady_Implementation(AController* ClientController, const bool bIsReady)
{
	if (ASDGameModeBase* SDGameMode = Cast<ASDGameModeBase>(GetWorld()->GetAuthGameMode()))
		SDGameMode->SetClientIsReady(ClientController, bIsReady);
}

void ASDPlayerController::Client_SendClientsReadyStatus_Implementation(const TArray<FConnectedClientData>& ConnectedClientsData)
{
	OnClientsDataChanged.Broadcast(ConnectedClientsData);
}

void ASDPlayerController::Client_NotifyRestart_Implementation(const bool bCancel)
{
	if (OnStartRestartTimer.IsBound())
		OnStartRestartTimer.Execute(bCancel);
}

void ASDPlayerController::ReceivedGameModeClass(TSubclassOf<class AGameModeBase> GameModeClass)
{
	bGameModeSet = true;
}

void ASDPlayerController::Client_CheckIfGameModeSet_Implementation(FName ConnectedClientName)
{
	if (bGameModeSet == true)
		Server_CallGameEnd(ConnectedClientName);
}

void ASDPlayerController::Server_CallGameEnd_Implementation(FName ConnectedClientName)
{
	if (ASDGameModeBase* SDGameMode = Cast<ASDGameModeBase>(GetWorld()->GetAuthGameMode()))
	{
		FConnectedClientData* ConnectedClientData = SDGameMode->GetConnectedClientDataFromClientName(ConnectedClientName);
		ConnectedClientData->bNeedsLateJoinCheck = false;
		SDGameMode->GameOver(USDGameplayLibrary::GetIsGameWon(this));
	}
}