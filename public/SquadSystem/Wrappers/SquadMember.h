// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Squads/Interfaces/CommandInterface.h"
#include "Squads/Interfaces/MemberInterface.h"
#include "SquadMember.generated.h"

struct FSquadMemberPositionInfo;
class ICommandableInterface;
class ILeaderInterface;
class USquadLeader;
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class SUPERDEFENCE_API USquadMember : public UObject, public IMemberInterface
{
	GENERATED_BODY()
public:
	USquadMember();
	virtual void BeginDestroy() override;
	
	virtual void ExecuteCommand(USquadCommand* Command, const FSquadCommandPayload& SquadCommandPayload) override;
	virtual void Init(ICommandableInterface* Outer, ILeaderInterface* InLeader) override;
	virtual void SetMemberID(int32 const InMemberID) override { SquadMemberID = InMemberID; };
	virtual int32 GetMemberID() const override { return SquadMemberID; };
	virtual const FSquadMemberPositionInfo* GetPositionInfo() override;
	virtual void Die() override;
	virtual void Move(const FComputedLocationInfo& ComputedLocation) const override;
	virtual void SetVisibilitySetting(const EVisibilitySetting InVisibilitySetting) override;
	
	ICommandableInterface* Get() const { return Member; }
	virtual ILeaderInterface* GetLeader() const override { return Leader; }
	
private:
	ICommandableInterface* Member = nullptr;
	ILeaderInterface* Leader = nullptr;

	int32 SquadMemberID = -1;
};
