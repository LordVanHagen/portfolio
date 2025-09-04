// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SDGameStateBase.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameOverSignature, bool);

/**
 * 
 */
UCLASS()
class SUPERDEFENCE_API ASDGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
public:
	FOnGameOverSignature OnGameOver;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnWin();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnLoss();

private:
	UPROPERTY(VisibleAnywhere, Category = "Gameplay")
	bool bGameOver = false;

	UPROPERTY(VisibleAnywhere, Category = "Gameplay")
	bool bGameWon = false;

public:
	FORCEINLINE bool GetGameOver() { return bGameOver; }
	FORCEINLINE bool GetGameWon() { return bGameWon; }
};
