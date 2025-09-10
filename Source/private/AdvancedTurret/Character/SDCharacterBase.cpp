// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SDCharacterBase.h"
#include "AbilitySystem/SDAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "SUPERDEFENCE/SUPERDEFENCE.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"

ASDCharacterBase::ASDCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetCollisionResponseToChannel(ECC_TurretSpawnChannel, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("StimuliSourceComponent");
	StimuliSourceComponent->bAutoRegister = true;
	StimuliSourceComponent->PrimaryComponentTick.bCanEverTick = false;
	StimuliSourceComponent->PrimaryComponentTick.bStartWithTickEnabled = false;
}

UAbilitySystemComponent* ASDCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ASDCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	RegisterStartingSenses();
}

FVector ASDCharacterBase::GetCombatSocketLocation()
{
	//TODO: When weapon system is implemeted, do something like EquippedWeapon->GetMesh()->GetSocketLocation(WeaponFireSocketName)
	return GetActorLocation();
}

FVector ASDCharacterBase::GetTargetVelocity() const
{
	return GetCharacterMovement()->Velocity;
}

UAnimMontage* ASDCharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

UAnimMontage* ASDCharacterBase::GetShootingMontage_Implementation()
{
	return ShootingMontage;
}

void ASDCharacterBase::Die()
{
	bIsDead = true;

	UnregisterAllStimuli();

	Multicast_HandleDeath();
}

bool ASDCharacterBase::IsDead()
{
	return bIsDead;
}

void ASDCharacterBase::ReceiveDamage(const FGameplayEffectSpecHandle& DamageEffectSpecHandle)
{
	if (DamageEffectSpecHandle.IsValid())
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
}

void ASDCharacterBase::Multicast_HandleDeath_Implementation()
{
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetMesh()->AddImpulse(FVector(0.f, 0.f, 100.f), NAME_None, true);
}

#pragma region Helper Functions
void ASDCharacterBase::ApplyEffectToSelf(const TSubclassOf<UGameplayEffect>& GameplayEffectClass, const float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(GameplayEffectClass);
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void ASDCharacterBase::RemoveGameplayEffectByTag(const FGameplayTagContainer& TagsToRemove) const
{
	check(IsValid(GetAbilitySystemComponent()));
	AbilitySystemComponent->RemoveActiveEffectsWithAppliedTags(TagsToRemove);
}

bool ASDCharacterBase::HasGameplayTag(const FGameplayTagContainer TagsToCheck) const
{
	check(IsValid(GetAbilitySystemComponent()));
	if (AbilitySystemComponent->HasAllMatchingGameplayTags(TagsToCheck))
		return true;
	return false;
}
#pragma endregion

/* PERCEPTION */
void ASDCharacterBase::RegisterStartingSenses()
{
	if (StimuliSourceComponent)
	{
		for (auto Sense : StartingSenses)
		{
			StimuliSourceComponent->RegisterForSense(Sense);
			RegisteredSenses.AddUnique(Sense);
		}
		StimuliSourceComponent->RegisterWithPerceptionSystem();
	}
}

void ASDCharacterBase::RegisterStimulus(const TSubclassOf<UAISense>& SenseToRegister)
{
	if (StimuliSourceComponent && !RegisteredSenses.Contains(SenseToRegister))
	{
		StimuliSourceComponent->RegisterForSense(SenseToRegister);
		RegisteredSenses.AddUnique(SenseToRegister);
	}
}

void ASDCharacterBase::UnregisterStimulus(const TSubclassOf<UAISense>& SenseToRegister)
{
	if (StimuliSourceComponent && RegisteredSenses.Contains(SenseToRegister))
	{
		StimuliSourceComponent->UnregisterFromSense(SenseToRegister);
		RegisteredSenses.RemoveSingle(SenseToRegister);
	}
}

void ASDCharacterBase::UnregisterAllStimuli()
{
	if (StimuliSourceComponent)
	{
		for (const auto Sense : RegisteredSenses)
		{
			StimuliSourceComponent->UnregisterFromSense(Sense);
		}
		RegisteredSenses.Empty();
		StimuliSourceComponent->UnregisterFromPerceptionSystem();
	}
}
/* PERCEPTION */