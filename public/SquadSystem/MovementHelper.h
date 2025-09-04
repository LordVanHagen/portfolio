// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/CommandInterface.h"
#include "Interfaces/LeaderInterface.h"

struct FPathPointData;
class USquadCommand;
struct FSquadMemberPositionInfo;
class IMemberInterface;
struct FSquadCommandPayload;

struct FComputedLocationInfo
{
	FVector Location = FVector::ZeroVector;
	FRotator Rotation = FRotator::ZeroRotator;
	int32 SquadID = -1;
	uint8 Flags = 0;
	uint8 Reserved1 = 0;
	uint8 Reserved2 = 0;
	uint8 Reserved3 = 0;

	FORCEINLINE bool HasMoved() const
	{
		return (Flags& ESquadFlags::HasMoved) != 0;
	}
	FORCEINLINE void SetHasMoved(bool b)
	{
		if (b)
			Flags |= ESquadFlags::HasMoved;
		else
			Flags &= ~ESquadFlags::HasMoved;
	}
	
	FORCEINLINE bool HasReachedDestination() const
	{
		return (Flags& ESquadFlags::HasReachedDestination) != 0;
	}
	FORCEINLINE void SetHasReachedDestination(bool b)
	{
		if (b)
			Flags |= ESquadFlags::HasReachedDestination;
		else
			Flags &= ~ESquadFlags::HasReachedDestination;
	}
};

class SUPERDEFENCE_API FMovementHelper
{
public:
	FMovementHelper();
	~FMovementHelper();

	void Init(const TArray<FPathPointData>& InPathPoints, const TMap<int32, IMemberInterface*>& InSquadMembers, AActor* InLeaderActor);
	
	void RegisterSquadMember(const int32& InSquadMemberID, IMemberInterface* SquadMember);
	void UnregisterSquadMember(const int32& InSquadMemberID);

	void MoveSquad(const float DeltaTime);
	void MoveLeader();

	static FComputedLocationInfo ComputeLocationForMember(
		int32 InSquadMemberID,
		const FSquadMemberPositionInfo& Info,
		const FPathPointData& TargetLocation,
		const TArray<FPathPointData>& CurrentPath,
		float DeltaTime);

	void FlushMovementQueue();
	void Cleanup();

private:

	TSoftObjectPtr<AActor> LeaderActor;
	TMap<int32, IMemberInterface*> SquadMembers;
	TMap<int32, const FSquadMemberPositionInfo*> MembersBySquadID;
	TMap<int32, FPathPointData> MemberPaths;
	TArray<FPathPointData> CurrentPathPoints;
	TArray<int32> FinishedMembers;

	TQueue<TPair<int32, FComputedLocationInfo>, EQueueMode::Mpsc> MemberPositionUpdateQueue;
};
