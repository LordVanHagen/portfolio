// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Combat/Ranged/SDShootProjectile.h"
#include "Actor/SDProjectile.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "SDGameplayTags.h"
#include "Game/SDGameStateBase.h"
#include "Combat/Data/SDAmmunitionDataInfo.h"

void USDShootProjectile::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                         const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void USDShootProjectile::SpawnProjectile(const FHitResult& TargetTraceHit)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer)
		return;

	const FAmmunitionData AmmunitionData = GetWorld()->GetGameStateChecked<ASDGameStateBase>()->AmmunitionDataInfo->GetAmmunitionDataFromTag(AmmunitionTypeTag);
		
	const FVector StartingLocation = TargetTraceHit.TraceStart;
	const FRotator Rotation = (TargetTraceHit.Location - TargetTraceHit.TraceStart).Rotation();

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