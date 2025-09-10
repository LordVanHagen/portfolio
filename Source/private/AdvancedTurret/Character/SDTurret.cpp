#include "Character/SDTurret.h"
#include "Net/UnrealNetwork.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SDTypes.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/SDAbilitySystemComponent.h"
#include "SDGameplayTags.h"

#pragma region Initialization
ASDTurret::ASDTurret()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ASDTurret::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->OnAbilityEnded.AddUObject(this, &ASDTurret::OnAbilityEnded);
	}
}

void ASDTurret::GetActorEyesViewPoint(FVector& Location, FRotator& Rotation) const
{
	Location = GetMesh()->GetSocketLocation(EyeSocketName);
	Rotation = GetMesh()->GetSocketRotation(EyeSocketName);
}

void ASDTurret::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASDTurret, CurrentTargetLocation);
	DOREPLIFETIME(ASDTurret, CurrentTargetDistance);
	DOREPLIFETIME(ASDTurret, bIsAiming);
	DOREPLIFETIME(ASDTurret, bIsTargetting);
	DOREPLIFETIME(ASDTurret, bIsRotatingBackToStart);
}
#pragma endregion

#pragma region Combat Interface
FTransform ASDTurret::GetAimTransform() const
{
	FTransform Transform;
	Transform.SetLocation(GetMesh()->GetSocketLocation(EyeSocketName));
	FRotator Rotation = GetMesh()->GetSocketRotation(EyeSocketName);
	Transform.SetRotation(Rotation.Quaternion());

	return Transform;
}

AActor* ASDTurret::Turret_GetCurrentTarget_Implementation() const
{
	return CurrentTarget;
}
#pragma endregion

void ASDTurret::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (IsValid(CurrentTarget))
	{
		bIsRotatingBackToStart = false;

		if (CurrentTarget != LastProcessedTarget)
		{
			bHasStartedTargeting = false;
			bLastTargetingStatus = false;
			LastProcessedTarget = CurrentTarget;

#if WITH_EDITOR
			if (bDrawDebug)
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("New Target Detected – Resetting Targeting Status"));
#endif
		}

		// Move barrel to target position, start shooting when barrel aligns
		InterpolateToPosition(DeltaSeconds, CurrentTarget->GetTargetLocation());
		CheckIfTargetAligned();
	}
	else
	{
		// Handle target loss
		if (bWasTargetValidLastTick)
		{
			OnTargetLost();
		}

		// Return barrel to starting position
		if (bIsRotatingBackToStart)
		{
			InterpolateToPosition(DeltaSeconds, StartingLocation);

#if WITH_EDITOR
			if (bDrawDebug)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Magenta, FString::Printf(TEXT("%s rotating back to start"), *GetNameSafe(this)));

				/*GEngine->AddOnScreenDebugMessage(3, 2.f, FColor::Magenta,
					FString::Printf(TEXT("CurrentTarget: %s | StartingLocation (Local): %s"),
						*(GetMesh()->GetComponentTransform().TransformPosition(CurrentTargetLocation) - TargetOffsetVector).ToString(),
						*StartingLocation.ToString()));*/

				DrawDebugSphere(GetWorld(), StartingLocation, 20.f, 12, FColor::Green);
				DrawDebugSphere(GetWorld(), GetMesh()->GetComponentTransform().TransformPosition(CurrentTargetLocation) - TargetOffsetVector, 20.f, 12, FColor::Red);

			}
#endif

			// When back at starting position, disable tick
			if ((GetMesh()->GetComponentTransform().TransformPosition(CurrentTargetLocation) - TargetOffsetVector).Equals(StartingLocation, 1.0f))
			{
				bIsRotatingBackToStart = false;

#if WITH_EDITOR
				if (bDrawDebug)
					GEngine->AddOnScreenDebugMessage(3, 2.f, FColor::Purple, FString::Printf(TEXT("%s arrived back at starting location"), *GetNameSafe(this)));
#endif
				SetActorTickEnabled(false);
			}
		}
	}

	bWasTargetValidLastTick = IsValid(CurrentTarget);
}

