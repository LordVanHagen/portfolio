// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Combat/Ranged/SDShootAbility.h"
#include "SDShootProjectile.generated.h"

class ASDProjectile;

/**
 * 
 */
UCLASS()
class SUPERDEFENCE_API USDShootProjectile : public USDShootAbility
{
	GENERATED_BODY()
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile(const FHitResult& TargetTraceHit);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<ASDProjectile> ProjectileClass;
};
