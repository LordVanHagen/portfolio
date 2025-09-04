// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Squads/Interfaces/CommandInterface.h"
#include "SquadCommands.generated.h"

/**
 * Abstract base class for commands, containing the commands name and the Execute function.
 * Child classes can define their own name and corresponding behavior inside Execute
 */
UCLASS(Abstract)
class SUPERDEFENCE_API USquadCommand : public UObject, public ISquadCommandInterface
{
	GENERATED_BODY()
public:
	virtual void Execute(ICommandableInterface* Commandable, const FSquadCommandPayload& SquadCommandPayload) override;

	virtual FName GetCommandName() const { return NAME_None; }
};

UCLASS()
class UMultiCommand : public USquadCommand
{
	GENERATED_BODY()
public:
	virtual void Execute(ICommandableInterface* Commandable, const FSquadCommandPayload& SquadCommandPayload) override;

	virtual void SetCommands(const TArray<USquadCommand*>& InSquadCommands);

private:
	UPROPERTY()
	TArray<USquadCommand*> SquadCommands;
};

UCLASS()
class UGoToCommand : public USquadCommand
{
	GENERATED_BODY()
public:
	virtual void Execute(ICommandableInterface* Commandable, const FSquadCommandPayload& SquadCommandPayload) override { Commandable->GoTo(SquadCommandPayload); }
	virtual FName GetCommandName() const override { return "GoTo"; }
};

UCLASS()
class UAttackCommand : public USquadCommand
{
	GENERATED_BODY()
public:
	virtual void Execute(ICommandableInterface* Commandable, const FSquadCommandPayload& SquadCommandPayload) override { Commandable->Attack(SquadCommandPayload); }
	virtual FName GetCommandName() const override { return "Attack"; }
};


UCLASS()
class USwarmCommand : public USquadCommand
{
	GENERATED_BODY()
public:
	virtual void Execute(ICommandableInterface* Commandable, const FSquadCommandPayload& SquadCommandPayload) override { Commandable->Swarm(SquadCommandPayload); }
	virtual FName GetCommandName() const override { return "Swarm"; }
};


UCLASS()
class UFleeCommand : public USquadCommand
{
	GENERATED_BODY()
public:
	virtual void Execute(ICommandableInterface* Commandable, const FSquadCommandPayload& SquadCommandPayload) override { Commandable->Flee(SquadCommandPayload); }
	virtual FName GetCommandName() const override { return "Flee"; }
};

UCLASS()
class UChaseCommand : public USquadCommand
{
	GENERATED_BODY()
public:
	virtual void Execute(ICommandableInterface* Commandable, const FSquadCommandPayload& SquadCommandPayload) override { Commandable->Flee(SquadCommandPayload); }
	virtual FName GetCommandName() const override { return "Chase"; }
};

UCLASS()
class URetreatCommand : public USquadCommand
{
	GENERATED_BODY()
public:
	virtual void Execute(ICommandableInterface* Commandable, const FSquadCommandPayload& SquadCommandPayload) override { Commandable->Flee(SquadCommandPayload); }
	virtual FName GetCommandName() const override { return "Retreat"; }
};