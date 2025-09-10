// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/SDAICharacter.h"
#include "Character/AI/TurretBehaviour.h"
#include "SDTypes.h"
#include "SDTurret.generated.h"

class UBlackboardComponent;
class UNiagaraSystem;
class USDShootAbility;
class USDGameplayAbility;
class UGameplayEffect;
struct FAbilityEndedData;

/**
 * 
 */
UCLASS()
class SUPERDEFENCE_API ASDTurret : public ASDAICharacter, public ITurretBehaviour
{
	GENERATED_BODY()
public:
	ASDTurret();
	virtual void GetActorEyesViewPoint(FVector& Location, FRotator& Rotation) const override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	virtual void Character_TurretSetCombatState(bool bInCombat) override;

	/* Combat Interface */
	virtual FTransform GetAimTransform() const override;
	virtual AActor* Turret_GetCurrentTarget_Implementation() const override;
	/* Combat Interface */

protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent)
	void StartCombatState();
	void StartIdleState();
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void StartTargeting();
	UFUNCTION(BlueprintCallable)
	void HandleShooting();

	/// <summary>
	/// Intended to be called via Animation Notify, therefore an Authoritycheck needs to be implemented before Shooting
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void TurretFire();
	/// <summary>
	/// Intended to be called via Animation Notify, therefore an Authoritycheck needs to be implemented
	/// </summary>
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void TurretFireEnd();

	void DestroyTurret();

	/* BP Relevant Information */
	UPROPERTY(EditAnywhere, Category = "SDCharacter|NEEDS TO BE SET", meta = (ToolTip = "The Socket used to determin the Characters Eye-Position"))
	FName EyeSocketName;

	UPROPERTY(EditAnywhere, Category = "SDCharacter|NEEDS TO BE SET", meta = (ToolTip = "Needs to be exactly named after the Blackboards Value!"))
	FName TurretStateBlackboardValueName;

	UPROPERTY(EditAnywhere, Category = "SDCharacter|Turret|Data")
	float RotationSpeedDegrees = 90.f;

	UPROPERTY(EditAnywhere, Category = "SDCharacter|Turret|Data")
	float AimErrorTolerance = 2.f;

	UPROPERTY(EditAnywhere, Category = "SDCharacter|Turret|Data")
	float FireRate = 1.f;

	UPROPERTY(EditAnywhere, Category = "SDCharacter|Turret|Effects")
	TObjectPtr<UNiagaraSystem> MuzzleFlash;

	UPROPERTY(EditAnywhere, Category = "SDCharacter|NEEDS TO BE SET", meta = (ToolTip = "The Socket used to determin the position of the Muzzle-Flash"))
	FName MuzzleFlashAttachmentPointName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SDCharacter|Turret|Targeting")
	float TurretAfterAwakeningTimer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SDCharacter|Turret|Targeting")
	float TurretAfterTargetingTimer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SDCharacter|Turret|Targeting")
	float TurretAfterFiringTimer;

	// Set in Blueprints BeginPlay
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UBlackboardComponent> Blackboard;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ETurretState CurrentTurretState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SDCharacter|Turret|Target")
	AActor* CurrentTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SDCharacter|Turret|Target")
	AActor* LastProcessedTarget;

	ICombatInterface* CachedCombatInterface = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SDCharacter|Turret|AnimBP", Replicated)
	FVector CurrentTargetLocation;

	// USED LATER FOR PREDICTION SENSE
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SDCharacter|Turret|AnimBP", Replicated)
	float CurrentTargetDistance;
	// USED LATER FOR PREDICTION SENSE

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SDCharacter|Turret|AnimBP", Replicated)
	bool bIsAiming = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SDCharacter|Turret|Target|Flags", Replicated)
	bool bIsTargetting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SDCharacter|Turret|Target|Flags", Replicated)
	bool bIsRotatingBackToStart = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SDCharacter|Turret|Target|Flags")
	bool bIsAligned = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SDCharacter|Turret|Target|Flags")
	bool bWasTargetValidLastTick;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SDCharacter|Turret|Offsets")
	FVector TargetOffsetVector = FVector(0.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SDCharacter|Turret|Offsets")
	FVector BarrelOffsetVector = FVector(0.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDCharacter|Turret|Offsets")
	FVector StartingLocation = FVector(0.f, 0.f, 0.f);
	/* BP Relevant Information */

	/* Interpolation */
	FVector TargetLocationWorldSpace;
	FVector DesiredLocalPosition;
	FVector SmoothedLocalTargetLocation;
	/* Interpolation */

	/* Tick Function */
	void InterpolateToPosition(float DeltaSeconds, const FVector& InPosition);
	void CheckIfTargetAligned();
	UPROPERTY(VisibleAnywhere, Category = "SDCharacter|Turret|Target")
	bool bHasStartedTargeting = false;
	UPROPERTY(VisibleAnywhere, Category = "SDCharacter|Turret|Target")
	bool bLastTargetingStatus = false;
	void OnTargetAligned();
	void OnTargetLost();
	/* Tick Function */

	/* GAS */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SDCharacter|Turret|Data");
	FGameplayTagContainer TurretShootingTags;

	UPROPERTY(EditAnywhere, Category = "SDCharacter|Turret|Data");
	TSubclassOf<UGameplayEffect> TurretShootingGameplayEffect;

	virtual void OnAbilityEnded(const FAbilityEndedData& EndedData);
	/* GAS */
	////////////// Getter and Setter //////////////
 public:
	 UFUNCTION(BlueprintCallable)
	 FORCEINLINE AActor* GetCurrentTarget() { return CurrentTarget; }
	 UFUNCTION(BlueprintCallable)
	 FORCEINLINE void SetCurrentTarget(AActor* NewCurrentTarget) { CurrentTarget = NewCurrentTarget; }
	 FORCEINLINE bool GetDrawDebug() { return bDrawDebug; }
	 void SetNewStartingLocation(const FRotator& NewRotation);
};