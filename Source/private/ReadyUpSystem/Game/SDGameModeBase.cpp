// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SDGameModeBase.h"

#include "Algo/AllOf.h"
#include "Game/SDGameStateBase.h"
#include "Player/SDPlayerController.h"
#include "Game/SDGameplayLibrary.h"
#include "Game/SDSpawnBehaviorComponent.h"

ASDGameModeBase::ASDGameModeBase()
{

}

/*
 * Gets called on Client login
 */
void ASDGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	ConnectedClients.Add(NewPlayer);

	FConnectedClientData NewClientData = FConnectedClientData();
	NewClientData.ClientController = NewPlayer;
	NewClientData.ClientName = GetNewPlayerName();
	NewClientData.bIsReady = false;
	// Map a client to a unique identifier, a name in this case
	ConnectedClientsDataByController.Add(NewPlayer, NewClientData);

	// Update to all other clients, that a new client has joined
	UpdateClientsReadyStatus();

	// Check if the game is already over and if the ready up screen needs to be loaded
	CheckIfGameOver(NewPlayer);
}

/*
 * Gets called on Client logout
 */
void ASDGameModeBase::Logout(AController* ExitingPlayer)
{
	Super::Logout(ExitingPlayer);

	// return if server
	if (ExitingPlayer->IsLocalPlayerController())
		return;
	
	ConnectedClients.Remove(ExitingPlayer);
	ConnectedClientsDataByController.Remove(ExitingPlayer);
	
	// Update to all other clients, that a client has left
	UpdateClientsReadyStatus();
}

void ASDGameModeBase::SetClientIsReady(AController* ClientController, bool bIsReady)
{
	if (!IsValid(ClientController))
		return;
	
	if (FConnectedClientData* ClientData = ConnectedClientsDataByController.Find(ClientController))
	{
		ClientData->bIsReady = bIsReady;
		UpdateClientsReadyStatus();
	}
}

void ASDGameModeBase::GameOver(bool bGameWon)
{
	if (GetSDGameState())
	{
		if (bGameWon == true)
			SDGameState->Multicast_OnWin();
		else
			SDGameState->Multicast_OnLoss();
	}
}

/*
 * Requests all clients ready status and updates them accordingly
 */
void ASDGameModeBase::UpdateClientsReadyStatus()
{
	if (ConnectedClients.Num() == 0)
		return;

	TArray<FConnectedClientData> ConnectedClientsData;
	ConnectedClientsDataByController.GenerateValueArray(ConnectedClientsData);
	
	for (AController* Controller : ConnectedClients)
	{
		if (ASDPlayerController* SDPlayerController = Cast<ASDPlayerController>(Controller))
		{
			SDPlayerController->Client_SendClientsReadyStatus(ConnectedClientsData);
		}
	}

	if (AllClientsReady())
		StartRestartProcess();
	else
		EndRestartProcess();	
}

FName ASDGameModeBase::GetNewPlayerName() const
{
	const int32 PlayerIndex = ConnectedClients.Num();
	const FString PlayerName = FString::Printf(TEXT("Player %d"), PlayerIndex);
	return FName(*PlayerName);
}

/*
* Checks if the game is over and triggers the ready up check for the given playercontroller if true
* Checks periodically if all data is present using a per client timer handle
*/
void ASDGameModeBase::CheckIfGameOver(AController* ClientController)
{
	if (GetSDGameState())
	{
		if (USDGameplayLibrary::GetIsGameOver(this))
		{
			if (FConnectedClientData* ClientData = ConnectedClientsDataByController.Find(ClientController))
			{
				ClientData->bNeedsLateJoinCheck = true;
				GetWorld()->GetTimerManager().SetTimer(ClientData->LateJoinCheckGameOverTimer,
					[this, ClientController]()
					{
						CheckForClientGameState(ClientController);
					},
					ClientLateJoinNotifyTimer, true);
			}
		}
	}
}

void ASDGameModeBase::CheckForClientGameState(AController* ClientController)
{
	if (GetSDGameState())
	{
		if (FConnectedClientData* ClientData = GetConnectedClientDataFromController(ClientController))
		{
			if (!ClientData->bNeedsLateJoinCheck)
			{
				GetWorld()->GetTimerManager().ClearTimer(ClientData->LateJoinCheckGameOverTimer);
				return;
			}
			
			if (ASDPlayerController* Client = Cast<ASDPlayerController>(ClientController))
					Client->Client_CheckIfGameModeSet(ClientData->ClientName);
		}
	}
}

bool ASDGameModeBase::AllClientsReady() const
{
	if (ConnectedClients.Num() == 0)
		return false;
	
	for (const AController* Client : ConnectedClients)
	{
		const FConnectedClientData* ClientData = ConnectedClientsDataByController.Find(Client);
		if (!ClientData || !ClientData->bIsReady)
		{
			UE_LOG(LogTemp, Warning, TEXT("A client is not ready. Ready-Check failed"));
			return false;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("All clients ready. Ready-Check succeeded"));

	return true;
}

void ASDGameModeBase::StartRestartProcess()
{
	if (RestartTimerActive)
		return;
	
	NotifyClientsOfRestart();
	RestartTimerActive = true;
	GetWorld()->GetTimerManager().SetTimer(RestartTimerHandle,
		[this]()
		{
			RestartGame();
		},
		RestartTimer, false);
}

void ASDGameModeBase::EndRestartProcess()
{
	NotifyClientsOfRestart(true);
	GetWorld()->GetTimerManager().ClearTimer(RestartTimerHandle);
	RestartTimerActive = false;
}

void ASDGameModeBase::NotifyClientsOfRestart(const bool bCancel)
{
	for (AController* Client : ConnectedClients)
	{
		if (ASDPlayerController* SDPlayerController = Cast<ASDPlayerController>(Client))
			SDPlayerController->Client_NotifyRestart(bCancel);
	}
}


FConnectedClientData* ASDGameModeBase::GetConnectedClientDataFromController(const AController* ClientController)
{
	if (ConnectedClientsDataByController.Contains(ClientController))
	{
		return ConnectedClientsDataByController.Find(ClientController);
	}

	return nullptr;
}

FConnectedClientData* ASDGameModeBase::GetConnectedClientDataFromClientName(const FName ClientName)
{
	for (TPair<AController* ,FConnectedClientData>& ClientData : ConnectedClientsDataByController)
	{
		if (ClientData.Value.ClientName == ClientName)
			return &ClientData.Value;
	}
	
	return nullptr;
}

ASDGameStateBase* ASDGameModeBase::GetSDGameState()
{
	if (SDGameState == nullptr)
		SDGameState = Cast<ASDGameStateBase>(GameState);
	return SDGameState;
}