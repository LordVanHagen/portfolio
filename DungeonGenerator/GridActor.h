// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../SpoopyGameTypes.h"
#include "GridActor.generated.h"

class UDoorPosition;

UENUM()
enum class ERoomType : uint8
{
	CellPiece UMETA(DisplayName = "Cell Piece"),
	Room UMETA(DisplayName = "Room"),
	Stairs UMETA(DisplayName = "Stairs"),
	Corridor UMETA(DisplayName = "Corridor")
};

UENUM()
enum class ERoomSize : uint8
{
	OneByOneByOne UMETA(DisplayName = "1X, 1Y, 1Z"),
	TwoByTwoByOne UMETA(DisplayName = "2X, 2Y, 1Z"),
	TwoByThreeByOne UMETA(DisplayName = "2X, 3Y, 1Z"),
	OneByOneByTwo UMETA(DisplayName = "1X, 1Y, 2Z"),
	TwoByTwoByTwo UMETA(DisplayName = "2X, 2Y, 2Z"),
	TwoByThreeByTwo UMETA(DisplayName = "2X, 3Y, 2Z")
};

UCLASS(Blueprintable)
class SPOOPYGAME_API UGridNode : public UObject
{
	GENERATED_BODY()

	UPROPERTY()
	int32 HeapIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node Information", meta = (AllowPrivateAccess = "true"))
	FVector PositionInGrid;

	UPROPERTY()
	int32 GridX;

	UPROPERTY()
	int32 GridY;

	UPROPERTY()
	AGridActor* Owner;

	UPROPERTY()
	UGridNode* Parent;

	UPROPERTY()
	int32 GCost = 0;

	UPROPERTY()
	int32 HCost = 0;

	UPROPERTY()
	UDoorPosition* Door;

public:

	UGridNode()
	{}

	UGridNode(FVector InPositionInGrid)
		: PositionInGrid(InPositionInGrid)
	{}

	int32 CompareTo(const UGridNode& Other) const
	{
		int compare;
		if (GetFCost() > Other.GetFCost())
			compare = -1;
		else if (GetFCost() < Other.GetFCost())
			compare = 1;
		else
			compare = 0;

		if (compare == 0)
		{
			if (HCost > Other.HCost)
				compare = -1;
			else if (HCost < Other.HCost)
				compare = 1;
			else
				compare = 0;
		}

		return -compare;
	}

	friend bool operator==(const UGridNode& Left, const UGridNode& Right)
	{
		return Left.PositionInGrid == Right.PositionInGrid;
	}

	friend bool operator!=(const UGridNode& Left, const UGridNode& Right)
	{
		return !(Left == Right);
	}

	bool Equals(const UGridNode& Other)
	{
		return *this == Other;
	}


	// Setter
	void SetPositionInGrid(FVector InPosition);
	void SetDoor(UDoorPosition* InDoor);
	void SetGCost(int32 InGCost);
	void SetHCost(int32 InHCost);
	void SetParent(UGridNode* InParent);
	void SetHeapIndex(int32 InHeapIndex);

	// Getter
	FVector GetPositionInGrid() const;
	UDoorPosition* GetDoor();
	int32 GetGCost() const;
	int32 GetHCost() const;
	UGridNode* GetParent();
	int32 GetHeapIndex();
	int32 GetFCost() const;
};


USTRUCT(BlueprintType)
struct FRoomSpaceNeeded
{
	GENERATED_USTRUCT_BODY();

	FVector StartingLocation = FVector::ZeroVector;

	FRotator StartingRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room Data", meta = (AllowPrivateAccess = "true"))
	TArray<UGridNode*> CoordinatesInGrid;
	
	UPROPERTY(BlueprintReadOnly)
	ESpawnRotation SpawnRotation;

	UPROPERTY(BlueprintReadOnly)
	TArray<int32> Floors;
};

UCLASS()
class SPOOPYGAME_API AGridActor : public AActor
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room Information", meta = (AllowPrivateAccess = "true"))
	ERoomType RoomType = ERoomType::CellPiece;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room Information", meta = (AllowPrivateAccess = "true"))
	ERoomSize RoomSize = ERoomSize::OneByOneByOne;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room Information", meta = (AllowPrivateAccess = "true"))
	FVector StandardRoomSize = FVector(100.f, 100.f, 50.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room Information", meta = (AllowPrivateAccess = "true"))
	bool RoomCountsAsStairs = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room Information", meta = (AllowPrivateAccess = "true"))
	FRoomSpaceNeeded RoomSpaceNeeded;

public:
	AGridActor();

	void CalculateFloorsBySpawnLocation(FVector InSpawnLocation);

	void SetRoomSpaceNeeded(FVector InStartingLocation, FRotator InStartingRotation);

	void SetSpawnRotation(FRotator InStartingRotation);

	ERoomType GetRoomType();

	FRoomSpaceNeeded GetRoomSpaceNeeded();

	bool GetRoomCountsAsStairs();

	void EmptyRoomSpaceNeeded();

	void LogAllRoomSpacesNeeded();

	void LogAllFloors();

	UGridNode* GetStartingGridNode();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void CalculateRoomSpaceNeeded(FVector InStartingLocation);

private:

	void AddCoordinatesInGrid(FVector InPosition);

};