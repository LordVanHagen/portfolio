// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Combat/Ranged/SDShootPredictedProjectile.h"
#include "Combat/Data/SDAmmunitionDataInfo.h"
#include "Game/SDGameStateBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "SDGameplayTags.h"
#include "Actor/SDProjectile.h"
#include "Combat/SDTargetingLibrary.h"
#include "Interaction/CombatInterface.h"

void USDShootPredictedProjectile::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void USDShootPredictedProjectile::SpawnPredictedProjectile(const FHitResult& TargetTraceHit)
{
	if (const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority(); !bIsServer)
		return;

	const FAmmunitionData AmmunitionData = GetWorld()->GetGameStateChecked<ASDGameStateBase>()->AmmunitionDataInfo->GetAmmunitionDataFromTag(AmmunitionTypeTag);

	const FVector StartingLocation = TargetTraceHit.TraceStart;

	FVector TargetLocation = TargetTraceHit.Location;

	if (const ICombatInterface* CombatInterface = Cast<ICombatInterface>(TargetTraceHit.GetActor()))
	{
		TargetLocation = USDTargetingLibrary::PredictTargetLocationBallistic(
			TargetTraceHit.TraceStart,
			TargetTraceHit.Location,
			CombatInterface->GetTargetVelocity(),
			AmmunitionData.InitialSpeed);
	}

	const FRotator Rotation = (TargetLocation - TargetTraceHit.TraceStart).Rotation();

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(StartingLocation);
	SpawnTransform.SetRotation(Rotation.Quaternion());

	ASDProjectile* Projectile = GetWorld()->SpawnActorDeferred<ASDProjectile>(
		ProjectileClass,
		SpawnTransform,
		GetOwningActorFromActorInfo(),
		Cast<APawn>(GetOwningActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(AmmunitionData.DamageEffectClass, GetAbilityLevel(), SourceASC->MakeEffectContext());

	const FSDGameplayTags GameplayTags = FSDGameplayTags::Get();
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Damage, AmmunitionData.Damage);
	Projectile->DamageEffectSpecHandle = SpecHandle;

	Projectile->SetAmmunitionData(AmmunitionData);
	Projectile->SetHitResult(TargetTraceHit);
	
	Projectile->FinishSpawning(SpawnTransform);
}
