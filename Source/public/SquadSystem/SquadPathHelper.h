// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/CommandInterface.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SquadPathHelper.generated.h"

enum EVisibilitySetting : uint8;
struct FSquadCommandPayload;
struct FPathPointData;
class ASDEnemyRoute;

USTRUCT()
struct FLODDistanceSettings
{
	GENERATED_BODY()
	float LOD_TickInterval = 0.016f;
	float ClosestDistanceToPlayers = FLT_MAX;
	EVisibilitySetting VisibilitySetting = SkeletalMeshSetting;
	
};

/**
 * 
 */
UCLASS()
class SUPERDEFENCE_API USquadPathHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "SquadPathHelper|Routes")
	static ASDEnemyRoute* GetClosestRoute(AActor* StartingActor);

	UFUNCTION(BlueprintCallable, Category = "SquadPathHelper|Routes")
	static TArray<FPathPointData> GetPathPointsForRoute(ASDEnemyRoute* Path);
	
	UFUNCTION(BlueprintCallable, Category = "SquadPathHelper|Routes")
	static FSquadCommandPayload GetCommandPayloadFromPathPoints(const TArray<FPathPointData>& PathPoints);

	UFUNCTION(BlueprintCallable, Category = "SquadPathHelper|RangeCheck")
	static float GetDistanceToClosestPlayer(const AActor* ActorToCheckFrom);

	UFUNCTION(Category = "SquadPathHelper|RangeCheck")
	static FLODDistanceSettings GetTickIntervalFromLODs(const float Distance);
};
