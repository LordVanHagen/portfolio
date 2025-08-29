// Fill out your copyright notice in the Description page of Project Settings.


#include "GridActor.h"
#include "..//SpoopyGameTypes.h"

#pragma region HeapIndex Interface implementation

AGridActor::AGridActor()
{
}

#pragma endregion


void AGridActor::BeginPlay()
{
	Super::BeginPlay();
}

void AGridActor::SetRoomSpaceNeeded(FVector InStartingLocation, FRotator InStartingRotation)
{
	RoomSpaceNeeded.StartingLocation = InStartingLocation;
	UGridNode* NewGridNode = NewObject<UGridNode>();
	NewGridNode->SetPositionInGrid(InStartingLocation);
	RoomSpaceNeeded.CoordinatesInGrid.Add(NewGridNode);
	SetSpawnRotation(InStartingRotation);
	CalculateFloorsBySpawnLocation(InStartingLocation);
	if (RoomType == ERoomType::CellPiece)
		return;
	if (RoomSize == ERoomSize::OneByOneByOne)
		return;
	CalculateRoomSpaceNeeded(RoomSpaceNeeded.StartingLocation);
}

void AGridActor::SetSpawnRotation(FRotator InStartingRotation)
{
	RoomSpaceNeeded.StartingRotation = InStartingRotation;

	if (InStartingRotation == FRotator(0.f, 0.f, 0.f))
	{
		RoomSpaceNeeded.SpawnRotation = ESpawnRotation::NORTH;
	}
	else if (InStartingRotation == FRotator(0.f, 90.f, 0.f))
	{
		RoomSpaceNeeded.SpawnRotation = ESpawnRotation::EAST;
	}
	else if (InStartingRotation == FRotator(0.f, 180.f, 0.f))
	{
		RoomSpaceNeeded.SpawnRotation = ESpawnRotation::SOUTH;
	}
	else if (InStartingRotation == FRotator(0.f, 270.f, 0.f))
	{
		RoomSpaceNeeded.SpawnRotation = ESpawnRotation::WEST;
	}
}

void AGridActor::CalculateFloorsBySpawnLocation(FVector InSpawnLocation)
{
	int32 FirstFloor = FMath::RoundToInt(InSpawnLocation.Z / StandardRoomSize.Z);
	RoomSpaceNeeded.Floors.Add(FirstFloor);

	if (RoomSize == ERoomSize::OneByOneByTwo || RoomSize == ERoomSize::TwoByTwoByTwo || RoomSize == ERoomSize::TwoByThreeByTwo)
	{
		int32 SecondFloor = FMath::RoundToInt(InSpawnLocation.Z / StandardRoomSize.Z - 1);
		RoomSpaceNeeded.Floors.Add(SecondFloor);
	}
}

ERoomType AGridActor::GetRoomType()
{
	return RoomType;
}

FRoomSpaceNeeded AGridActor::GetRoomSpaceNeeded()
{
	return RoomSpaceNeeded;
}

bool AGridActor::GetRoomCountsAsStairs()
{
	return RoomCountsAsStairs;
}

void AGridActor::EmptyRoomSpaceNeeded()
{
	RoomSpaceNeeded.CoordinatesInGrid.Reset();
	RoomSpaceNeeded.Floors.Reset();
}

void AGridActor::LogAllRoomSpacesNeeded()
{
	for (const UGridNode* RoomSpace : RoomSpaceNeeded.CoordinatesInGrid)
	{
		UE_LOG(LogTemp, Display, TEXT("RoomSpaceNeeded, X: %f, Y: %f, Z: %f"), RoomSpace->GetPositionInGrid().X, RoomSpace->GetPositionInGrid().Y, RoomSpace->GetPositionInGrid().Z);
	}

	switch (RoomSpaceNeeded.SpawnRotation)
	{
	case ESpawnRotation::NORTH:
		UE_LOG(LogTemp, Warning, TEXT("Spawn Rotation: NORTH"));
		break;
	case ESpawnRotation::SOUTH:
		UE_LOG(LogTemp, Warning, TEXT("Spawn Rotation: SOUTH"));
		break;
	case ESpawnRotation::EAST:
		UE_LOG(LogTemp, Warning, TEXT("Spawn Rotation: EAST"));
		break;
	case ESpawnRotation::WEST:
		UE_LOG(LogTemp, Warning, TEXT("Spawn Rotation: WEST"));
		break;
	}

}

void AGridActor::LogAllFloors()
{
	for (const int32& Floor : RoomSpaceNeeded.Floors)
	{
		UE_LOG(LogTemp, Warning, TEXT("Floor beeing used by %s: %d"), *GetName(), Floor);
	}
}

