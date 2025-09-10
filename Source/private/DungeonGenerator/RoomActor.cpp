#include "RoomActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "GridActor.h"
#include "DoorPosition.h"

ARoomActor::ARoomActor()
{
	TSet<UActorComponent*> Components = GetComponents();

	AmountOfNonOptionalDoors = 0;

	for (UActorComponent* Component : Components)
	{
		if (UDoorPosition* DoorPosition = Cast<UDoorPosition>(Component))
		{
			DoorPositions.Add(DoorPosition);
		}
	}
}

//void ARoomActor::SpawnRoomsBasedOnDoorPositions()
//{
//	for (UDoorPosition* Component : DoorPositions)
//	{
//		if (Component)
//		{
//			UE_LOG(LogTemp, Warning, TEXT("Doorposition found."));
//			UDoorPosition* DoorPosition = Cast<UDoorPosition>(Component);
//
//			FTransform ReferenceTransform = RootComponent->GetRelativeTransform();
//			//FTransform ReferenceTransform = DoorPosition->GetAttachParent()->GetRelativeTransform();
//			FVector SpawnLocation = ReferenceTransform.GetLocation() + ReferenceTransform.TransformVector(GetLocationBasedOnRotation(DoorPosition)) + CalculateLocationInGridActor(DoorPosition);
//			FRotator SpawnRotation = ReferenceTransform.Rotator();
//
//			if (AActor* NewActor = GetWorld()->SpawnActor<AActor>(ActorToSpawn, SpawnLocation, SpawnRotation))
//			{
//				// Der neue Actor wurde erfolgreich gespawnt
//				UE_LOG(LogTemp, Warning, TEXT("Spawned new actor at location: %s"), *SpawnLocation.ToString());
//			}
//			else
//			{
//				UE_LOG(LogTemp, Warning, TEXT("Failed to spawn new actor!"));
//			}
//		}
//	}
//}

FVector ARoomActor::GetLocationBasedOnRotation(UDoorPosition* InDoorPosition)
{
	FVector LocationToSpawn = FVector(0.f, 0.f, 0.f);

	switch (InDoorPosition->GetDoorData().SpawnRotation)
	{
	case ESpawnRotation::SOUTH:
		LocationToSpawn = FVector(-StandardRoomSize.X, 0.f, 0.f);
		break;
	case ESpawnRotation::EAST:
		LocationToSpawn = FVector(0.f, StandardRoomSize.X, 0.f);
		break;
	case ESpawnRotation::WEST:
		LocationToSpawn = FVector(0.f, -StandardRoomSize.X, 0.f);
		break;
	default:
		LocationToSpawn = FVector(StandardRoomSize.X, 0.f, 0.f);
		break;
	}

	return LocationToSpawn;
}

void ARoomActor::CalculateCenterOfMass_Implementation()
{
	// IMPLEMENTATION FOUND IN BLUEPRINT
}

void ARoomActor::SpawnDoorsInRoom_Implementation()
{
	// IMPLEMENTATION FOUND IN BLUEPRINT
}

FVector ARoomActor::GetNodePositionBasedOnRotation(FVector InUsedGridInRoom, ESpawnRotation InSpawnRotation)
{
	FVector OutPos = FVector(0.0f, 0.0f, 0.0f);
	float PosX = InUsedGridInRoom.X;
	float PosY = InUsedGridInRoom.Y;
	float PosZ = InUsedGridInRoom.Z;
	float Temp;

	switch (InSpawnRotation)
	{
	case ESpawnRotation::SOUTH:
		PosX = -PosX;
		PosY = -PosY;
		break;
	case ESpawnRotation::EAST:
		Temp = PosX;
		PosX = -PosY;
		PosY = Temp;
		break;
	case ESpawnRotation::WEST:
		Temp = PosX;
		PosX = PosY;
		PosY = -Temp;
	default:
		break;
	}

	OutPos = FVector(PosX, PosY, PosZ);

	return OutPos;
}

FVector ARoomActor::ConvertDoorNodePosition(UDoorPosition* InDoorPosition)
{
	FVector UsedGridInRoom = InDoorPosition->GetDoorData().UsedGridInRoom;
	FVector NodePos = GetNodePositionBasedOnRotation(UsedGridInRoom, RoomSpaceNeeded.SpawnRotation);

	FVector OutConvertedDoorNodePosition = NodePos * StandardRoomSize;
	
	return OutConvertedDoorNodePosition + GetActorLocation();
}

int ARoomActor::GetAmountOfNonOptionalDoors()
{
	int DoorAmount = 0;

	for (UDoorPosition* DoorPosition : DoorPositions)
	{
		if (!DoorPosition->GetDoorData().IsOptional)
		{
			DoorAmount++;
		}
	}

	AmountOfNonOptionalDoors = DoorAmount;

	return AmountOfNonOptionalDoors;
}

bool ARoomActor::GetCOMWasGenerated()
{
	return bCOMWasGenerated;
}

void ARoomActor::SetCOMWasGenerated(bool Condition)
{
	bCOMWasGenerated = Condition;
}

void ARoomActor::SetVertexLocation(FVector VertexLocation, int32 Floor)
{
	Vertices.Add(Floor, FVertex(VertexLocation));
}

TMap<int32, FVertex> ARoomActor::GetAllVertices() const
{
	return Vertices;
}

FVertex* ARoomActor::GetVertexByFloor(int32 Floor)
{
	FVertex* FoundVertex = Vertices.Find(Floor);
	return FoundVertex;
}

void ARoomActor::AddDoorPosition(UDoorPosition* DoorPositionToAdd)
{
	DoorPositions.Add(DoorPositionToAdd);
}