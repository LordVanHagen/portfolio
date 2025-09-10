// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TurretBehaviour.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType, NotBlueprintable)
class UTurretBehaviour : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SUPERDEFENCE_API ITurretBehaviour
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	virtual bool Controller_TurretSelectTarget();
	UFUNCTION(BlueprintCallable)
	virtual void Character_TurretSetCombatState(bool bInCombat);
};
