// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/SquadEnemyData.h"
#include "Subsystems/WorldSubsystem.h"
#include "SquadMemberSubsystem.generated.h"

struct FGameplayTag;
struct FEnemyStats;
/**
 * 
 */
UCLASS()
class SUPERDEFENCE_API USquadMemberSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	FORCEINLINE USquadEnemyData* GetSquadEnemyData() const { return SquadEnemyData; }
	
private:
	UPROPERTY()
	USquadEnemyData* SquadEnemyData = nullptr;
};