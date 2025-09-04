// Fill out your copyright notice in the Description page of Project Settings.


#include "Squads/Wrappers/SquadLeader.h"

#include "Settings/SquadEnemySettings.h"
#include "Squads/SquadFactory.h"
#include "Squads/SquadPathHelper.h"
#include "Squads/Interfaces/CommandInterface.h"
#include "Squads/Interfaces/MemberInterface.h"

USquadLeader::USquadLeader()
{
	
}

void USquadLeader::BeginDestroy()
{
	Reset();
	Super::BeginDestroy();
}

void USquadLeader::Init(AActor* Outer)
{
	Leader = Outer;
	Reset();
}

void USquadLeader::AddMember(IMemberInterface* NewMember)
{
	if (ensure(NewMember))
	{
		const int32 NewSquadID = GetNewSquadID();
		NewMember->SetMemberID(NewSquadID);
		MembersBySquadID.Add(NewSquadID, NewMember);
	}
}

void USquadLeader::RemoveMember(IMemberInterface* MemberToRemove)
{
	if (MemberToRemove)
	{
		MembersBySquadID.Remove(MemberToRemove->GetMemberID());
		MovementHelper.UnregisterSquadMember(MemberToRemove->GetMemberID());
	}

	if (MembersBySquadID.Num() == 0)
		TerminateSquad();
}

TArray<IMemberInterface*> USquadLeader::GetMembers()
{
	TArray<IMemberInterface*> Members;

	if (!MembersBySquadID.IsEmpty())
	{
		for (auto Member : MembersBySquadID)
		{
			Members.Add(Member.Value);
		}
	}
	return Members;
}

void USquadLeader::Join(ILeaderInterface* Other)
{
	// if we have very little squadmembers, look for other squadleaders to join them
	// give all of our squad, remove them from us and destroy self
}

void USquadLeader::Split()
{
	// One condition, could be more
	/*if (MembersBySquadID.Num() >= SquadSplitAmount)
	{
		// Create new Squadleader, give it half of our squad and remove them afterwards
	}*/
}

void USquadLeader::ExecuteCommand(USquadCommand* Command, const FSquadCommandPayload& SquadCommandPayload)
{
	for (const auto& Member : MembersBySquadID)
	{
		if (Member.Value)
			Member.Value->ExecuteCommand(Command, SquadCommandPayload);
	}
}

void USquadLeader::NotifySquadReady()
{
	DoRangeCheck();
	StartSquadMovement();
	EnableRangeCheckTimer(true);
	
	//ASDEnemyRoute* ClosestPath = USquadPathHelper::GetClosestRoute(Leader);
	//const FSquadCommandPayload CommandPayload = USquadPathHelper::GetCommandPayloadFromPathPoints(USquadPathHelper::GetPathPointsForRoute(ClosestPath));
	//USquadCommand* Command = USquadFactory::GetCommand(Leader, "GoTo");
	//CurrentCommand = Command;
	//ExecuteCommand(Command,  CommandPayload);	
}

void USquadLeader::Blueprint_AddMember(UObject* NewMember)
{
	if (NewMember && NewMember->GetClass()->ImplementsInterface(UMemberInterface::StaticClass()))
	{
		if (IMemberInterface* Member =  Cast<IMemberInterface>(NewMember))
		{
			AddMember(Member);
			return;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("Member could not be added."));
}

void USquadLeader::Tick(const float DeltaTime)
{
	if (CurrentTickInterval <= 0.f)
	{
		TickSquad(DeltaTime);
		return;
	}

	TimeSinceLastTick += DeltaTime;

	if (TimeSinceLastTick >= CurrentTickInterval)
	{
		TickSquad(TimeSinceLastTick);
		TimeSinceLastTick = 0.f;
	}
}

TStatId USquadLeader::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USquadLeader, STATGROUP_Tickables);
}

void USquadLeader::TerminateSquad()
{
	if (!MembersBySquadID.IsEmpty())
		MembersBySquadID.Empty();

	if (Leader)
		Leader->Destroy();
}

int32 USquadLeader::GetNewSquadID()
{
	while (MembersBySquadID.Contains(NextSquadID))
	{
		++NextSquadID;
	}
	return NextSquadID;
}

void USquadLeader::StartSquadMovement()
{
	ASDEnemyRoute* ClosestRoute = USquadPathHelper::GetClosestRoute(Leader);
	const FSquadCommandPayload CommandPayload = USquadPathHelper::GetCommandPayloadFromPathPoints(USquadPathHelper::GetPathPointsForRoute(ClosestRoute));
	USquadCommand* Command = USquadFactory::GetCommand(Leader, "GoTo");
	CurrentCommand = Command;

	bIsTickEnabled = true;
	
	MovementHelper.Init(CommandPayload.PathPoints, MembersBySquadID, Leader);
	bMoveSquad = true;
}

void USquadLeader::TickSquad(float DeltaTime)
{
	if (bMoveSquad)
	{
		MovementHelper.MoveSquad(DeltaTime);
		MovementHelper.FlushMovementQueue();	
	}

#if WITH_EDITOR
	if (Leader)
		UE_LOG(LogTemp, Log, TEXT("Leader %s ticking with interval: %f"), *Leader->GetName(), CurrentTickInterval);
#endif
}

void USquadLeader::EnableRangeCheckTimer(const bool bEnable)
{
	const UWorld* World = GetWorld();
	if (bEnable)
		World->GetTimerManager().SetTimer(LeaderTimer, this, &USquadLeader::DoRangeCheck, GetSettings()->LeaderDistanceCheckTimer, true);
	else
	{
		if (World && World->GetTimerManager().IsTimerActive(LeaderTimer))
			World->GetTimerManager().ClearTimer(LeaderTimer);
	}
}

void USquadLeader::DoRangeCheck()
{
	const float ClosestPlayerDistance = USquadPathHelper::GetDistanceToClosestPlayer(Leader);
	UpdateTickInterval(ClosestPlayerDistance);
	// Do further checks here, like switching logic to attack players in range
	//TArray<AActor*> HostilesInRange = USDGameplayLibrary::GetAllHostilesWithTags();
}

void USquadLeader::UpdateTickInterval(const float DistanceToClosestPlayer)
{
	const FLODDistanceSettings LODDistanceSettings = USquadPathHelper::GetTickIntervalFromLODs(DistanceToClosestPlayer);
	const float NewTickInterval = LODDistanceSettings.LOD_TickInterval;
	if (FMath::IsNearlyEqual(CurrentTickInterval, NewTickInterval))
		return;

	CurrentTickInterval = NewTickInterval;
	UpdateSquadVisibilitySetting(LODDistanceSettings.VisibilitySetting);
}

void USquadLeader::UpdateSquadVisibilitySetting(const EVisibilitySetting InVisibilitySetting)
{
	for (const auto& Pair : MembersBySquadID)
	{
		Pair.Value->SetVisibilitySetting(InVisibilitySetting);
	}
}

const USquadEnemySettings* USquadLeader::GetSettings()
{
	return GetDefault<USquadEnemySettings>();
}

void USquadLeader::Reset()
{
	MembersBySquadID.Empty();
	bMoveSquad = false;
	bIsTickEnabled = false;
	EnableRangeCheckTimer(false);
	MovementHelper.Cleanup();
}
