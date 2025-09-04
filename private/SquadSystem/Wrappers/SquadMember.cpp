// Fill out your copyright notice in the Description page of Project Settings.


#include "Squads/Wrappers/SquadMember.h"

#include "Squads/Interfaces/LeaderInterface.h"
#include "Squads/SquadCommands/SquadCommands.h"

USquadMember::USquadMember()
{
}

void USquadMember::BeginDestroy()
{
	Member  = nullptr;
	Leader  = nullptr;
	UObject::BeginDestroy();
}

void USquadMember::Init(ICommandableInterface* Outer, ILeaderInterface* InLeader)
{
	Member = Outer;
	Leader = InLeader;
	Member->InitStats();
}

const FSquadMemberPositionInfo* USquadMember::GetPositionInfo()
{
	if (Member)
		return Member->GetPositionInfo(SquadMemberID);

	return nullptr;
}

void USquadMember::Die()
{
	if (Leader)
		Leader->RemoveMember(this);
}

void USquadMember::Move(const FComputedLocationInfo& ComputedLocation) const
{
	if (Member)
		Member->Move(ComputedLocation);
}

void USquadMember::SetVisibilitySetting(const EVisibilitySetting InVisibilitySetting)
{
	if (Member)
		Member->SetVisibilitySetting(InVisibilitySetting);
};

void USquadMember::ExecuteCommand(USquadCommand* Command, const FSquadCommandPayload& SquadCommandPayload)
{
	if (!Member || !Command)
		return;
	Command->Execute(Member, SquadCommandPayload);
}