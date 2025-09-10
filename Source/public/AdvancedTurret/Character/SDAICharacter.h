// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/SDCharacterBase.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "SDAICharacter.generated.h"

/**
 * 
 */
UCLASS()
class SUPERDEFENCE_API ASDAICharacter : public ASDCharacterBase
{
	GENERATED_BODY()
public:
	ASDAICharacter();

	/* Combat Interface */
	UFUNCTION(BlueprintCallable)
	virtual FVector GetCombatSocketLocation() override;
	virtual int32 GetPlayerLevel() override;
	/* End Combat Interface */

protected:
	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo() override;
	virtual void InitializeDefaultAttributes() const override;
	virtual void AddCharacterAbilities() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SDCharacter|Character Class Defaults")
	ECharacterClass CharacterClass = ECharacterClass::SpeedSwarmer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SDCharacter|Character Class Defaults")
	int32 PlayerLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SDCharacter|NEEDS TO BE SET", meta = (ToolTip = "The Socket this Character is Shooting from"))
	FName CombatSocketName;
};
