// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Interaction/CombatInterface.h"
#include "GameplayTagContainer.h"
#include "SDCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;
class UGameplayAbility;
struct FOnAttributeChangeData;
struct FGameplayAbilitySpecHandle;
class UAIPerceptionStimuliSourceComponent;
class UAISense;

UCLASS(Abstract)
class SUPERDEFENCE_API ASDCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	ASDCharacterBase();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	/* Combat Interface */
	virtual FVector GetCombatSocketLocation() override;
	virtual FVector GetTargetVelocity() const override;
	virtual UAnimMontage* GetHitReactMontage_Implementation() override;
	virtual UAnimMontage* GetShootingMontage_Implementation() override;
	virtual void Die() override;
	virtual bool IsDead() override;
	virtual void ReceiveDamage(const FGameplayEffectSpecHandle& DamageEffectSpecHandle) override;
	/* End Combat Interface */

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	/* OVERRIDE IN CHILD CLASSES */
	virtual void InitAbilityActorInfo() {};
	virtual void InitializeDefaultAttributes() const {};
	virtual void AddCharacterAbilities() {};
	/* OVERRIDE IN CHILD CLASSES */

	/* GAS Helper Functions */
	void ApplyEffectToSelf(const TSubclassOf<UGameplayEffect>& GameplayEffectClass, float Level) const;
	void RemoveGameplayEffectByTag(const FGameplayTagContainer& TagsToRemove) const;

	UFUNCTION(BlueprintCallable)
	bool HasGameplayTag(const FGameplayTagContainer TagsToCheck) const;
	/* GAS Helper Functions */

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "SDCharacter|GAS|Tags")
	FGameplayTag ActorTypeTag;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "SDCharacter")
	bool bDrawDebug = false;
#endif

	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_HandleDeath();

	/* PERCEPTION for the TurretAI */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SDCharacter|GAS|Perception")
	UAIPerceptionStimuliSourceComponent* StimuliSourceComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SDCharacter|GAS|Perception")
	TArray<TSubclassOf<UAISense>> StartingSenses;
	TArray<TSubclassOf<UAISense>> RegisteredSenses;

	void RegisterStartingSenses();
	void RegisterStimulus(const TSubclassOf<UAISense>& SenseToRegister);
	void UnregisterStimulus(const TSubclassOf<UAISense>& SenseToRegister);
	void UnregisterAllStimuli();
	/* PERCEPTION for the TurretAI */

private:

	UPROPERTY(EditAnywhere, Category = "SDCharacter|GAS|Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditAnywhere, Category = "SDCharacter|GAS|Combat")
	TObjectPtr<UAnimMontage> ShootingMontage;

	UPROPERTY(VisibleAnywhere, Category = "SDCharacter|GAS|Combat")
	bool bIsDead = false;
};