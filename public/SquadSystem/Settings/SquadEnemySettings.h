// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SquadEnemySettings.generated.h"

class USquadEnemyData;
/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta = (DisplayName="Squad Enemy Settings"))
class SUPERDEFENCE_API USquadEnemySettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", AdvancedDisplay)
	TSoftObjectPtr<USquadEnemyData> SquadData;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	float TickInterval = 0.016f;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	float LOD_1_Distance = 2000.0f;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	float LOD_1_TickInterval = 0.33f;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	float LOD_2_Distance = 5000.0f;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	float LOD_2_TickInterval = 0.1f;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	float LOD_3_Distance = 10000.0f;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	float LOD_3_TickInterval = 1.0f;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	float LeaderDistanceCheckRadius = 850.f;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	float LeaderDistanceCheckTimer = 1.f;
};
