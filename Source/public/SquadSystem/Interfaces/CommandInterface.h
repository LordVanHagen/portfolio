// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CommandInterface.generated.h"

struct FComputedLocationInfo;
class USquadMember;

USTRUCT(Blueprintable, BlueprintType)
struct FPathPointData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route Data")
	FVector PointLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route Data")
	bool bPointReached = false;

	FORCEINLINE bool operator==(const FPathPointData& Other) const
	{
		return PointLocation.Equals(Other.PointLocation, KINDA_SMALL_NUMBER);
	}
	FORCEINLINE bool operator!=(const FPathPointData& Other) const
	{
		return !(*this == Other);
	}
};

enum ESquadFlags
{
	IsInitialized = 1 << 0,
	HasMoved = 1 << 1,
	HasReachedDestination = 1 << 2,
};

USTRUCT()
struct FSquadMemberPositionInfo
{
	GENERATED_BODY()
	float MoveSpeed = 300.f;
	float TargetAcceptanceRadius = 50.f;
	FVector CurrentPosition = FVector::ZeroVector;
	FRotator CurrentRotation = FRotator::ZeroRotator;
	int32 SquadID = -1;
	uint8 Flags = 0;
	uint8 Reserved1 = 0;
	uint8 Reserved2 = 0;
	uint8 Reserved3 = 0;

	FORCEINLINE bool IsInitialized() const
	{
		return (Flags& ESquadFlags::IsInitialized) != 0;
	}
	FORCEINLINE void SetIsInitialized(bool b)
	{
		if (b)
			Flags |= ESquadFlags::IsInitialized;
		else
			Flags &= ~ESquadFlags::IsInitialized;
	}

	FORCEINLINE bool operator==(const FSquadMemberPositionInfo& Other) const
	{
		return CurrentPosition.Equals(Other.CurrentPosition, KINDA_SMALL_NUMBER);
	}
	FORCEINLINE bool operator!=(const FSquadMemberPositionInfo& Other) const
	{
		return !(*this == Other);
	}
};

USTRUCT(BlueprintType)
struct FSquadCommandPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad Command")
	AActor* TargetActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad Command")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad Command")
	FVector OffsetToTarget = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad Command")
	TArray<FPathPointData> PathPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad Command")
	bool bIsSet = false;
};

UENUM()
enum EVisibilitySetting : uint8
{
	SkeletalMeshSetting,
	StaticMeshSetting,
	NiagaraSystemSetting
};


UINTERFACE()
class UCommandableInterface : public UInterface
{
	GENERATED_BODY()
};

class ICommandableInterface
{
	GENERATED_BODY()
public:
	virtual void GoTo(const FSquadCommandPayload& SquadCommandPayload) = 0;
	virtual void Attack(const FSquadCommandPayload& SquadCommandPayload) = 0;
	virtual void Swarm(const FSquadCommandPayload& SquadCommandPayload) = 0;
	virtual void Flee(const FSquadCommandPayload& SquadCommandPayload) = 0;

	// Possible commands for the future:
	virtual void FollowPath() = 0;
	virtual void JumpDown() = 0;
	
	virtual void SetMemberWrapper(USquadMember* InMemberWrapper) = 0;
	virtual void InitStats() = 0;
	virtual const FSquadMemberPositionInfo* GetPositionInfo(const int32& InSquadID) = 0;
	virtual void Move(const FComputedLocationInfo& ComputedLocation) = 0;
	virtual void SetVisibilitySetting(const EVisibilitySetting InVisibility) = 0;
};

UINTERFACE(MinimalAPI)
class USquadCommandInterface : public UInterface
{
	GENERATED_BODY()
};

class ISquadCommandInterface
{
	GENERATED_BODY()
public:
	virtual void Execute(ICommandableInterface* Commandable, const FSquadCommandPayload& SquadCommandPayload) = 0;
};
