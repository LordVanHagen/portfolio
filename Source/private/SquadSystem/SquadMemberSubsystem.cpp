// Fill out your copyright notice in the Description page of Project Settings.


#include "Squads/SquadMemberSubsystem.h"

#include "Settings/SquadEnemySettings.h"
#include "Squads/SquadFactory.h"

void USquadMemberSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const USquadEnemySettings* SquadSettings = GetDefault<USquadEnemySettings>();
	SquadEnemyData = SquadSettings->SquadData.LoadSynchronous();

	USquadFactory::ClearCommandMap();
	USquadFactory::RegisterCommands();
}

void USquadMemberSubsystem::Deinitialize()
{
	SquadEnemyData = nullptr;
	USquadFactory::ClearCommandMap();
	
	Super::Deinitialize();
}