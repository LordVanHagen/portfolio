// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SquadDamageHelper.generated.h"

/**
 * 
 */

struct FGameplayTag;
struct FGameplayEffectSpecHandle;

USTRUCT(BlueprintType)
struct FEnemyStats
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squads")
	float MaxHealth = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Squads")
	float CurrentHealth = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squads")
	float MaxArmor = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Squads")
	float CurrentArmor = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squads")
	float MaxMovementSpeed = 0.f;
};

UCLASS()
class SUPERDEFENCE_API USquadDamageHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "SquadDamageHelper|Damage")
	static float GetDamage(const FEnemyStats& EnemyStats, const FGameplayEffectSpecHandle& ContextHandle);

	static const FEnemyStats* GetStatsForType(const UObject* WorldContextObject, const FGameplayTag& EnemyTypeTag);
};