void ASDTurret::Character_TurretSetCombatState(bool bInCombat)
{
	switch (CurrentTurretState)
	{
	case ETurretState::Idle:
		if (bInCombat == true)
		{
			CurrentTurretState = ETurretState::Combat;
			Blackboard->SetValueAsEnum(TurretStateBlackboardValueName, static_cast<uint8>(CurrentTurretState));
			StartCombatState();
		}
		break;
	case ETurretState::Combat:
		if (bInCombat == false)
		{
			CurrentTurretState = ETurretState::Idle;
			Blackboard->SetValueAsEnum(TurretStateBlackboardValueName, static_cast<uint8>(CurrentTurretState));
			StartIdleState();
		}
		break;
	}
}

void ASDTurret::StartIdleState()
{
	OnTargetLost();
	bIsAiming = false;
	bIsTargetting = false;

	// maybe after delay? if so, better be put into Blueprint
	bIsRotatingBackToStart = true;

	// If firing in bursts, reset currentburstcount

	// disable effects and sounds specific to the turret finding a target via a RepNotify boolean variable
	// disable effects and sounds specific to targeting via a RepNotify boolean variable
}

/// <summary>
/// Gets called from Animation Montage, therefore gets called on every client.
/// This should only play sounds or vfx, may get replaced by Gameplay Cues at a later time
/// </summary>
void ASDTurret::TurretFire()
{
	UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFlash, GetMesh(), MuzzleFlashAttachmentPointName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
	// play shooting sound
}

/// <summary>
/// Handles the actual shooting process of the Turret.
/// Checks if the Turret possesses a specified Tag, if so tries to activate an GameplayAbility based on that Tag
/// </summary>
void ASDTurret::HandleShooting()
{
	if (!HasGameplayTag(TurretShootingTags))
	{
#if WITH_EDITOR
		if (bDrawDebug)
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, FString::Printf(TEXT("%s tried to Shoot without having the specified Tags"), *GetNameSafe(this)));
#endif
		return;
	}

	if (bool bActive = AbilitySystemComponent->TryActivateAbilitiesByTag(TurretShootingTags))
	{
#if WITH_EDITOR
		if (bDrawDebug)
			GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, FString::Printf(TEXT("%s activated an Ability"), *GetNameSafe(this)));
#endif
	}
}

/// <summary>
/// Gets called when the turret gets Destoyed from inside the Die() function
/// </summary>
void ASDTurret::DestroyTurret()
{
	// Disable all active VFX and Sounds before destruction
	// Play destruction VFX and Sound
	// If the destruction effects are being replicated, destory the Actor after a timer, so that the it will always get destroyed after the effects have been played
	Destroy();
}

#pragma region Tick Functionality
/*
 * Rotates the Turret to the CurrentTarget´s location at a given rotation speed
 */
void ASDTurret::InterpolateToPosition(float DeltaSeconds, const FVector& InPosition)
{
	TargetLocationWorldSpace = InPosition + TargetOffsetVector;
	DesiredLocalPosition = GetMesh()->GetComponentTransform().InverseTransformPosition(TargetLocationWorldSpace);

	if (SmoothedLocalTargetLocation.IsNearlyZero())
		SmoothedLocalTargetLocation = GetAimTransform().InverseTransformPosition(GetCombatSocketLocation());

	SmoothedLocalTargetLocation = FMath::VInterpConstantTo(
		SmoothedLocalTargetLocation,
		DesiredLocalPosition,
		DeltaSeconds,
		RotationSpeedDegrees
	);

	CurrentTargetLocation = SmoothedLocalTargetLocation;

#if WithEditor
	if (bDrawDebug)
	{
		DrawDebugSphere(
			GetWorld(),
			GetMesh()->GetComponentTransform().TransformPosition(CurrentTargetLocation) - TargetOffsetVector,
			25.f,
			12,
			FColor::Red,
			false,
			-1.f,
			0,
			1.0f
		);
	}
#endif
}

