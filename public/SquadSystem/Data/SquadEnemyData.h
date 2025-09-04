// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "Squads/SquadDamageHelper.h"
#include "SquadEnemyData.generated.h"

/**
 * 
 */

UCLASS()
class SUPERDEFENCE_API USquadEnemyData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad Data")
	TMap<FGameplayTag, FEnemyStats> EnemyStatsMap;
};