UGridNode* AGridActor::GetStartingGridNode()
{
	UGridNode* ReturningNode = nullptr;

	for (UGridNode* Node : RoomSpaceNeeded.CoordinatesInGrid)
	{
		if (Node->GetPositionInGrid() == RoomSpaceNeeded.StartingLocation)
		{
			ReturningNode = Node;
			break;
		}
	}

	if (ReturningNode == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("No starting node found."));
	}

	return ReturningNode;
}

void AGridActor::CalculateRoomSpaceNeeded(FVector InStartingLocation)
{
	///// <<< 2X2X1 ROOM >>> /////
	if (RoomSize == ERoomSize::TwoByTwoByOne)
	{
		///// <<< NORTH >>> /////
		if (RoomSpaceNeeded.SpawnRotation == ESpawnRotation::NORTH)
		{
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, StandardRoomSize.Y, 0.f));
		}
		///// <<< SOUTH >>> /////
		else if (RoomSpaceNeeded.SpawnRotation == ESpawnRotation::SOUTH)
		{
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, -StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, -StandardRoomSize.Y, 0.f));
		}
		///// <<< EAST >>> /////
		else if (RoomSpaceNeeded.SpawnRotation == ESpawnRotation::EAST)
		{
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, StandardRoomSize.Y, 0.f));
		}
		///// <<< WEST >>> /////
		else if (RoomSpaceNeeded.SpawnRotation == ESpawnRotation::WEST)
		{
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, -StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, -StandardRoomSize.Y, 0.f));
		}
	}
	///// <<< 2X3X1 ROOM >>> /////
	else if (RoomSize == ERoomSize::TwoByThreeByOne)
	{
		///// <<< NORTH >>> /////
		if (RoomSpaceNeeded.SpawnRotation == ESpawnRotation::NORTH)
		{
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector((StandardRoomSize.X * 2), 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector((StandardRoomSize.X * 2), StandardRoomSize.Y, 0.f));
		}
		///// <<< SOUTH >>> /////
		else if (RoomSpaceNeeded.SpawnRotation == ESpawnRotation::SOUTH)
		{
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, -StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, -StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(-(StandardRoomSize.X * 2), 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(-(StandardRoomSize.X * 2), -StandardRoomSize.Y, 0.f));
		}
		///// <<< EAST >>> /////
		else if (RoomSpaceNeeded.SpawnRotation == ESpawnRotation::EAST)
		{
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, (StandardRoomSize.Y * 2), 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, (StandardRoomSize.Y * 2), 0.f));
		}
		///// <<< WEST >>> /////
		else if (RoomSpaceNeeded.SpawnRotation == ESpawnRotation::WEST)
		{
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, -StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, -StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, -(StandardRoomSize.Y * 2), 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, -(StandardRoomSize.Y * 2), 0.f));
		}
	}
	///// <<< 1X1X2 ROOM >>> /////
	else if (RoomSize == ERoomSize::OneByOneByTwo)
	{
		AddCoordinatesInGrid(InStartingLocation + FVector(0.f, 0.f, -StandardRoomSize.Z));
	}
	///// <<< 2X2X2 ROOM >>> /////
	else if (RoomSize == ERoomSize::TwoByTwoByTwo)
	{
		///// <<< NORTH >>> /////
		if (RoomSpaceNeeded.SpawnRotation == ESpawnRotation::NORTH)
		{
			///// <<< Z = 0 >>> /////
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, StandardRoomSize.Y, 0.f));
			///// <<< Z = -1 >>> /////
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, 0.f, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, StandardRoomSize.Y, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, 0.f, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, StandardRoomSize.Y, -StandardRoomSize.Z));
		}
		///// <<< SOUTH >>> /////
		else if (RoomSpaceNeeded.SpawnRotation == ESpawnRotation::SOUTH)
		{
			///// <<< Z = 0 >>> /////
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, -StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, -StandardRoomSize.Y, 0.f));
			///// <<< Z = -1 >>> /////
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, 0.f, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, -StandardRoomSize.Y, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, 0.f, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, -StandardRoomSize.Y, -StandardRoomSize.Z));
		}
		///// <<< EAST >>> /////
		else if (RoomSpaceNeeded.SpawnRotation == ESpawnRotation::EAST)
		{
			///// <<< Z = 0 >>> /////
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, StandardRoomSize.Y, 0.f));
			///// <<< Z = -1 >>> /////
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, 0.f, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, 0.f, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, StandardRoomSize.Y, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, StandardRoomSize.Y, -StandardRoomSize.Z));
		}
		///// <<< WEST >>> /////
		else if (RoomSpaceNeeded.SpawnRotation == ESpawnRotation::WEST)
		{
			///// <<< Z = 0 >>> /////
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, -StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, -StandardRoomSize.Y, 0.f));
			///// <<< Z = -1 >>> /////
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, 0.f, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, 0.f, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, -StandardRoomSize.Y, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, -StandardRoomSize.Y, -StandardRoomSize.Z));
		}
	}
	///// <<< 2X3X2 ROOM >>> /////
	else if (RoomSize == ERoomSize::TwoByThreeByTwo)
	{
		///// <<< NORTH >>> /////
		if (RoomSpaceNeeded.SpawnRotation == ESpawnRotation::NORTH)
		{
			///// <<< Z = 0 >>> /////
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector((StandardRoomSize.X * 2), 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector((StandardRoomSize.X * 2), StandardRoomSize.Y, 0.f));
			///// <<< Z = -1 >>> /////
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, 0.f, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, StandardRoomSize.Y, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, 0.f, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, StandardRoomSize.Y, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector((StandardRoomSize.X * 2), 0.f, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector((StandardRoomSize.X * 2), StandardRoomSize.Y, -StandardRoomSize.Z));
		}
		///// <<< SOUTH >>> /////
		else if (RoomSpaceNeeded.SpawnRotation == ESpawnRotation::SOUTH)
		{
			///// <<< Z = 0 >>> /////
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, -StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, -StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(-(StandardRoomSize.X * 2), 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(-(StandardRoomSize.X * 2), -StandardRoomSize.Y, 0.f));
			///// <<< Z = -1 >>> /////
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, 0.f, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, -StandardRoomSize.Y, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, 0.f, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, -StandardRoomSize.Y, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(-(StandardRoomSize.X * 2), 0.f, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(-(StandardRoomSize.X * 2), -StandardRoomSize.Y, -StandardRoomSize.Z));
		}
		///// <<< EAST >>> /////
		else if (RoomSpaceNeeded.SpawnRotation == ESpawnRotation::EAST)
		{
			///// <<< Z = 0 >>> /////
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, (StandardRoomSize.Y * 2), 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, (StandardRoomSize.Y * 2), 0.f));
			///// <<< Z = -1 >>> /////
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, 0.f, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, 0.f, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, StandardRoomSize.Y, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, StandardRoomSize.Y, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, (StandardRoomSize.Y * 2), -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(-StandardRoomSize.X, (StandardRoomSize.Y * 2), -StandardRoomSize.Z));
		}
		///// <<< WEST >>> /////
		else if (RoomSpaceNeeded.SpawnRotation == ESpawnRotation::WEST)
		{
			///// <<< Z = 0 >>> /////
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, 0.f, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, -StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, -StandardRoomSize.Y, 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, -(StandardRoomSize.Y * 2), 0.f));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, -(StandardRoomSize.Y * 2), 0.f));
			///// <<< Z = -1 >>> /////
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, 0.f, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, 0.f, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, -StandardRoomSize.Y, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, -StandardRoomSize.Y, -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(0.f, -(StandardRoomSize.Y * 2), -StandardRoomSize.Z));
			AddCoordinatesInGrid(InStartingLocation + FVector(StandardRoomSize.X, -(StandardRoomSize.Y * 2), -StandardRoomSize.Z));
		}
	}
}

