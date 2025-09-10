// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Combat/Ranged/SDShootAbility.h"
#include "SDShootPredictedProjectile.generated.h"

class ASDProjectile;

/**
 * 
 */
UCLASS()
class SUPERDEFENCE_API USDShootPredictedProjectile : public USDShootAbility
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnPredictedProjectile(const FHitResult& TargetTraceHit);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<ASDProjectile> ProjectileClass;
};
