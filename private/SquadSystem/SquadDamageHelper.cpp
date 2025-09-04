// Fill out your copyright notice in the Description page of Project Settings.


#include "Squads/SquadDamageHelper.h"

#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "SDGameplayTags.h"
#include "Squads/SquadMemberSubsystem.h"

float USquadDamageHelper::GetDamage(const FEnemyStats& EnemyStats, const FGameplayEffectSpecHandle& ContextHandle)
{
	if (!ContextHandle.IsValid())
		return 0.f;
	
	const FGameplayTag DamageTag = FSDGameplayTags::Get().Damage;
	//const FGameplayTag ArmorPenTag = FSDGameplayTags::Get().ArmorPenetration;
	
	const FGameplayEffectSpec* Spec = ContextHandle.Data.Get();
	if (!Spec)
		return 0.f;
	
	const int32 RawDamage = Spec->GetSetByCallerMagnitude(DamageTag);
	//const float ArmorPen = Spec->GetSetByCallerMagnitude(ArmorPenTag);

	//const float ClampedArmorPen = FMath::Clamp(ArmorPen, 0.f, EnemyStats.CurrentArmor);
	//const float EffectiveArmor = FMath::Max(0.f, EnemyStats.CurrentArmor - ClampedArmorPen);

	//const float FinalDamage = FMath::Max(0.f, RawDamage - EffectiveArmor);	
	
	//return FinalDamage;
	return RawDamage;
}

const FEnemyStats* USquadDamageHelper::GetStatsForType(const UObject* WorldContextObject, const FGameplayTag& EnemyTypeTag)
{
	if (!WorldContextObject)
		return nullptr;
	const UWorld* World = WorldContextObject->GetWorld();
	if (!World)
		return nullptr;
	const USquadMemberSubsystem* Subsystem = World->GetSubsystem<USquadMemberSubsystem>();
	if (!Subsystem || !Subsystem->GetSquadEnemyData())
		return nullptr;

	return Subsystem->GetSquadEnemyData()->EnemyStatsMap.Find(EnemyTypeTag);
}
