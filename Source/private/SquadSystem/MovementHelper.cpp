// Fill out your copyright notice in the Description page of Project Settings.


#include "Squads/MovementHelper.h"

#include "Squads/Interfaces/MemberInterface.h"
#include "Squads/SquadCommands/SquadCommands.h"

FMovementHelper::FMovementHelper()
{
}

FMovementHelper::~FMovementHelper()
{
}

void FMovementHelper::Init(const TArray<FPathPointData>& InPathPoints, const TMap<int32, IMemberInterface*>& InSquadMembers, AActor* InLeaderActor)
{
	SquadMembers = InSquadMembers;
	CurrentPathPoints = InPathPoints;
	MembersBySquadID.Empty();
	MemberPaths.Empty();
	LeaderActor = InLeaderActor;
	
	if (InPathPoints.IsEmpty() || InSquadMembers.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("MovementHelper failed. Either PathPoints or SquadMembers are 0."));	
		return;	
	}
	
	for (const auto& Member : InSquadMembers)
	{
		MembersBySquadID.Add(Member.Key, Member.Value->GetPositionInfo());
		MemberPaths.Add(Member.Key, InPathPoints[0]);
	}
}

void FMovementHelper::RegisterSquadMember(const int32& InSquadMemberID, IMemberInterface* SquadMember)
{
	if (SquadMember && !MembersBySquadID.Contains(InSquadMemberID))
	{
		SquadMembers.Add(InSquadMemberID, SquadMember);
		MembersBySquadID.Add(InSquadMemberID, SquadMember->GetPositionInfo());
	}
}

void FMovementHelper::UnregisterSquadMember(const int32& InSquadMemberID)
{
	SquadMembers.Remove(InSquadMemberID);
	MembersBySquadID.Remove(InSquadMemberID);
}

void FMovementHelper::MoveSquad(const float DeltaTime)
{
	TArray<int32> MemberIDs;
	MembersBySquadID.GetKeys(MemberIDs);

	const TMap<int32, const FSquadMemberPositionInfo*>& MemberSnapShot = MembersBySquadID;
	const TMap<int32, FPathPointData>& MemberPathSnapShot = MemberPaths;
	const TArray<FPathPointData>& PathRef = CurrentPathPoints;

	TArray<FComputedLocationInfo> NewLocations;

	ParallelFor(MemberIDs.Num(), [this, &MemberIDs, &MemberSnapShot, &MemberPathSnapShot, &PathRef, DeltaTime](const int32 Index)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(MoveSquad_WorkItem);
		const int32 ThreadID = FPlatformTLS::GetCurrentThreadId();
		TRACE_BOOKMARK(TEXT("Running on Thread %d (Index %d)"), ThreadID, Index);
		
		int32 MemberID = MemberIDs[Index];
		const FSquadMemberPositionInfo* Info = MemberSnapShot[MemberID];

		if (!Info)
			return;

		const FPathPointData& TargetLocation = MemberPathSnapShot[MemberID];

		if (FComputedLocationInfo ComputedLocation = ComputeLocationForMember(MemberID, *Info, TargetLocation, PathRef, DeltaTime); ComputedLocation.HasMoved())
		{
			MemberPositionUpdateQueue.Enqueue({MemberID, ComputedLocation});
		}
	}, EParallelForFlags::Unbalanced);
}

void FMovementHelper::MoveLeader()
{
	if (MembersBySquadID.Num() == 0 || !LeaderActor.IsValid())
		return;

	FVector AveragePosition = FVector::ZeroVector;
	int32 Count = 0;

	for (const auto& Pair : MembersBySquadID)
	{
		AveragePosition += Pair.Value->CurrentPosition;
		Count++;
	}

	if (Count == 0)
		return;

	AveragePosition /= Count;

	if (FVector::DistSquared(LeaderActor.Get()->GetActorLocation(), AveragePosition) > FMath::Square(5.f))
	{
		LeaderActor.Get()->SetActorLocation(AveragePosition);
	}
}

FComputedLocationInfo FMovementHelper::ComputeLocationForMember(const int32 InSquadMemberID, const FSquadMemberPositionInfo& Info, const FPathPointData& TargetLocation, const TArray<FPathPointData>& CurrentPath, float DeltaTime)
{
	const FVector& Target = TargetLocation.PointLocation;
	const FVector Location = Info.CurrentPosition;
	const FRotator& Rotation = Info.CurrentRotation;
	
	FVector Direction = Target - Location;
	FComputedLocationInfo LocationInfo;
	LocationInfo.SquadID = InSquadMemberID;

	if (const float DistanceSqr = Direction.SizeSquared(); DistanceSqr < FMath::Square(Info.TargetAcceptanceRadius))
	{
		LocationInfo.SetHasReachedDestination(true);
		if (CurrentPath.Contains(TargetLocation))
		{
			if (const int32 NextIndex = CurrentPath.IndexOfByKey(TargetLocation) + 1; CurrentPath.IsValidIndex(NextIndex))
			{
				Direction = CurrentPath[NextIndex].PointLocation - Location;
			}
			else
			{
				// Visited Last Location
				return LocationInfo;
			}
		}
		else
		{
			return LocationInfo;
		}
	}

	if (!Direction.IsNearlyZero())
	{
		Direction.Normalize();
		const FVector DeltaMove = Direction * Info.MoveSpeed * DeltaTime;
		const FVector NewLocation = Location + DeltaMove;

		const FRotator NewRotation = Direction.Rotation();
		const FRotator Smoothed = FMath::RInterpTo(Rotation, NewRotation, DeltaTime, 5.f);

		LocationInfo.Location = NewLocation;
		LocationInfo.Rotation = Smoothed;
		LocationInfo.SetHasMoved(true);

		return LocationInfo;
	}

	return LocationInfo;
}


void FMovementHelper::FlushMovementQueue()
{
	const FVector& FinalTarget = CurrentPathPoints.Last().PointLocation;
	TPair<int32, FComputedLocationInfo> Pair;

	TRACE_CPUPROFILER_EVENT_SCOPE(MoveSquad_FlushCommands);
	const int32 ThreadID = FPlatformTLS::GetCurrentThreadId();
	TRACE_BOOKMARK(TEXT("Running on Thread %d"), ThreadID);
	
	while (MemberPositionUpdateQueue.Dequeue(Pair))
	{
		if (!Pair.Value.HasMoved())
		{
			if (FVector::DistSquared(Pair.Value.Location, FinalTarget) < FMath::Square(10.f))
			{
				MembersBySquadID.Remove(Pair.Key);
				FinishedMembers.Add(Pair.Key);
			}
			continue;
		}

		if (IMemberInterface* const* Member = SquadMembers.Find(Pair.Key))
		{
			(*Member)->Move(Pair.Value);
		}
	}
	
	MoveLeader();
}

void FMovementHelper::Cleanup()
{
	SquadMembers.Empty();
	MembersBySquadID.Empty();
	MemberPaths.Empty();
	CurrentPathPoints.Empty();
	FinishedMembers.Empty();
	MemberPositionUpdateQueue.Empty();
}
