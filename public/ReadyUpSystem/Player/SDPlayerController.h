// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SDPlayerController.generated.h"

struct FConnectedClientData;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnConnectedClientsDataChanged, const TArray<FConnectedClientData>& NewConnectedClientData);
DECLARE_DELEGATE_OneParam(FOnLateJoinGameOverSignature, bool bGameWon);
DECLARE_DELEGATE_OneParam(FOnRequestRestartTimer, bool bCancel);

/**
 * 
 */
UCLASS()
class SUPERDEFENCE_API ASDPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ASDPlayerController();

	FOnConnectedClientsDataChanged OnClientsDataChanged;
	FOnLateJoinGameOverSignature OnLateJoinGameOver;
	FOnRequestRestartTimer OnStartRestartTimer;
	
	UFUNCTION(Client, Reliable, BlueprintCallable)
	virtual void Client_SendClientsReadyStatus(const TArray<FConnectedClientData>& ConnectedClientsData);
	virtual void Client_UpdateGameEnd(bool bGameWon);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	virtual void Server_SetClientIsReady(AController* ClientController, const bool bIsReady);

	UFUNCTION(Client, Reliable, BlueprintCallable)
	virtual void Client_NotifyRestart(const bool bCancel);
	
	virtual void ReceivedGameModeClass(TSubclassOf<class AGameModeBase> GameModeClass) override;

	UFUNCTION(Client, Reliable)
	virtual void Client_CheckIfGameModeSet(FName ConnectedClientName);

	UFUNCTION(Server, Reliable)
	virtual void Server_CallGameEnd(FName ConnectedClientName);

private:
	bool bGameModeSet = false;
};
