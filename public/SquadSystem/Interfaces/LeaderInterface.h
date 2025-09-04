// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LeaderInterface.generated.h"

class USquadCommand;
class IMemberInterface;
struct FSquadCommandPayload;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULeaderInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SUPERDEFENCE_API ILeaderInterface
{
	GENERATED_BODY()
public:
	virtual void ExecuteCommand(USquadCommand* Command, const FSquadCommandPayload& SquadCommandPayload) = 0;
	virtual void NotifySquadReady() = 0;
	virtual void AddMember(IMemberInterface* NewMember) = 0;
	virtual void RemoveMember(IMemberInterface* MemberToRemove) = 0;
	virtual TArray<IMemberInterface*> GetMembers() = 0;
	virtual void Join(ILeaderInterface* Other) = 0;
	virtual void Split() = 0;
	virtual void TerminateSquad() = 0;
};
