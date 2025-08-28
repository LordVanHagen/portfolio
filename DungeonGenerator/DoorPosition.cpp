// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/DoorPosition.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"

UDoorPosition::UDoorPosition()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FDoorData UDoorPosition::GetDoorData()
{
	return DoorData;
}

UGridNode* UDoorPosition::GetParentNode()
{
	return ParentNode;
}

void UDoorPosition::SetParentNode(AGridActor* InParent, FVector InDoorLocation)
{
	if (InParent)
	{
		//UE_LOG(LogTemp, Display, TEXT("%d nodes found."), InParent->GetRoomSpaceNeeded().CoordinatesInGrid.Num());
		//UE_LOG(LogTemp, Display, TEXT("New DoorPosition: %s"), *this->GetName());
		//UE_LOG(LogTemp, Display, TEXT("InDoorLocation is X: %f, Y: %f, Z: %f"), InDoorLocation.X, InDoorLocation.Y, InDoorLocation.Z);
		for (UGridNode* RoomSpace : InParent->GetRoomSpaceNeeded().CoordinatesInGrid)
		{
			//UE_LOG(LogTemp, Display, TEXT("RoomSpaceNeeded, X: %f, Y: %f, Z: %f"), RoomSpace.PositionInGrid.X, RoomSpace.PositionInGrid.Y, RoomSpace.PositionInGrid.Z);
			if (InDoorLocation == RoomSpace->GetPositionInGrid())
			{
				//UE_LOG(LogTemp, Display, TEXT("RoomSpace matches with DoorPosition"));
				ParentNode = RoomSpace;
				RoomSpace->SetDoor(this);
				break;
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No parent found"));
		return;
	}
}

void UDoorPosition::SetDoorIsUsed(bool InDoorIsUsed)
{
	bDoorIsUsed = InDoorIsUsed;
}

bool UDoorPosition::GetDoorIsUsed()
{
	return bDoorIsUsed;
}
