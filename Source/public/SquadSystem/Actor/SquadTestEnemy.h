// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/CombatInterface.h"
#include "Squads/SquadDamageHelper.h"
#include "Squads/Interfaces/CommandInterface.h"
#include "SquadTestEnemy.generated.h"

class UAISense;
class UAIPerceptionStimuliSourceComponent;
class UCapsuleComponent;

UCLASS(BLueprintable, BlueprintType)
class SUPERDEFENCE_API ASquadTestEnemy : public AActor, public ICommandableInterface, public ICombatInterface
{
	GENERATED_BODY()
	
public:	
	ASquadTestEnemy();
	virtual void BeginPlay() override;	
	
	/// Start Commandable Interface ///
	virtual void GoTo(const FSquadCommandPayload& SquadCommandPayload) override;
	virtual void Attack(const FSquadCommandPayload& SquadCommandPayload) override;
	virtual void Swarm(const FSquadCommandPayload& SquadCommandPayload) override;
	virtual void Flee(const FSquadCommandPayload& SquadCommandPayload) override;

	virtual void FollowPath() override;
	virtual void JumpDown() override;
	
	virtual void SetMemberWrapper(USquadMember* InMemberWrapper) override;
	virtual void InitStats() override;
	virtual const FSquadMemberPositionInfo* GetPositionInfo(const int32& InSquadID) override;
	virtual void Move(const FComputedLocationInfo& ComputedLocation) override;
	virtual void SetVisibilitySetting(const EVisibilitySetting InVisibility) override;
	/// End Commandable Interface ///

	/// Start Combat Interface ///
	virtual void Die() override;
	virtual bool IsDead() override;
	virtual void ReceiveDamage(const FGameplayEffectSpecHandle& DamageEffectSpecHandle) override;
	virtual FVector GetCombatSocketLocation() override;
	virtual FVector GetTargetVelocity() const override;
	/// End Combat Interface //
	
	virtual void PreparingToStartCommand(const bool bShouldMove, const bool bUseTick);
	virtual void HasFinishedCommand();

private:
	virtual void Tick(float DeltaTime) override;

	virtual void SetAndValidateSquadCommandData(const FSquadCommandPayload& SquadCommandPayload);

	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_HandleDeath();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	TObjectPtr<UCapsuleComponent> CapsuleComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Squad")
	FGameplayTag SquadEnemyTypeTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Squad")
	FName AimSocketName = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	bool bIsOnTheMove = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Squad")
	float ArrivalTolerance = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Squad")
    float TickInterval = 0.016;
	
	FSquadCommandPayload CurrentSquadCommandData;

	UPROPERTY()
	USquadMember* MemberWrapper = nullptr;

	/* PERCEPTION for the TurretAI */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad|Perception")
	UAIPerceptionStimuliSourceComponent* StimuliSourceComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Squad|Perception")
	TArray<TSubclassOf<UAISense>> StartingSenses;
	TArray<TSubclassOf<UAISense>> RegisteredSenses;

	void RegisterStartingSenses();
	void RegisterStimulus(const TSubclassOf<UAISense>& SenseToRegister);
	void UnregisterStimulus(const TSubclassOf<UAISense>& SenseToRegister);
	void UnregisterAllStimuli();
	/* PERCEPTION for the TurretAI */
	
private:
	void MoveToLocation(const float InDeltaTime);
	FVector GetNextValidTargetPoint();

	FVector CurrentTargetLocation = FVector::ZeroVector;
	FPathPointData* CurrentPath = nullptr;
	FVector Velocity = FVector::ZeroVector;
	
	FEnemyStats Stats;
	bool bIsDead = false;

	FSquadMemberPositionInfo PositionInfo;
};