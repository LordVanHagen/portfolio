// Fill out your copyright notice in the Description page of Project Settings.


#include "Squads/SquadFactory.h"

#include "Squads/SquadCommands/SquadCommands.h"
#include "Squads/Interfaces/CommandInterface.h"
#include "Squads/Wrappers/SquadLeader.h"
#include "Squads/Wrappers/SquadMember.h"

TMap<FName, USquadCommand*> USquadFactory::CommandMap;

void USquadFactory::RegisterCommands()
{
	CommandMap.Add("GoTo", NewObject<UGoToCommand>(GetTransientPackage()));
	CommandMap.Add("Attack", NewObject<UAttackCommand>(GetTransientPackage()));
	CommandMap.Add("Swarm",  NewObject<USwarmCommand>(GetTransientPackage()));
	CommandMap.Add("Flee",  NewObject<UFleeCommand>(GetTransientPackage()));
	CommandMap.Add("Chase", NewObject<UChaseCommand>(GetTransientPackage()));
	CommandMap.Add("Retreat", NewObject<URetreatCommand>(GetTransientPackage()));
}

USquadCommand* USquadFactory::GetCommand(UObject* WorldContextObject, const FName CommandName)
{
	if (USquadCommand** Command = CommandMap.Find(CommandName))
		return *Command;

	UE_LOG(LogTemp, Error, TEXT("Unknown CommandName: %s"), *CommandName.ToString());
	return nullptr;
}

USquadLeader* USquadFactory::CreateSquadLeader(AActor* Outer)
{
	USquadLeader* SquadLeader = NewObject<USquadLeader>(Outer);
	SquadLeader->Init(Outer);
	return SquadLeader;
}

USquadMember* USquadFactory::CreateSquadMember(UObject* CommandableObject, USquadLeader* Leader)
{
	if (!CommandableObject || !CommandableObject->Implements<UCommandableInterface>())
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateSquadMember: Object does not implement ICommandableInterface"));
		return nullptr;
	}

	ICommandableInterface* Commandable = Cast<ICommandableInterface>(CommandableObject);
	if (!Commandable)
		return nullptr;

	USquadMember* NewMember = NewObject<USquadMember>(CommandableObject);
	NewMember->Init(Commandable, Leader);
	Commandable->SetMemberWrapper(NewMember);
	return NewMember;
}

void USquadFactory::ClearCommandMap()
{
	CommandMap.Empty();
}
