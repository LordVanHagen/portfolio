// Fill out your copyright notice in the Description page of Project Settings.


#include "Squads/SquadCommands/SquadCommands.h"

void USquadCommand::Execute(ICommandableInterface* Commandable, const FSquadCommandPayload& CommandPayload)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString(TEXT("Default Command has been triggered")));
}

void UMultiCommand::Execute(ICommandableInterface* Commandable, const FSquadCommandPayload& SquadCommandPayload)
{
	if (SquadCommands.IsEmpty())
		return;

	for (ISquadCommandInterface* Command : SquadCommands)
		Command->Execute(Commandable, SquadCommandPayload);
}

void UMultiCommand::SetCommands(const TArray<USquadCommand*>& InSquadCommands)
{
	if (!SquadCommands.IsEmpty())
		SquadCommands.Empty();

	SquadCommands = InSquadCommands;
}
