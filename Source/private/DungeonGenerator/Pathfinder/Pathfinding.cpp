// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/Pathfinder/Pathfinding.h"
#include "../GridSpawner.h"
#include "../GridActor.h"
#include "Algo/Reverse.h"

Pathfinding::Pathfinding()
{
}

void Pathfinding::Initialize(AGridSpawner* InGrid)
{
	Grid = InGrid;
}

void Pathfinding::FindPath(UGridNode* StartingNode, UGridNode* TargetNode, UGridNode* OutgoingNode, UGridNode* IngoingNode)
{
	// Create OpenSet
	TArray<UGridNode*> OpenSet;
	// Create ClosedSet
	TSet<UGridNode*> ClosedSet;

	StartingNode->SetGCost(0);
	StartingNode->SetHCost(GetDistance(StartingNode, TargetNode));
	StartingNode->SetParent(nullptr);
	// Add StartingNode to OpenSet
	OpenSet.Add(StartingNode);

	UE_LOG(LogTemp, Display, TEXT("Starting node is %s with position X: %f, Y: %f, Z: %f"), *StartingNode->GetName(), StartingNode->GetPositionInGrid().X, StartingNode->GetPositionInGrid().Y, StartingNode->GetPositionInGrid().Z);
	UE_LOG(LogTemp, Display, TEXT("Target node is %s with position X: %f, Y: %f, Z: %f"), *TargetNode->GetName(), TargetNode->GetPositionInGrid().X, TargetNode->GetPositionInGrid().Y, TargetNode->GetPositionInGrid().Z);

	// while loop
	while (OpenSet.Num() > 0)
	{
		// Create temporary CurrentNode resembling Node with lowest FCost from OpenSet
		UE_LOG(LogTemp, Display, TEXT("Doing operations..."));

		UGridNode* CurrentNode = OpenSet[0];

		if (OpenSet.Contains(TargetNode))
		{
			CurrentNode = TargetNode;
			UE_LOG(LogTemp, Display, TEXT("Target node is now current node."));
		}

		// Remove CurrentNode from OpenSet
		OpenSet.Remove(CurrentNode);
		// Add CurrentNode to ClosedSet
		ClosedSet.Add(CurrentNode);

		// if CurrentNode equals TargetNode call RetracePath and return
		if (CurrentNode == TargetNode)
		{
			UE_LOG(LogTemp, Display, TEXT("Path found."));
			RetracePath(StartingNode, TargetNode, OutgoingNode, IngoingNode);
			return;
		}

		TArray<UGridNode*> Neighbours = GetNeighbours(CurrentNode, TargetNode, OpenSet, ClosedSet);

		// foreach Neighbour of CurrentNode
		for (UGridNode* Neighbour : Neighbours)
		{
			//UE_LOG(LogTemp, Display, TEXT("Checking values for neighbour: %s"), *Neighbour->GetName());
			FVector NodePos = FVector(Neighbour->GetPositionInGrid());

			int32 NodePosX = FMath::RoundToInt(NodePos.X / Grid->GetGridCellActorSize().X);
			int32 NodePosY = FMath::RoundToInt(NodePos.Y / Grid->GetGridCellActorSize().Y);
			int32 NodePosZ = FMath::RoundToInt(NodePos.Z / Grid->GetGridCellActorSize().Z);

			int32 ValueAtCurrentNode = Grid->GetGridArray()[NodePosX][NodePosY][NodePosZ];

			// TODO: Later check for intersections to enable smoother traversal
			if ((ValueAtCurrentNode != 0  && ValueAtCurrentNode != 3) || ClosedSet.Contains(Neighbour))


			// if Neighbours GridValue is not 0 or Neighbour is in ClosedSet
			if (ValueAtCurrentNode != 0 || ClosedSet.Contains(Neighbour))
			{
				// Skip to the next Neighbour
				UE_LOG(LogTemp, Display, TEXT("Neighbours position in Grid cant be used or neighbour is allready in closed set."));
				continue;
			}

			int32 NewMovementCostToNeighbour = CurrentNode->GetGCost() + GetDistance(CurrentNode, Neighbour);

			// if new Path to Neighbour is shorter or neighbour is not in OpenSet
			if (NewMovementCostToNeighbour < Neighbour->GetGCost() || !OpenSet.Contains(Neighbour))
			{
				// set GCost of Neighbour
				Neighbour->SetGCost(NewMovementCostToNeighbour);
				// set HCost of Neighbour
				Neighbour->SetHCost(GetDistance(Neighbour, TargetNode));
				// set Parent of Neighbour to Current
				Neighbour->SetParent(CurrentNode);

				// if Neighbour is not in OpenSet, add Neighbour to Openset
				if(!OpenSet.Contains(Neighbour))
					OpenSet.Add(Neighbour);
			}	
		}

		SortOpenSet(OpenSet);
	}
}

