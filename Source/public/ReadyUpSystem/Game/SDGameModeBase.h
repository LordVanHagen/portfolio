// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SDGameModeBase.generated.h"

class ASDGameStateBase;

/**
 * 
 */
UCLASS()
class SUPERDEFENCE_API ASDGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	ASDGameModeBase();

	// Gameloop //
	void EnemyDefeated();
	UFUNCTION(BlueprintImplementableEvent)
	void RestartGame();
	UFUNCTION(BlueprintCallable)
	virtual void SetClientIsReady(AController* ClientController, bool bIsReady);
	virtual void GameOver(bool bGameWon);
	FConnectedClientData* GetConnectedClientDataFromClientName(const FName ClientName);
	// Gameloop //

protected:

	virtual void OnPostLogin(AController* NewPlayer) override;
	virtual void Logout(AController* ExitingPlayer) override;

	UFUNCTION(BlueprintCallable)
	virtual void UpdateClientsReadyStatus();

	UPROPERTY(VisibleAnywhere, Category = "Networking|Sessions|Clients")
	TMap<AController*, FConnectedClientData> ConnectedClientsDataByController;

	UPROPERTY(VisibleAnywhere, Category = "Networking|Sessions|Clients")
	TArray<AController*> ConnectedClients;

	UPROPERTY(EditAnywhere, Category = "Networking|Sessions|Clients")
	float ClientLateJoinNotifyTimer = 2.f;
	
	UPROPERTY(EditAnywhere, Category = "Networking|Sessions|Clients")
	float RestartTimer = 5.f;
	FTimerHandle RestartTimerHandle;
	bool RestartTimerActive = false;


private:
	FName GetNewPlayerName() const;

	void CheckIfGameOver(AController* ClientController);

	void CheckForClientGameState(AController* ClientController);

	ASDGameStateBase* GetSDGameState();

	FConnectedClientData* GetConnectedClientDataFromController(const AController* ClientController);
	
	bool AllClientsReady() const;
	void StartRestartProcess();
	void EndRestartProcess();
	void NotifyClientsOfRestart(const bool bCancel = false);

	UPROPERTY()
	ASDGameStateBase* SDGameState;
};
