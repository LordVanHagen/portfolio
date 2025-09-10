// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/SDGameplayAbility.h"
#include "SDShootAbility.generated.h"

/**
 * 
 */
UCLASS()
class SUPERDEFENCE_API USDShootAbility : public USDGameplayAbility
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AmmunitionTypeTag;
};