/*
 * Checks if the Barrel aligns with the CurrentTarget´s location
 * Starts the shooting process if true
 */
void ASDTurret::CheckIfTargetAligned()
{
	const FVector MuzzleLocation = GetCombatSocketLocation();
	const FVector MuzzleForward = GetAimTransform().GetRotation().GetForwardVector();
	const FVector TargetLocation = CurrentTarget->GetActorLocation() + BarrelOffsetVector;

	const FVector DirectionToTarget = (TargetLocation - MuzzleLocation).GetSafeNormal();

	float Dot = FVector::DotProduct(MuzzleForward, DirectionToTarget);

	Dot = FMath::Clamp(Dot, -1.0f, 1.0f);

	const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(Dot));

#if WITH_EDITOR
	if (bDrawDebug)
	{
		GEngine->AddOnScreenDebugMessage(0, 0.f, FColor::Yellow, FString::Printf(TEXT("Aim Error: %.1f°"), AngleDeg));

		DrawDebugLine(GetWorld(), MuzzleLocation, MuzzleLocation + MuzzleForward * 200.f, FColor::Blue);
		DrawDebugLine(GetWorld(), MuzzleLocation, MuzzleLocation + DirectionToTarget * 200.f, FColor::Red);
	}
#endif
	
	bIsAligned = AngleDeg <= AimErrorTolerance;

	if (bIsAligned != bLastTargetingStatus)
	{
		bLastTargetingStatus = bIsAligned;

		if(bIsAligned)
			OnTargetAligned();
		else
			OnTargetLost();
	}
}

void ASDTurret::OnTargetAligned()
{
	if (bHasStartedTargeting)
		return;

	bHasStartedTargeting = true;
	ApplyEffectToSelf(TurretShootingGameplayEffect, PlayerLevel);
	StartTargeting();

#if WITH_EDITOR
	if (bDrawDebug)
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString(TEXT("Aligned with Target")));
#endif
}

void ASDTurret::OnTargetLost()
{
	bHasStartedTargeting = false;
	RemoveGameplayEffectByTag(TurretShootingTags);

#if WITH_EDITOR
	if(bDrawDebug)
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString(TEXT("Target was lost")));
#endif
}
#pragma endregion

/// <summary>
/// Restarts the targeting process after an Ability has ended.
/// Relies on the Abilities Cooldown, may change in future builds
/// </summary>
void ASDTurret::OnAbilityEnded(const FAbilityEndedData& EndedData)
{
	if (!EndedData.AbilityThatEnded)
		return;

	if (EndedData.AbilityThatEnded->GetAssetTags().HasAllExact(TurretShootingTags))
	{
#if WITH_EDITOR
		if(bDrawDebug)
			GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, FString::Printf(TEXT("%s Ability ended on %s"), *GetNameSafe(EndedData.AbilityThatEnded), *GetNameSafe(this)));
#endif
		
		StartTargeting();
	}
}

void ASDTurret::SetNewStartingLocation(const FRotator& NewRotation)
{
	const FRotator OnlyYawRotation = FRotator(0, NewRotation.Yaw, 0.f);

	const FVector ConvertedOffset = FVector(TargetOffsetVector.Y, 0.f, 0.f);

	const FVector RotatedOffset = OnlyYawRotation.RotateVector(ConvertedOffset);
	const FVector NewStartingLocation = GetMesh()->GetComponentLocation() + RotatedOffset;

	StartingLocation = NewStartingLocation - FVector(0.f, 0.f, TargetOffsetVector.Z);

	bIsRotatingBackToStart = true;
	SetActorTickEnabled(true);
}
