// Fill out your copyright notice in the Description page of Project Settings.


#include "Squads/SquadPathHelper.h"

#include "Character/SDPlayerCharacter.h"
#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Settings/SquadEnemySettings.h"
#include "Squads/Interfaces/CommandInterface.h"
#include "Squads/Navigation/SDEnemyRoute.h"

ASDEnemyRoute* USquadPathHelper::GetClosestRoute(AActor* StartingActor)
{
	TArray<AActor*> RoutesToFind;
	UGameplayStatics::GetAllActorsOfClass(StartingActor->GetWorld(), ASDEnemyRoute::StaticClass(), RoutesToFind);

	if (RoutesToFind.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Route could not be found, no routes placed inside the level!"));
		return nullptr;
	}

	ASDEnemyRoute* ClosestRoute  = nullptr;
	float ClosestDistance = FLT_MAX;
	const FVector LeaderLocation = StartingActor->GetActorLocation();
	
	for (AActor* PossibleRoute : RoutesToFind)
	{
		if (ASDEnemyRoute* Route = Cast<ASDEnemyRoute>(PossibleRoute))
		{
			const float Distance = FVector::Distance(LeaderLocation, Route->GetActorLocation());

			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestRoute = Route;
			}
		}
	}

	if (ClosestRoute == nullptr)
		UE_LOG(LogTemp, Error, TEXT("Closest route was nullptr. Did you assign the correct subclass?"));

	return ClosestRoute;
}

TArray<FPathPointData> USquadPathHelper::GetPathPointsForRoute(ASDEnemyRoute* Path)
{
	TArray<FPathPointData> PathPoints;
	if (Path == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Path was nullptr."));
		return PathPoints;
	}

	const USplineComponent* PathSpline = Path->GetSplinePath();
	const int32 SplinePoints = PathSpline->GetNumberOfSplinePoints();
	
	for (int32 i = 0; i < SplinePoints; i++)
	{
		const FTransform SplinePointTransform = PathSpline->GetTransformAtSplinePoint(i, ESplineCoordinateSpace::World);
		FPathPointData SplinePointData;
		SplinePointData.PointLocation = SplinePointTransform.GetLocation();
		PathPoints.Add(SplinePointData);
	}

	return PathPoints;
}

FSquadCommandPayload USquadPathHelper::GetCommandPayloadFromPathPoints(const TArray<FPathPointData>& PathPoints)
{
	FSquadCommandPayload CommandPayload;
	CommandPayload.PathPoints = PathPoints;
	return CommandPayload;
}

float USquadPathHelper::GetDistanceToClosestPlayer(const AActor* ActorToCheckFrom)
{
	float ClosestDistance = FLT_MAX;
	if (ActorToCheckFrom)
	{
		TArray<AActor*> Players;
		UGameplayStatics::GetAllActorsOfClass(ActorToCheckFrom->GetWorld(), ASDPlayerCharacter::StaticClass(), Players);
		if (Players.Num() > 0)
		{
			for (const AActor* Player : Players)
			{
				if (const float DistanceToPlayer = ActorToCheckFrom->GetDistanceTo(Player); DistanceToPlayer < ClosestDistance)
					ClosestDistance = DistanceToPlayer;
			}
		}
	}
	return ClosestDistance;
}

FLODDistanceSettings USquadPathHelper::GetTickIntervalFromLODs(const float Distance)
{
	const USquadEnemySettings* Settings = GetDefault<USquadEnemySettings>();
	FLODDistanceSettings LODDistanceSettings;
	LODDistanceSettings.ClosestDistanceToPlayers = Distance;

	if (Distance >= Settings->LOD_3_Distance)
	{
		LODDistanceSettings.VisibilitySetting = StaticMeshSetting;
		LODDistanceSettings.LOD_TickInterval = Settings->LOD_3_TickInterval;
	}
	else if (Distance >= Settings->LOD_2_Distance && Distance < Settings->LOD_3_Distance)
	{
		LODDistanceSettings.VisibilitySetting = StaticMeshSetting;
		LODDistanceSettings.LOD_TickInterval = Settings->LOD_2_TickInterval;
	}
	else
	{
		LODDistanceSettings.VisibilitySetting = SkeletalMeshSetting;
		LODDistanceSettings.LOD_TickInterval = (Distance < Settings->LOD_2_Distance && Distance >= Settings->LOD_1_Distance) ? Settings->LOD_1_TickInterval: Settings->TickInterval;	
	}
	
	return LODDistanceSettings;
}
