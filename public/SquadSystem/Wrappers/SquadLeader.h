// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Squads/MovementHelper.h"
#include "Squads/Interfaces/LeaderInterface.h"
#include "SquadLeader.generated.h"

class USquadEnemySettings;
struct FPathPointData;
struct FSquadCommandPayload;
class IMemberInterface;
/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class SUPERDEFENCE_API USquadLeader : public UObject, public ILeaderInterface, public FTickableGameObject
{
	GENERATED_BODY()
public:
	USquadLeader();
	virtual void BeginDestroy() override;
	virtual void Init(AActor* Outer);
	
	/* Start Leader Interface */
	virtual void AddMember(IMemberInterface* NewMember) override;
	virtual void RemoveMember(IMemberInterface* MemberToRemove) override;
	virtual TArray<IMemberInterface*> GetMembers() override;
	virtual void Join(ILeaderInterface* Other) override;
	virtual void Split() override;
	UFUNCTION(BlueprintCallable, Category = "Squad")
	virtual void ExecuteCommand(USquadCommand* Command, const FSquadCommandPayload& SquadCommandPayload) override;

	UFUNCTION(BlueprintCallable, Category = "Squad")
	virtual void NotifySquadReady() override;

	UFUNCTION(BlueprintCallable, Category = "Squad")
	virtual void Blueprint_AddMember(UObject* NewMember);
	virtual void TerminateSquad() override;
	/* End Leader Interface */

	/* Start TickableGameObject Interface */
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return bIsTickEnabled; }
	/* Start TickableGameObject Interface */
	
	UObject* Get() const { return Leader; }

private:
	UPROPERTY()
	AActor* Leader = nullptr;
	TMap<int32, IMemberInterface*> MembersBySquadID;
	//UPROPERTY(EditAnywhere, Category = "SquadManagement")
	//int32 SquadSplitAmount = 50;
	int32 GetNewSquadID();
	int32 NextSquadID = 1;
	UPROPERTY()
	USquadCommand* CurrentCommand = nullptr;
	// Maybe have a Command Catalog to check which commands are available with their transitions

	bool bIsTickEnabled = false;
	
	FMovementHelper MovementHelper;
	
	/// Squad Movement Handling ///
	virtual void StartSquadMovement();
	void TickSquad(float DeltaTime);
	bool bMoveSquad = false;
	/// Squad Movement Handling ///

	float CurrentTickInterval = 0.016f;
	float TimeSinceLastTick = 0.0f;
	void EnableRangeCheckTimer(const bool bEnable);
	void DoRangeCheck();
	void UpdateTickInterval(float DistanceToClosestPlayer);
	void UpdateSquadVisibilitySetting(const EVisibilitySetting InVisibilitySetting);
	FTimerHandle LeaderTimer;

	static const USquadEnemySettings* GetSettings();

	void Reset();
};