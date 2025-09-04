// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SDGameStateBase.h"
#include "Net/UnrealNetwork.h"

void ASDGameStateBase::Multicast_OnWin_Implementation()
{
	bGameOver = true;
	bGameWon = true;

	OnGameOver.Broadcast(true);
}

void ASDGameStateBase::Multicast_OnLoss_Implementation()
{
	bGameOver = true;
	bGameWon = false;

	OnGameOver.Broadcast(false);
}