void Pathfinding::RetracePath(UGridNode* StartingNode, UGridNode* TargetNode, UGridNode* OutgoingNode, UGridNode* IngoingNode)
{
	FPathData NewPathData;
	TArray<UGridNode*> Path;
	UGridNode* CurrentNode = TargetNode;

	UE_LOG(LogTemp, Display, TEXT("Retrace called."));

	while (CurrentNode != nullptr)
	{
		Path.Add(CurrentNode);
		CurrentNode = CurrentNode->GetParent();
	}

	Algo::Reverse(Path);

	NewPathData.Path = Path;
	NewPathData.OutgoingNode = OutgoingNode;
	NewPathData.IngoingNode = IngoingNode;

	// DEBUGGING
	//PrintPathData(Path);

	Grid->SetCurrentPath(NewPathData);
}

int32 Pathfinding::GetDistance(UGridNode* NodeA, UGridNode* NodeB)
{
	int32 DisX = FMath::Abs((NodeA->GetPositionInGrid().X / Grid->GetGridCellActorSize().X) - (NodeB->GetPositionInGrid().X / Grid->GetGridCellActorSize().X));
	int32 DisY = FMath::Abs((NodeA->GetPositionInGrid().Y / Grid->GetGridCellActorSize().Y) - (NodeB->GetPositionInGrid().Y / Grid->GetGridCellActorSize().Y));

	return 10 * (DisX + DisY);
}

