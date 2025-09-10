// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SDAICharacter.h"
#include "AbilitySystem/SDAbilitySystemComponent.h"
#include "AbilitySystem/SDAttributeSet.h"
#include "AbilitySystem/SDAbilitySystemLibrary.h"

ASDAICharacter::ASDAICharacter()
{
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	AbilitySystemComponent = CreateDefaultSubobject<USDAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<USDAttributeSet>("AttributeSet");
}

FVector ASDAICharacter::GetCombatSocketLocation()
{
	return GetMesh()->GetSocketLocation(CombatSocketName);
}

int32 ASDAICharacter::GetPlayerLevel()
{
	return PlayerLevel;
}

void ASDAICharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		InitAbilityActorInfo();
		AddCharacterAbilities();
	}
}

void ASDAICharacter::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<USDAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();

	InitializeDefaultAttributes();
}

void ASDAICharacter::InitializeDefaultAttributes() const
{
	USDAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, PlayerLevel, AbilitySystemComponent);
}

void ASDAICharacter::AddCharacterAbilities()
{
	USDAbilitySystemLibrary::GiveStartupAbilities(this, CharacterClass, PlayerLevel, AbilitySystemComponent);
}
