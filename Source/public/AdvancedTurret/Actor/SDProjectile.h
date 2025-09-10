// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "Combat/Data/SDAmmunitionDataInfo.h"
#include "SDProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;

UCLASS()
class SUPERDEFENCE_API ASDProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	ASDProjectile();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
	FGameplayEffectSpecHandle DamageEffectSpecHandle;

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UFUNCTION(BlueprintCallable)
	virtual void OnHit();

	UFUNCTION()
	void OnSphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category="DEBUG")
	bool bDrawDebug = true;
#endif
	
private:
	UPROPERTY(EditDefaultsOnly)
	float LifeSpan = 15.f;
	bool bHit = false;

	FAmmunitionData AmmunitionData;
	FHitResult HitResult;
	bool bTracksEnemy = false;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootSceneComponent;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> LoopingSound;
	UPROPERTY()
	TObjectPtr<UAudioComponent> LoopingSoundComponent;

	void DoDamage(AActor* OtherActor) const;
	void DoAoeDamage(const FHitResult& ImpactPoint) const;

	FVector NearZeroCheck(const FVector& VectorToCheck) const;

public:
	
	void SetAmmunitionData(const FAmmunitionData& InAmmunitionData);
	FORCEINLINE FAmmunitionData GetAmmunitionData() const { return AmmunitionData; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FVector GetCurrentTargetLocation() const { return HitResult.Location; }
	void SetHitResult(const FHitResult& InHitResult) { HitResult = InHitResult; }
	UFUNCTION(BlueprintCallable)
	FVector GetTargetDirection() const;
};