void Pathfinding::PrintPathData(TArray<UGridNode*> InPath)
{
	if (InPath.Num() > 0)
	{
		UE_LOG(LogTemp, Display, TEXT("Amount of nodes found in path: %d"), InPath.Num());

		FColor PathColor = FColor::Cyan;
		FColor StartColor = FColor::Magenta;
		FColor TargetColor = FColor::Green;

		int32 Counter = 0;

		for (UGridNode* Node : InPath)
		{
			FVector Pos = Node->GetPositionInGrid();
			UE_LOG(LogTemp, Display, TEXT("Node found with position X: %f, Y: %f, Z: %f"), Pos.X, Pos.Y, Pos.Z);

			FColor CurrentColor = PathColor;
			if (Counter == 0)
				CurrentColor = StartColor;

			if (Counter == InPath.Num() - 1)
				CurrentColor = TargetColor;

			DrawDebugSphere(
				Grid->GetWorld(),
				Pos,
				500.f,
				12,
				CurrentColor,
				true
				);

			Counter++;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No path found."));

	}
}

TArray<UGridNode*> Pathfinding::GetNeighbours(UGridNode* Node, UGridNode* TargetNode, TArray<UGridNode*> OpenSet, TSet<UGridNode*> ClosedSet)
{
	TArray<UGridNode*> ValidNeighbours;

	TArray<FVector> Offsets = {
		FVector(0.0f, Grid->GetGridCellActorSize().Y, 0.0f),
		FVector(0.0f, -Grid->GetGridCellActorSize().Y, 0.0f),
		FVector(-Grid->GetGridCellActorSize().X, 0.0f, 0.0f),
		FVector(Grid->GetGridCellActorSize().X, 0.0f, 0.0f),
	};

	for (int i = 0; i < Offsets.Num(); i++)
	{
		FVector Check = Node->GetPositionInGrid() + Offsets[i];

		if (Check == TargetNode->GetPositionInGrid())
		{
			UE_LOG(LogTemp, Display, TEXT("Target was added to neighbours."));
			ValidNeighbours.Add(TargetNode);
			continue;
		}

		//UE_LOG(LogTemp, Display, TEXT("Potential neighbour at position X: %f, Y: %f, Z: %f"), Check.X, Check.Y, Check.Z);

		FVector CheckedForGrid = FVector(Check.X / Grid->GetGridCellActorSize().X, Check.Y / Grid->GetGridCellActorSize().Y, Check.Z / Grid->GetGridCellActorSize().Z);

		int32 ConvertedX = FMath::RoundToInt(CheckedForGrid.X);
		int32 ConvertedY = FMath::RoundToInt(CheckedForGrid.Y);
		int32 ConvertedZ = FMath::RoundToInt(CheckedForGrid.Z);

		FVector GridCheckRounded = FVector(ConvertedX, ConvertedY, ConvertedZ);

		UE_LOG(LogTemp, Display, TEXT("Potential neighbour at grid location X: %d, Y: %d, Z: %d"), ConvertedX, ConvertedY, ConvertedZ);

		if (!Grid->IsOutOfBounds(GridCheckRounded))
		{
			bool bIsInOpenSet = false;
			bool bIsInClosedSet = false;

			if (OpenSet.Num() > 0)
			{
				for (UGridNode* OpenNode : OpenSet)
				{
					if (OpenNode->GetPositionInGrid() == Check)
					{
						bIsInOpenSet = true;
						UE_LOG(LogTemp, Display, TEXT("Neighbour allready in openset. Neighbour wont be added to valid neighbours."));
						break;
					}
				}
			}

			if (ClosedSet.Num() > 0)
			{
				for (UGridNode* ClosedNode : ClosedSet)
				{
					if (ClosedNode->GetPositionInGrid() == Check)
					{
						bIsInClosedSet = true;
						UE_LOG(LogTemp, Display, TEXT("Neighbour allready in closedset. Neighbour wont be added to valid neighbours."));
						break;
					}
				}
			}

			if (!bIsInOpenSet && !bIsInClosedSet)
			{
				UGridNode* NewNode = NewObject<UGridNode>();
				NewNode->SetPositionInGrid(Check);
				ValidNeighbours.Add(NewNode);
			}
		}
	}

	return ValidNeighbours;
}

// Helper function for partitioning of the array
int Pathfinding::Partition(TArray<UGridNode*>& OpenSet, int low, int high)
{
	// select pivot element
	UGridNode* pivot = OpenSet[high];
	int i = low - 1;

	for (int j = low; j < high; j++)
	{
		// Compare the current element to the pivot element
		if (OpenSet[j]->GetFCost() < pivot->GetFCost() ||
			(OpenSet[j]->GetFCost() == pivot->GetFCost() && OpenSet[j]->GetHCost() < pivot->GetHCost()))
		{
			// Swap elements, if the current element is smaller or equal to the pivot element
			i++;
			OpenSet.Swap(i, j);
		}
	}
	// Swap the pivot element into its correct position.
	OpenSet.Swap(i + 1, high);
	return i + 1;
}

void Pathfinding::QuickSort(TArray<UGridNode*>& OpenSet, int low, int high)
{
	if (low < high)
	{
		// Partition the array to get the pivot position
		int pivotIndex = Partition(OpenSet, low, high);

		// Recursive calls for the two parts of the array
		QuickSort(OpenSet, low, pivotIndex - 1);
		QuickSort(OpenSet, pivotIndex + 1, high);
	}
}

void Pathfinding::SortOpenSet(TArray<UGridNode*>& OpenSet)
{
	if (OpenSet.Num() > 0)
	{
		UE_LOG(LogTemp, Display, TEXT("Sorting OpenSet using Quicksort."));

		QuickSort(OpenSet, 0, OpenSet.Num() - 1);
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Cannot sort OpenSet, since OpenSet is empty."));
	}
}