void AGridActor::AddCoordinatesInGrid(FVector InPosition)
{
	UGridNode* NodeToAdd = NewObject<UGridNode>();
	NodeToAdd->SetPositionInGrid(InPosition);
	RoomSpaceNeeded.CoordinatesInGrid.Add(NodeToAdd);
}

void UGridNode::SetPositionInGrid(FVector InPosition)
{
	PositionInGrid = InPosition;
}

FVector UGridNode::GetPositionInGrid() const
{
	return PositionInGrid;
}

void UGridNode::SetDoor(UDoorPosition* InDoor)
{
	Door = InDoor;
}

void UGridNode::SetGCost(int32 InGCost)
{
	GCost = InGCost;
}

void UGridNode::SetHCost(int32 InHCost)
{
	HCost = InHCost;
}

void UGridNode::SetParent(UGridNode* InParent)
{
	Parent = InParent;
}

void UGridNode::SetHeapIndex(int32 InHeapIndex)
{
	HeapIndex = InHeapIndex;
}

UDoorPosition* UGridNode::GetDoor()
{
	return Door;
}

int32 UGridNode::GetGCost() const
{
	return GCost;
}

int32 UGridNode::GetHCost() const
{
	return HCost;
}

UGridNode* UGridNode::GetParent()
{
	return Parent;
}

int32 UGridNode::GetHeapIndex()
{
	return HeapIndex;
}

int32 UGridNode::GetFCost() const
{
	return GCost + HCost;
}
