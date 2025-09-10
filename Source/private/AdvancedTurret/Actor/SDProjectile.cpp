// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/SDProjectile.h"

#include "GameplayEffect.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Combat/SDTargetingLibrary.h"
#include "Engine/OverlapResult.h"
#include "Interaction/CombatInterface.h"
#include "SUPERDEFENCE/SUPERDEFENCE.h"

ASDProjectile::ASDProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->bSweepCollision = true;
}

void ASDProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeSpan);
	SetReplicateMovement(true);
	Sphere->OnComponentHit.AddDynamic(this, &ASDProjectile::OnSphereHit);

	if (LoopingSoundComponent)
		LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());
}

void ASDProjectile::OnHit()
{
	UGameplayStatics::PlaySoundAtLocation(this, AmmunitionData.ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, AmmunitionData.ImpactEffect, GetActorLocation());
	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}
	bHit = true;
}

void ASDProjectile::Destroyed()
{
	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}
	if (!bHit && !HasAuthority())
		OnHit();
	Super::Destroyed();
}

void ASDProjectile::OnSphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (DamageEffectSpecHandle.Data.IsValid() && DamageEffectSpecHandle.Data.Get()->GetContext().GetEffectCauser() == OtherActor)
	return;
	
	if (!bHit)
		OnHit();

	if (HasAuthority())
	{
		if (AmmunitionData.bIsAOE)
			DoAoeDamage(Hit);
		else
			DoDamage(OtherActor);
		
		Destroy();
	}
	else
		bHit = true;
}

void ASDProjectile::DoDamage(AActor* OtherActor) const
{
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(OtherActor))
	{
		if (!CombatInterface->IsDead())
			CombatInterface->ReceiveDamage(DamageEffectSpecHandle);
	}
}

void ASDProjectile::DoAoeDamage(const FHitResult& ImpactPoint) const
{
	const FVector ExplosionStartLocation = ImpactPoint.Location;
	const float BlastRadius = AmmunitionData.BlastRadius;
	TArray<FOverlapResult> OverlapResults;
	const FCollisionShape CollisionShape = FCollisionShape::MakeSphere(BlastRadius);

	const bool bAnyHit = GetWorld()->OverlapMultiByChannel(
	 	OverlapResults,
	 	ExplosionStartLocation,
	 	FQuat::Identity,
	 	ECC_Projectile,
	 	CollisionShape);

#if WITH_EDITOR
	if (bDrawDebug)
	{
		DrawDebugSphere(
			GetWorld(),
			ExplosionStartLocation,
			BlastRadius,
			12,
			FColor::Red,
			false,
			2.f
		);
	}
#endif
	
	if (!bAnyHit)
		return;

	TSet<AActor*> AlreadyHitActors;
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		if (OverlapResult.GetActor()->ActorHasTag("Player"))
			continue;

		AActor* HitActor = OverlapResult.GetActor();
		if (!HitActor || AlreadyHitActors.Contains(HitActor))
			continue;

		AlreadyHitActors.Add(HitActor);
		
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(HitActor))
		{
			CombatInterface->ReceiveDamage(DamageEffectSpecHandle);
			
#if WITH_EDITOR
			if (bDrawDebug && OverlapResult.GetActor() != nullptr) 
			{
				DrawDebugBox(
					GetWorld(),
					OverlapResult.GetActor()->GetActorLocation(),
					FVector(10.f),
					FColor::Green,
					false,
					2.f
				);
			}
#endif
		}
	}
}

void ASDProjectile::SetAmmunitionData(const FAmmunitionData& InAmmunitionData)
{
	AmmunitionData = InAmmunitionData;
	ProjectileMovement->InitialSpeed = AmmunitionData.InitialSpeed;
}

FVector ASDProjectile::GetTargetDirection() const
{
	AActor* HitActor = HitResult.GetActor();
	const FVector CurrentLocation = GetActorLocation();
	if (!HitActor)
	{
		const FVector FallbackDirection = (HitResult.Location - CurrentLocation).GetSafeNormal();
		return NearZeroCheck(FallbackDirection);
	}

	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(HitActor))
	{
		const FVector TargetLocation = CombatInterface->GetCombatSocketLocation();
		const FVector TargetVelocity = CombatInterface->GetTargetVelocity();
		const float ProjectileSpeed = AmmunitionData.InitialSpeed;

		if (TargetVelocity.IsNearlyZero())
		{
			UE_LOG(LogTemp, Warning, TEXT("ASDProjectile::GetTargetDirection - CombatInterface TargetLocation is Zero!"));
			return GetActorForwardVector();
		}

		const FVector PredictedLocation = USDTargetingLibrary::PredictTargetLocationBallistic(
			CurrentLocation,
			TargetLocation,
			TargetVelocity,
			ProjectileSpeed
		);

		const FVector PredictedDirection = (PredictedLocation - CurrentLocation).GetSafeNormal();
		return NearZeroCheck(PredictedDirection);
	}

	const FVector DefaultDirection = (HitResult.Location - GetActorLocation()).GetSafeNormal();
	return NearZeroCheck(DefaultDirection);
}

FVector ASDProjectile::NearZeroCheck(const FVector& VectorToCheck) const
{
	return VectorToCheck.IsNearlyZero() ? GetActorForwardVector() : VectorToCheck;
}
