// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SDEnemyRoute.generated.h"

class USplineComponent;

UCLASS()
class SUPERDEFENCE_API ASDEnemyRoute : public AActor
{
	GENERATED_BODY()
public:	
	ASDEnemyRoute();

	UFUNCTION(BlueprintImplementableEvent)
	USplineComponent* GetSplinePath();
};
