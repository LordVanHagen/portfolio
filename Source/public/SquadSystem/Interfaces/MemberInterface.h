// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MemberInterface.generated.h"

enum EVisibilitySetting : uint8;
struct FComputedLocationInfo;
struct FSquadMemberPositionInfo;
struct FPathPointData;
class ICommandableInterface;
class USquadCommand;
class ILeaderInterface;
struct FSquadCommandPayload;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMemberInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SUPERDEFENCE_API IMemberInterface
{
	GENERATED_BODY()
public:
	virtual void ExecuteCommand(USquadCommand* Command, const FSquadCommandPayload& SquadCommandPayload) = 0;
	virtual void Init(ICommandableInterface* Outer, ILeaderInterface* InLeader) = 0;
	virtual void SetMemberID(const int32 InMemberID) = 0;
	virtual int32 GetMemberID() const = 0;
	virtual const FSquadMemberPositionInfo* GetPositionInfo() = 0;
	virtual void Die() = 0;
	virtual ILeaderInterface* GetLeader() const = 0;
	virtual void Move(const FComputedLocationInfo& ComputedLocation) const = 0;
	virtual void SetVisibilitySetting(const EVisibilitySetting InVisibilitySetting) = 0;
};