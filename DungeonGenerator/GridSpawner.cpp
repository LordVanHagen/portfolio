// Fill out your copyright notice in the Description page of Project Settings.


#include "GridSpawner.h"
#include "GridActor.h"
#include "RoomActor.h"
#include "SpoopyGameStatics.h"
#include "Pathfinder/Pathfinding.h"
#include "DoorPosition.h"

#pragma region INITIALIZATION

AGridSpawner::AGridSpawner()
{
}

void AGridSpawner::BeginPlay()
{
	Super::BeginPlay();

	BeginBuildingSteps();
}

void AGridSpawner::BeginBuildingSteps()
{
	StartAfter(this, GridInitTimer, [this]() { TryBuildingGrid(); });
	//TryBuildingGrid();

	StartAfter(this, RoomSpawnTimer, [this]()
		{ TryPlacingRooms();

			if (DestroyGridCellsAfterGeneration)
			{
				DestroyAllGridCellsLeft();
			}

			SetFloorDetails();

			// DEBUG TEST
			LogFloorDetails();
		});
	
	//TryPlacingRooms();

	StartAfter(this, StairSpawnTimer, [this]() { TryPlacingStairs(); });
	//TryPlacingStairs();

	StartAfter(this, FloorSpawnTimer, [this]() { TryConnectingRooms(); });
	//TryConnectingRooms();
}

#pragma endregion

#pragma region << STEP_01: BUILDING THE GRID >>

void AGridSpawner::TryBuildingGrid()
{
	CreateGridArray();
	GenerateGrid();
}

void AGridSpawner::CreateGridArray()
{
	for (int32 x = 0; x < GridSize.X; x++)
	{
		TArray<TArray<int32>> DepthArray;
		for (int32 y = 0; y < GridSize.Y; y++)
		{
			TArray<int32> RowArray;
			for (int32 z = 0; z < GridSize.Z; z++)
			{
				RowArray.Add(0);
			}
			DepthArray.Add(RowArray);
		}
		GridArray.Add(DepthArray);
	}
}

void AGridSpawner::GenerateGrid()
{
	int32 NumCellsX = GridSize.X;
	int32 NumCellsY = GridSize.Y;
	int32 NumCellsZ = GridSize.Z;

	for (int32 X = 0; X < NumCellsX; ++X)
	{
		for (int32 Y = 0; Y < NumCellsY; ++Y)
		{
			for (int32 Z = 0; Z < NumCellsZ; ++Z)
			{
				FVector SpawnLocation = FVector(X * GridCellActorSize.X, Y * GridCellActorSize.Y, Z * GridCellActorSize.Z);

				AGridActor* SpawnedActor = GetWorld()->SpawnActor<AGridActor>(GridCellActorClass, SpawnLocation, FRotator::ZeroRotator);
				if (SpawnedActor)
				{
					SpawnedActor->SetRoomSpaceNeeded(SpawnLocation, FRotator::ZeroRotator);

					GridActors.Add(SpawnedActor);
				}
			}
		}
	}
}

#pragma endregion


#pragma region << STEP_02: PLACING THE ROOMS >>

void AGridSpawner::TryPlacingRooms()
{
	int RoomsSpawned = 0;
	const int MaxRetries = 10;
	int CurrentRetry = 0;

	while (RoomsSpawned < AmountOfRoomsToSpawn && CurrentRetry <= MaxRetries)
	{
		if (CouldSpawnRoom())
		{
			RoomsSpawned++;
			continue;
		}

		CurrentRetry++;
	}

	if (RoomsSpawned != AmountOfRoomsToSpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Only %d rooms could be spawned"), RoomsSpawned);
	}
}


bool AGridSpawner::CouldSpawnRoom()
{
	FVector SpawnLocation = GetNewRandomLocation();

	if (GridArray[SpawnLocation.X / GridCellActorSize.X][SpawnLocation.Y / GridCellActorSize.Y][SpawnLocation.Z / GridCellActorSize.Z] == 0)
	{
		if (SpawnRoom(SpawnLocation))
		{
			return true;
		}
	}

	return false;
}

bool AGridSpawner::SpawnRoom(FVector InSpawnLocation)
{
	FVector SpawnLocation = InSpawnLocation;
	static int32 MaxTryCounter = 20;
	int32 CurrentTry = 0;
	bool RoomSet = false;

	TSubclassOf<ARoomActor> RoomToSpawn = PossibleRoomsToSpawn[FMath::RandRange(0, PossibleRoomsToSpawn.Num() - 1)];

	FRotator SpawnRotation = GetRandomRotation();

	ARoomActor* SpawnedRoom = GetWorld()->SpawnActor<ARoomActor>(RoomToSpawn, SpawnLocation, SpawnRotation);

	//UE_LOG(LogTemp, Warning, TEXT("Room was spawned at location: X: %f, Y: %f, Z: %f"), SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
	//UE_LOG(LogTemp, Warning, TEXT("Gridsize: X: %f, Y: %f, Z: %f"), GridSize.X, GridSize.Y, GridSize.Z);

	if (SpawnedRoom)
	{
		SpawnedRoom->SetRoomSpaceNeeded(SpawnLocation, SpawnRotation);
		//SpawnedRoom->LogAllRoomSpacesNeeded();

		while (!RoomSet && CurrentTry < MaxTryCounter)
		{
			TArray<UGridNode*> CoordinatesInGridNeeded = SpawnedRoom->GetRoomSpaceNeeded().CoordinatesInGrid;
			bool AllInBounds = true;
			for (const UGridNode* Coordinates : CoordinatesInGridNeeded)
			{
				float X = Coordinates->GetPositionInGrid().X / GridCellActorSize.X;
				float Y = Coordinates->GetPositionInGrid().Y / GridCellActorSize.Y;
				float Z = Coordinates->GetPositionInGrid().Z / GridCellActorSize.Z;

				//UE_LOG(LogTemp, Warning, TEXT("Rooms Space in Grid: X: %f, Y: %f, Z: %f"), X, Y, Z);

				if (X < 0 || X >= GridSize.X || Y < 0 || Y >= GridSize.Y || Z < 0 || Z >= GridSize.Z)
				{
					// UE_LOG(LogTemp, Warning, TEXT("Room Was out of Bounds"));
					AllInBounds = false;
					break;
				}
			}

			if (!AllInBounds)
			{
				SetNewRoomData(SpawnedRoom);
				CurrentTry++;
				continue;
			}

			bool DoorsHaveEnoughSpace = this->DoorsHaveEnoughSpace(SpawnedRoom);

			if (!DoorsHaveEnoughSpace)
			{
				UE_LOG(LogTemp, Warning, TEXT("Door of room was blocked."));
				SetNewRoomData(SpawnedRoom);
				CurrentTry++;
				continue;
			}

			bool AllSpacesAvailable = true;
			for (const UGridNode* Coordinates : CoordinatesInGridNeeded)
			{
				int32 X = FMath::RoundToInt(Coordinates->GetPositionInGrid().X / GridCellActorSize.X);
				int32 Y = FMath::RoundToInt(Coordinates->GetPositionInGrid().Y / GridCellActorSize.Y);
				int32 Z = FMath::RoundToInt(Coordinates->GetPositionInGrid().Z/ GridCellActorSize.Z);

				//UE_LOG(LogTemp, Display, TEXT("Coordinates in Grid tested: X: %d, Y: %d, Z: %d"), X, Y, Z);

				if (GridArray[X][Y][Z] != 0)
				{
					AllSpacesAvailable = false;
					break;
				}
			}

			if (!AllSpacesAvailable)
			{
				//(LogTemp, Warning, TEXT("Room is overlapping with another room"));
				SetNewRoomData(SpawnedRoom);
				CurrentTry++;
				continue;
			}

			if (AllSpacesAvailable)
			{
				for (const UGridNode* Coordinates : CoordinatesInGridNeeded)
				{
					int32 X = FMath::RoundToInt(Coordinates->GetPositionInGrid().X / GridCellActorSize.X);
					int32 Y = FMath::RoundToInt(Coordinates->GetPositionInGrid().Y / GridCellActorSize.Y);
					int32 Z = FMath::RoundToInt(Coordinates->GetPositionInGrid().Z / GridCellActorSize.Z);

					GridArray[X][Y][Z] = 1;
					//UE_LOG(LogTemp, Display, TEXT("Room now occupies these spaces in Grid: X: %d, Y: %d, Z: %d"), X, Y, Z);
				}

				//UE_LOG(LogTemp, Display, TEXT("Room was successfully spawned into Grid"));
				RoomSet = true;
				GridActors.Add(SpawnedRoom);
				SpawnedRooms.Add(SpawnedRoom);
				if (ARoomActor* RoomToAdd = Cast<ARoomActor>(SpawnedRoom))
				{
					if (RoomToAdd->GetRoomSpaceNeeded().Floors.Num() > 0)
					{
						int32 CurrentFloor = RoomToAdd->GetRoomSpaceNeeded().Floors[0];
						if (RoomsByFloors.Contains(CurrentFloor))
						{
							RoomsByFloors[CurrentFloor].Add(RoomToAdd);
						}
						else
						{
							RoomsByFloors.Add(CurrentFloor, { RoomToAdd });
						}
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("SpawnedRoom could not be cast into RoomActor"));
				}
			}

			TArray<AGridActor*> CellPiecesToRemove;
			for (AGridActor* GridActor : GridActors)
			{
				if (GridActor && GridActor->GetRoomType() == ERoomType::CellPiece)
				{
					if(GridActor)
					if (SpawnedRoom->GetRoomSpaceNeeded().CoordinatesInGrid.Contains(GridActor->GetStartingGridNode()))
					{
						CellPiecesToRemove.Add(GridActor);
					}
				}
			}

			for (AGridActor* CellPiece : CellPiecesToRemove)
			{
				//UE_LOG(LogTemp, Warning, TEXT("CellActor %s was destroyed at: X: %f, Y: %f, Z: %f"), *CellPiece->GetName(), CellPiece->GetRoomSpaceNeeded().StartingLocation.X, CellPiece->GetRoomSpaceNeeded().StartingLocation.Y, CellPiece->GetRoomSpaceNeeded().StartingLocation.Z);
				GridActors.Remove(CellPiece);
				CellPiece->Destroy();
			}

			return RoomSet;
		}
		if (CurrentTry == MaxTryCounter)
		{
			UE_LOG(LogTemp, Error, TEXT("Room could not be Spawned"));
			UE_LOG(LogTemp, Error, TEXT("Max tries reached, destroy SpawnedRoom"));
			SpawnedRoom->Destroy();
			return RoomSet = false;
		}
	}

	return RoomSet;
}

void AGridSpawner::SetNewRoomData(AGridActor* CurrentRoom, int32 Floor)
{
	CurrentRoom->EmptyRoomSpaceNeeded();
	FVector NewSpawnLocation;
	if (Floor != INT_MIN)
		NewSpawnLocation = GetNewRandomLocation(Floor);
	else
		NewSpawnLocation = GetNewRandomLocation();
	FRotator NewSpawnRotation = GetRandomRotation();
	CurrentRoom->SetRoomSpaceNeeded(NewSpawnLocation, NewSpawnRotation);
	//UE_LOG(LogTemp, Warning, TEXT("Room was newly spawned at location: X: %f, Y: %f, Z: %f"), NewSpawnLocation.X, NewSpawnLocation.Y, NewSpawnLocation.Z);
	//CurrentRoom->LogAllRoomSpacesNeeded();
	CurrentRoom->SetActorLocation(NewSpawnLocation);
	CurrentRoom->SetActorRotation(NewSpawnRotation);
}

FVector AGridSpawner::GetNewRandomLocation(int32 Floor)
{
	int32 RoundedGridSizeX = FMath::RoundToInt(GridSize.X);
	int32 RoundedGridSizeY = FMath::RoundToInt(GridSize.Y);
	int32 RoundedGridSizeZ = FMath::RoundToInt(GridSize.Z);

	int32 RandomX = FMath::RandRange(0, RoundedGridSizeX - 1);
	int32 RandomY = FMath::RandRange(0, RoundedGridSizeY - 1);
	int32 RandomZ = FMath::RandRange(0, RoundedGridSizeZ - 1);

	if (Floor != INT_MIN)
		RandomZ = Floor;

	return FVector(RandomX * GridCellActorSize.X, RandomY * GridCellActorSize.Y, RandomZ * GridCellActorSize.Z);
}

FRotator AGridSpawner::GetRandomRotation()
{
	int32 RandomValue = FMath::RandRange(0, 3);

	switch (RandomValue)
	{
	case 1:
		return FRotator(0.f, 90.f, 0.f);
	case 2:
		return FRotator(0.f, 180.f, 0.f);
	case 3:
		return FRotator(0.f, 270.f, 0.f);
	default:
		return FRotator::ZeroRotator;
	}
}

#pragma endregion


#pragma region << STEP_03: PLACING THE STAIRS >>

void AGridSpawner::TryPlacingStairs()
{

	for (int32 Floor = FloorDetails.HighestFloor; Floor > FloorDetails.LowestFloor; Floor--)
	{
		if (FloorDetails.FloorHasStairs[Floor])
		{
			//UE_LOG(LogTemp, Warning, TEXT("Floor %d allready contains a room with stairs"), Floor);
			continue;
		}

		const int32 MaxTries = 20;
		int32 CurrentTry = 0;
		bool RoomSet = false;

		// Set Initial Spawn Location and Rotation
		FVector SpawnLocation = GetNewRandomLocation(Floor);
		FRotator SpawnRotation = GetRandomRotation();
		
		// Choose Stairs actor to spawn
		TSubclassOf<ARoomActor> StairsToSpawn = PossibleStairsToSpawn[FMath::RandRange(0, PossibleStairsToSpawn.Num() - 1)];
		
		// Spawn Stairs actor
		ARoomActor* SpawnedStairs = GetWorld()->SpawnActor<ARoomActor>(StairsToSpawn, SpawnLocation, SpawnRotation);
		
		// Calculate The Space needed by the Stairs in the grid
		if (SpawnedStairs)
		{
			while (!RoomSet && CurrentTry < MaxTries)
			{
				SpawnedStairs->SetRoomSpaceNeeded(SpawnLocation, SpawnRotation);
				TArray<UGridNode*> CoordinatesInGridNeeded = SpawnedStairs->GetRoomSpaceNeeded().CoordinatesInGrid;

				bool DoorsHaveEnoughSpace = this->DoorsHaveEnoughSpace(SpawnedStairs);

				if (!DoorsHaveEnoughSpace)
				{
					UE_LOG(LogTemp, Warning, TEXT("Door of stairs was blocked."));
					SetNewRoomData(SpawnedStairs, Floor);
					CurrentTry++;
					continue;
				}

				bool AllSpacesAvailable = true;
				for (const UGridNode* Coordinates : CoordinatesInGridNeeded)
				{
					int32 X = FMath::RoundToInt(Coordinates->GetPositionInGrid().X / GridCellActorSize.X);
					int32 Y = FMath::RoundToInt(Coordinates->GetPositionInGrid().Y / GridCellActorSize.Y);
					int32 Z = FMath::RoundToInt(Coordinates->GetPositionInGrid().Z / GridCellActorSize.Z);

					//UE_LOG(LogTemp, Warning, TEXT("Coordinates in Grid tested: X: %d, Y: %d, Z: %d"), X, Y, Z);

					if (GridArray[X][Y][Z] == 1 || GridArray[X][Y][Z] == 2)
					{
						AllSpacesAvailable = false;
						break;
					}


				}

				if (!AllSpacesAvailable)
				{
					//(LogTemp, Warning, TEXT("Stairs are overlapping with another room or stairs"));
					SetNewRoomData(SpawnedStairs, Floor);
					CurrentTry++;
					continue;
				}

				if (AllSpacesAvailable)
				{
					for (const UGridNode* Coordinates : CoordinatesInGridNeeded)
					{
						int32 X = FMath::RoundToInt(Coordinates->GetPositionInGrid().X / GridCellActorSize.X);
						int32 Y = FMath::RoundToInt(Coordinates->GetPositionInGrid().Y / GridCellActorSize.Y);
						int32 Z = FMath::RoundToInt(Coordinates->GetPositionInGrid().Z / GridCellActorSize.Z);

						GridArray[X][Y][Z] = 1;
						//UE_LOG(LogTemp, Warning, TEXT("Room now occupies these spaces in Grid: X: %d, Y: %d, Z: %d"), X, Y, Z);
					}

					//UE_LOG(LogTemp, Warning, TEXT("Stairs were successfully spawned into Grid"));
					RoomSet = true;
					GridActors.Add(SpawnedStairs);
					SpawnedRooms.Add(SpawnedStairs);
					if (ARoomActor* StairsToAdd = Cast<ARoomActor>(SpawnedStairs))
					{
						if (StairsToAdd->GetRoomSpaceNeeded().Floors.Num() > 0)
						{
							int32 CurrentFloor = StairsToAdd->GetRoomSpaceNeeded().Floors[0];
							if (RoomsByFloors.Contains(CurrentFloor))
							{
								RoomsByFloors[CurrentFloor].Add(StairsToAdd);
							}
							else
							{
								RoomsByFloors.Add(CurrentFloor, { StairsToAdd });
							}
						}
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("SpawnedStairs couldnt be casted into RoomActor"));
					}

					///// --> DEBUG TESTING
					//SpawnedStairs->LogAllFloors();
				}

				TArray<AGridActor*> CellPiecesToRemove;
				for (AGridActor* GridActor : GridActors)
				{
					if (GridActor && GridActor->GetRoomType() == ERoomType::CellPiece)
					{
						if (SpawnedStairs->GetRoomSpaceNeeded().CoordinatesInGrid.Contains(GridActor->GetStartingGridNode()))
						{
							CellPiecesToRemove.Add(GridActor);
						}
					}
				}

				for (AGridActor* CellPiece : CellPiecesToRemove)
				{
					//UE_LOG(LogTemp, Warning, TEXT("CellActor %s was destroyed at: X: %f, Y: %f, Z: %f"), *CellPiece->GetName(), CellPiece->GetRoomSpaceNeeded().StartingLocation.X, CellPiece->GetRoomSpaceNeeded().StartingLocation.Y, CellPiece->GetRoomSpaceNeeded().StartingLocation.Z);
					GridActors.Remove(CellPiece);
					CellPiece->Destroy();
				}
			}
		}

		//UE_LOG(LogTemp, Warning, TEXT("Stairs where spawned for floors %d and %d"), Floor, Floor-1);
	}
}

void AGridSpawner::SetFloorDetails()
{
	if (SpawnedRooms.Num() == 0)
	{
		//UE_LOG(LogTemp, Warning, TEXT("No rooms have been spawned yet"));
		return;
	}

	int32 HighestFloor = INT_MIN;
	int32 LowestFloor = INT_MAX;
	TArray<int32> FloorsThatCountAsStairs;

	for (AGridActor* Room : SpawnedRooms)
	{
		if (!Room)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Room was invalid"));
			continue;
		}

		for (int32 Floor : Room->GetRoomSpaceNeeded().Floors)
		{
			HighestFloor = FMath::Max(HighestFloor, Floor);
			LowestFloor = FMath::Min(LowestFloor, Floor);
		}

		if (Room->GetRoomCountsAsStairs())
		{
			FloorsThatCountAsStairs.Add(Room->GetRoomSpaceNeeded().Floors[0]);
		}
	}

	FloorDetails.HighestFloor = HighestFloor;
	FloorDetails.LowestFloor = LowestFloor;

	for (int32 Floor = LowestFloor; Floor <= HighestFloor; Floor++)
	{
		if (FloorsThatCountAsStairs.Contains(Floor))
		{
			FloorDetails.FloorHasStairs.Add(Floor, true);
		}
		else
		{
			FloorDetails.FloorHasStairs.Add(Floor, false);
		}
	}

	//UE_LOG(LogTemp, Warning, TEXT("Highest floor used: %d"), HighestFloor);
	//UE_LOG(LogTemp, Warning, TEXT("Lowest floor used: %d"), LowestFloor);
}

void AGridSpawner::LogFloorDetails()
{
	for (const auto& Entry : FloorDetails.FloorHasStairs)
	{
		int32 Floor = Entry.Key;
		bool HasStairs = Entry.Value;

		//UE_LOG(LogTemp, Warning, TEXT("Floor: %d, Has Stairs: %s"), Floor, HasStairs ? TEXT("true") : TEXT("false"));
	}
}
#pragma endregion


#pragma region << STEP_04: CONNECT THE ROOMS >>


void AGridSpawner::TryConnectingRooms()
{
	// Check if more than one Vertex can be created
	if (SpawnedRooms.Num() <= 1)
		return;

	Triangulate();

	//UE_LOG(LogTemp, Warning, TEXT("Triangulation succeeded."));

	//CreateConnections();
}

void AGridSpawner::Triangulate()
{
	//UE_LOG(LogTemp, Warning, TEXT("Triangulation called"));
	//UE_LOG(LogTemp, Warning, TEXT("Current highest floor: %d, current lowest floor: %d"), FloorDetails.HighestFloor, FloorDetails.LowestFloor);
	for (int32 Floor = FloorDetails.HighestFloor; Floor >= FloorDetails.LowestFloor; Floor--)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Triangulation in progress on %d floor"), Floor);
		TMap<FVertex, ARoomActor*> Vertices;

		if (RoomsByFloors.Contains(Floor) && RoomsByFloors[Floor].Num() > 0)
		{
			for (ARoomActor* RoomActor : RoomsByFloors[Floor])
			{
				if (RoomActor->GetCOMWasGenerated())
				{
					//UE_LOG(LogTemp, Warning, TEXT("Room allready set."));
					continue;
				}

				RoomActor->SpawnDoorsInRoom();
				RoomActor->CalculateCenterOfMass();
				//UE_LOG(LogTemp, Warning, TEXT("Center of Mass created for Room: '%s' with '%d' non optional doors"), *RoomActor->GetName(), RoomActor->GetAmountOfNonOptionalDoors());

				for (const TPair<int32, FVertex>& Pair : RoomActor->GetAllVertices())
				{
					int32 FloorToAdd = Pair.Key;

					const FVertex& Vertex = Pair.Value;

					VerticesByFloor.FindOrAdd(FloorToAdd).Add(FVertex(Vertex), RoomActor);
				}

				RoomActor->SetCOMWasGenerated(true);
			}
			Vertices = VerticesByFloor[Floor];

			//PrintVerticesByFloor(Floor);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Floor contains no Rooms"));

			Vertices = VerticesByFloor[Floor];

			//PrintVerticesByFloor(Floor);
		}
		if (Vertices.Num() <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("No vertices found."));
			//PrintVerticesByFloor();
			return;
		}
		TArray<FVertex> VertexArray;
		Vertices.GenerateKeyArray(VertexArray);
		Delaunay = UDelaunay::Triangulate(VertexArray, Floor, GridCellActorSize.Z);
		
#if WITH_EDITOR
		if (bDrawDelaunayEdges)
		{
			TArray<FDelaunayEdge> CurrentEdges = Delaunay->Edges;
			StartAfter(GetWorld(), DelaunayEdgesDrawTimer, [this, CurrentEdges]() { DrawEdges<FDelaunayEdge>(GetWorld(), CurrentEdges, FColor::Cyan, DelaunayEdgesDrawDuration); });

			//DrawDelaunayEdges(Delaunay->Edges);
		}
#endif
		
		CreateConnections(Delaunay->Edges, Vertices);
		Vertices.Empty();
	}
}

void AGridSpawner::CreateConnections(TArray<FDelaunayEdge> InEdges, TMap<FVertex, ARoomActor*> RoomsByVertices)
{
	TArray<FPrimEdge> Edges;

	for (const FDelaunayEdge& Edge : InEdges)
	{
		Edges.Add(FPrimEdge(Edge.U, Edge.V));
	}

	if(Edges.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Minimum spanning tree not possible to create, not enough rooms available."));
		return;
	}

	TArray<FPrimEdge> MST = USpoopyGameStatics::MinimumSpanningTree(Edges, Edges[0].U);

#if WITH_EDITOR
	if (bDrawPrimEdges)
	{
		StartAfter(GetWorld(), PrimEdgesDrawTimer, [this, MST]() { DrawEdges<FPrimEdge>(GetWorld(), MST, FColor::Red, PrimEdgesDrawDuration); });

		//DrawPrimEdges(MST);
	}
#endif

	TSet<FPrimEdge> SelectedEdges = TSet<FPrimEdge>(MST);

	TSet<FPrimEdge> LoopedEdges = GenerateLoops(Edges, SelectedEdges);

#if WITH_EDITOR
	if (bDrawAllEdges)
	{
		TArray LoopedArray = LoopedEdges.Array();
		StartAfter(GetWorld(), AllEdgesDrawTimer, [this, LoopedArray]() { DrawEdges<FPrimEdge>(GetWorld(), LoopedArray, FColor::Purple, AllEdgesDrawDuration); });

		//DrawAllEdges(LoopedEdges);
	}
#endif

	TMap<FVertex, ARoomActor*> CurrentRoomsByVertices = RoomsByVertices;
	StartAfter(GetWorld(), CorridorSpawnTimer, [this, LoopedEdges, CurrentRoomsByVertices]() { SpawnCorridors(LoopedEdges, CurrentRoomsByVertices); });
}

TSet<FPrimEdge> AGridSpawner::GenerateLoops(TArray<FPrimEdge> AllEdges, TSet<FPrimEdge> SelectedEdges)
{
	TSet<FPrimEdge> RemainingEdges;
	TSet<FPrimEdge> LoopedEdges;

	//UE_LOG(LogTemp, Display, TEXT("AllEdged count: %d"), AllEdges.Num());
	//UE_LOG(LogTemp, Display, TEXT("SelectedEdges count: %d"), SelectedEdges.Num());

	for (const FPrimEdge& Edge : AllEdges)
	{
		if (!SelectedEdges.Contains(Edge))
		{
			RemainingEdges.Add(Edge);
		}
	}

	//UE_LOG(LogTemp, Display, TEXT("RemainingEdges count: %d"), RemainingEdges.Num());

	if (RemainingEdges.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No remaining Edges can be added."));
		return SelectedEdges;
	}

	for (const FPrimEdge& Edge : SelectedEdges)
	{
		LoopedEdges.Add(Edge);
	}

	for (const FPrimEdge& EdgeToAdd : RemainingEdges)
	{
		int32 RandomNumber = rand() % 100;
		if (RandomNumber < PathSpawnPercentage)
		{
			LoopedEdges.Add(EdgeToAdd);
		}
	}

	return LoopedEdges;
}

void AGridSpawner::SpawnCorridors(TSet<FPrimEdge> LoopedEdges, TMap<FVertex, ARoomActor*> RoomsByVertices)
{
	Pathfinding* Pathfinder = new Pathfinding();
	Pathfinder->Initialize(this);
	
	// THIS NEEDS TO BE IMPLEMENTED IN THE ROOM SPAWNING PROCESS
	// Check for all DoorPositions to be available and not blocked, if so set them as bIsBlocked
	// if all DoorPositions are beeing blocked, move the RoomActor to a new Location
	// if a RoomActor passes all the checks, mark it as bIsSet, and dont allow them to be moved again

	// 1.) Determine Vertices of the Edges

	//UE_LOG(LogTemp, Display, TEXT("Amount of edges: %d"), LoopedEdges.Num());

	for (const FPrimEdge& Edge : LoopedEdges)
	{
		FVertex VertU = FVector(FLT_MIN, FLT_MIN, FLT_MIN);
		FVertex VertV = FVector(FLT_MIN, FLT_MIN, FLT_MIN);

		//UE_LOG(LogTemp, Log, TEXT("Edges U position X: %f, Y: %f, Z: %f | Edges V position X: %f, Y: %f, Z: %f"), Edge.U.Position.X, Edge.U.Position.Y, Edge.U.Position.Z, Edge.V.Position.X, Edge.V.Position.Y, Edge.V.Position.Z);

		for (const TPair<FVertex, ARoomActor*>& Pair : RoomsByVertices)
		{
			//UE_LOG(LogTemp, Display, TEXT("Current V position X: %f, Y: %f, Z: %f"), Pair.Key.Position.X, Pair.Key.Position.Y, Pair.Key.Position.Z);

			if (Pair.Key.Position == Edge.U.Position)
			{
				VertU = Pair.Key.Position;
				//UE_LOG(LogTemp, Log, TEXT("Vertex U used with Room: %s at position X: %f, Y: %f, Z: %f "), *Pair.Value->GetName(), Pair.Key.Position.X, Pair.Key.Position.Y, Pair.Key.Position.Z);
			}
			if (Pair.Key.Position == Edge.V.Position)
			{
				VertV = Pair.Key.Position;
				//UE_LOG(LogTemp, Log, TEXT("Vertex V used with Room: %s at position X: %f, Y: %f, Z: %f "), *Pair.Value->GetName(), Pair.Key.Position.X, Pair.Key.Position.Y, Pair.Key.Position.Z);
			}

			if (VertU != FVector(FLT_MIN, FLT_MIN, FLT_MIN) && VertV != FVector(FLT_MIN, FLT_MIN, FLT_MIN))
			{
				//UE_LOG(LogTemp, Log, TEXT("U position X: %f, Y: %f, Z: %f | V position X: %f, Y: %f, Z: %f"), VertU.Position.X, VertU.Position.Y, VertU.Position.Z, VertV.Position.X, VertV.Position.Y, VertV.Position.Z);
				break;
			}
		}
		// 2.) Edges with corresponding Vertices known

		if (VertU != FVector(FLT_MIN, FLT_MIN, FLT_MIN) && VertV != FVector(FLT_MIN, FLT_MIN, FLT_MIN))
		{
			ARoomActor* OutgoingRoom = nullptr;
			ARoomActor* IngoingRoom = nullptr;

			TPair<ARoomActor*, TArray<UGridNode*>> DoorNodesOutgoingRoom;
			TPair<ARoomActor*, TArray<UGridNode*>> DoorNodesIngoingRoom;

			// 3.) Determine which Room matches the Vertices
			for (TPair<FVertex, ARoomActor*>& Elem : RoomsByVertices)
			{
				FVertex Vertex = Elem.Key;

				if (Vertex == VertU)
				{
					OutgoingRoom = Elem.Value;
					//UE_LOG(LogTemp, Display, TEXT("Vertex U for Edge found with position X: %f, Y: %f, Z: %f"), Vertex.Position.X, Vertex.Position.Y, Vertex.Position.Z);
				}

				if (Vertex == VertV)
				{
					IngoingRoom = Elem.Value;
					//UE_LOG(LogTemp, Display, TEXT("Vertex V for Edge found with position X: %f, Y: %f, Z: %f"), Vertex.Position.X, Vertex.Position.Y, Vertex.Position.Z);
				}

				if (OutgoingRoom && IngoingRoom)
				{
					//UE_LOG(LogTemp, Display, TEXT("Both rooms set.\n - Outgoing room: %s\n - Ingoingroom: %s"), *OutgoingRoom->GetName(), *IngoingRoom->GetName());
					break;
				}
			}

			int32 CurrentFloor = VertV.Position.Z - (GridCellActorSize.Z / 2);

			DoorNodesOutgoingRoom.Key = OutgoingRoom;
			DoorNodesOutgoingRoom.Value = GetDoorNodesByRooms(OutgoingRoom, CurrentFloor);

			DoorNodesIngoingRoom.Key = IngoingRoom;
			DoorNodesIngoingRoom.Value = GetDoorNodesByRooms(IngoingRoom, CurrentFloor);

			//// 4.) Determine which GridNode with a DoorPosition of the outgoing room is closest to the center of mass of the incoming room, based on distance
			//// 4.1.) Determine if the DoorPosition is already being used, as each door can only be used once
			UGridNode* NearestOutgoingNode = GetNearestNode(VertV, DoorNodesOutgoingRoom);
			if (NearestOutgoingNode != nullptr)
			{
				FVector StartingPosition = GetNodeBeforeDoor(NearestOutgoingNode->GetDoor(), OutgoingRoom);
				if (StartingPosition != FVector(FLT_MIN, FLT_MIN, FLT_MIN))
				{
					UGridNode* StartingNode = NewObject<UGridNode>();
					StartingNode->SetPositionInGrid(StartingPosition);
					//// 5.) Get the closest GridNode with a DoorPosition of the incoming Room that is not being used, based on distance
					//// 5.1.) Determine if the DoorPosition of the Node is already in use
					UGridNode* NearestIngoingNode = GetNearestNode(VertU, DoorNodesIngoingRoom);
					if (NearestIngoingNode != nullptr)
					{
						FVector EndingPosition = GetNodeBeforeDoor(NearestIngoingNode->GetDoor(), IngoingRoom);
						if (EndingPosition != FVector(FLT_MIN, FLT_MIN, FLT_MIN))
						{
							UGridNode* EndingNode = NewObject<UGridNode>();
							EndingNode->SetPositionInGrid(EndingPosition);

							// 6.) Start the Pathfinder from the two marked Rooms
							if (NearestOutgoingNode != nullptr && NearestIngoingNode != nullptr)
							{
								Pathfinder->FindPath(StartingNode, EndingNode, NearestOutgoingNode, NearestIngoingNode);
								ProcessPathData();
							}
							else
								UE_LOG(LogTemp, Warning, TEXT("No nodes for pathfinding found."));
							// 6.1.) Mark used DoorPositions as bDoorIsUsed, to prevent multiple usages per Door
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("Nearest ingoing node not valid"));
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Nearest outgoing node not valid."));
			}
		}
		// 7.) After the Path has been created, determine the fitting floor pieces for each PathNode 		
		// 8.) After generating, mark both doors as being used
	}
}

void AGridSpawner::ProcessPathData()
{
	TArray<UGridNode*> Path = CurrentPath.Path;

	if (Path.Num() > 0)
	{
		int32 Counter = 0;
		for (UGridNode* Node : Path)
		{
			UGridNode* Parent;
			UGridNode* Child;

			EPathPiece PathPiece = EPathPiece::MIDDLE_PIECE;

			if (Counter == 0)
				PathPiece = EPathPiece::STARTING_PIECE;

			if (Counter == Path.Num() - 1)
				PathPiece = EPathPiece::END_PIECE;

			switch (PathPiece)
			{
			case EPathPiece::STARTING_PIECE:
				Parent = CurrentPath.OutgoingNode;
				if (Path.Num() == 1)
					Child = CurrentPath.IngoingNode;
				else
					Child = Path[Counter + 1];
				break;
			case EPathPiece::END_PIECE:
				Child = CurrentPath.IngoingNode;
				if (Path.Num() == 1)
					Parent = CurrentPath.OutgoingNode;
				else
					Parent = Path[Counter - 1];
				break;
			default:
				Parent = Path[Counter - 1];
				Child = Path[Counter + 1];
				break;
			}

			NodesWithDoorsUsed.Add(CurrentPath.OutgoingNode);
			NodesWithDoorsUsed.Add(CurrentPath.IngoingNode);

			SpawnCorridorPiece(Node, Child, Parent, PathPiece);

			Counter++;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No path to trace left."));
	}
}

void AGridSpawner::SpawnCorridorPiece(UGridNode* PieceToSpawn, UGridNode* ChildNode, UGridNode* ParentNode, EPathPiece PathPiece)
{
	// Get current location and check if that location allready has a corridor peace generated
	FVector SpawnLocation = PieceToSpawn->GetPositionInGrid();

	int32 XPos = SpawnLocation.X / GridCellActorSize.X;
	int32 YPos = SpawnLocation.Y / GridCellActorSize.Y;
	int32 ZPos = SpawnLocation.Z / GridCellActorSize.Z;

	// Get the locations of parent and child.
	FVector ParentPos = ParentNode->GetPositionInGrid();
	FVector ChildPos = ChildNode->GetPositionInGrid();

	TSet<FVector> NeighbouringPositions;
	NeighbouringPositions.Add(ParentPos);
	NeighbouringPositions.Add(ChildPos);

	ECorridorPiece CorridorPiece = ECorridorPiece::I_PIECE;

	bool ActorAllreadyExists = false;

	FSpawnedCorridor* FoundPair = SpawnedCorridors.FindByPredicate([&](const FSpawnedCorridor& Pair)
	{
		return Pair.PositionInGrid->GetPositionInGrid() == PieceToSpawn->GetPositionInGrid();
	});

	if (FoundPair)
		ActorAllreadyExists = true;

	// if location has no corridor allready
	if (!ActorAllreadyExists)
	{
		// check if parent and child differ only on 1 axis, if so use a i-piece, if not use a l-piece
		bool bLPieceNeeded = DiffersOnMutlipleAxis(ChildPos, ParentPos, SpawnLocation);

		if (bLPieceNeeded)
			CorridorPiece = ECorridorPiece::L_PIECE;
		// define rotation of the room
		// Rotational calculations, especially important for crossections
		FRotator SpawnRotation = GetCorridorSpawnRotation(SpawnLocation, NeighbouringPositions, CorridorPiece);

		ARoomActor* SpawnedRoom = GetWorld()->SpawnActor<ARoomActor>(CorridorToSpawn, SpawnLocation, SpawnRotation);

		if (SpawnedRoom)
		{
			UChildActorComponent* ChildComp = SpawnedRoom->FindComponentByClass<UChildActorComponent>();

			if(ChildComp)
			{
				ChildComp->SetChildActorClass(Corridor_I_Piece);
				
				if (bLPieceNeeded)
					ChildComp->SetChildActorClass(Corridor_L_Piece);
			}
			else
				UE_LOG(LogTemp, Error, TEXT("No childactorcomponent found on corridor."));

			// set room and save into array, dont forget to set startinglocation
			GridArray[XPos][YPos][ZPos] = 3;
			SpawnedRoom->SetRoomSpaceNeeded(SpawnLocation, SpawnRotation);

			SpawnedCorridors.Add(FSpawnedCorridor(PieceToSpawn, SpawnedRoom));
		}
		else
			UE_LOG(LogTemp, Error, TEXT("Corridor could not be spawned."));
	}
	else
	{
		// if location has corridor allready
		UE_LOG(LogTemp, Display, TEXT("Another piece was allready at this place."));

		// get all orthogonal neighbours including parent and child
		for (FVector NeighbourPos : GetNeighbouringPositions(SpawnLocation))
		{
			NeighbouringPositions.Add(NeighbourPos);
			UE_LOG(LogTemp, Display, TEXT("Neighbour found at position X: %f, Y: %f, Z: %f"), NeighbourPos.X, NeighbourPos.Y, NeighbourPos.Z);
		}

		// if neighbours == 2, return
		if (NeighbouringPositions.Num() == 2)
			return;

		// Get the room at current location
		ARoomActor* CurrentRoom = nullptr;
		for (FSpawnedCorridor Corridor : SpawnedCorridors)
		{
			if (Corridor.PositionInGrid->GetPositionInGrid() == SpawnLocation)
			{
				CurrentRoom = Corridor.Corridor;
				break;
			}
		}

		if (CurrentRoom != nullptr)
		{
			UChildActorComponent* ChildComp = CurrentRoom->FindComponentByClass<UChildActorComponent>();
			if (ChildComp)
			{
				FRotator SpawnRotation = FRotator::ZeroRotator;
				// if neighbours == 3, use t-piece and define rotation based on neighbours locations
				if (NeighbouringPositions.Num() == 3)
				{
					CorridorPiece = ECorridorPiece::T_PIECE;
					SpawnRotation = GetCorridorSpawnRotation(SpawnLocation, NeighbouringPositions, CorridorPiece);
					ChildComp->SetChildActorClass(Corridor_T_Piece);
					CurrentRoom->SetActorRotation(SpawnRotation);
				}
				// if neighbours == 4, use x-piece, no rotational calculations needed, as it uses all directions simultaniously
				else if (NeighbouringPositions.Num() == 4)
				{
					CorridorPiece = ECorridorPiece::X_PIECE;
					SpawnRotation = GetCorridorSpawnRotation(SpawnLocation, NeighbouringPositions, CorridorPiece);
					ChildComp->SetChildActorClass(Corridor_X_Piece);
					CurrentRoom->SetActorRotation(SpawnRotation);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("No childcomponent found on room: %s"), *CurrentRoom->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No room found at corridor node position X: %f, Y: %f, Z: %f"), SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
		}
	}

}

TArray<FVector> AGridSpawner::GetNeighbouringPositions(FVector CurrentPos)
{
	TArray<FVector> ValidNeighbours;

	TArray<FVector> Offsets = {
		FVector(0.0f, GridCellActorSize.Y, 0.0f),
		FVector(0.0f, -GridCellActorSize.Y, 0.0f),
		FVector(-GridCellActorSize.X, 0.0f, 0.0f),
		FVector(GridCellActorSize.X, 0.0f, 0.0f),
	};

	for (const FVector& Offset : Offsets)
	{
		FVector NeighbouringPos = CurrentPos + Offset;

		int32 NodePosX = NeighbouringPos.X / GridCellActorSize.X;
		int32 NodePosY = NeighbouringPos.Y / GridCellActorSize.Y;
		int32 NodePosZ = NeighbouringPos.Z / GridCellActorSize.Z;

		FVector ConvertedGridPos = FVector(NodePosX, NodePosY, NodePosZ);

		if (!IsOutOfBounds(ConvertedGridPos))
		{
			int32 GridValue = GridArray[NodePosX][NodePosY][NodePosZ];

			if (GridValue == 3)
			{
				ValidNeighbours.Add(NeighbouringPos);
			}
			else if (GridValue == 1)
			{
				for (UGridNode* DoorNode : NodesWithDoorsUsed)
				{
					if (NeighbouringPos == DoorNode->GetPositionInGrid())
					{
						ValidNeighbours.Add(NeighbouringPos);
						UE_LOG(LogTemp, Display, TEXT("Added door-node to valid neighbours."));
					}
				}
			}
		}

	}

	return ValidNeighbours;
}

void AGridSpawner::DrawDelaunayEdges(const TArray<FDelaunayEdge>& Edges)
{
	UWorld* World = GetWorld();
	UE_LOG(LogTemp, Warning, TEXT("Amount of Edges Drawn: %d"), Edges.Num());
	for (const FDelaunayEdge& Edge : Edges)
	{
		DrawDebugLine(
			World,
			Edge.U.Position,
			Edge.V.Position,
			FColor::Green,
			true,
			-1.0f,
			0,
			1.0f
		);
	}

	//UE_LOG(LogTemp, Warning, TEXT("Amount of Triangles found in Delaunay: %d"), Delaunay->Triangles.Num());
	//UE_LOG(LogTemp, Warning, TEXT("Amount of Edges found in Delaunay: %d"), Delaunay->Edges.Num());
}

void AGridSpawner::DrawPrimEdges(const TArray<FPrimEdge>& Edges)
{
	UWorld* World = GetWorld();
	for (const FPrimEdge& Edge : Edges)
	{
		DrawDebugLine(
			World,
			Edge.U.Position,
			Edge.V.Position,
			FColor::Red,
			true,
			-1.0f,
			0,
			1.0f
		);
	}

	//UE_LOG(LogTemp, Warning, TEXT("Amount of Triangles found in Prim: %d"), Delaunay->Triangles.Num());
	//UE_LOG(LogTemp, Warning, TEXT("Amount of Edges found in Prim: %d"), Delaunay->Edges.Num());
}

void AGridSpawner::DrawAllEdges(const TSet<FPrimEdge>& Edges)
{
	UWorld* World = GetWorld();
	for (const FPrimEdge& Edge : Edges)
	{
		DrawDebugLine(
			World,
			Edge.U.Position,
			Edge.V.Position,
			FColor::Cyan,
			true,
			-1.0f,
			0,
			1.0f
		);
	}

	//UE_LOG(LogTemp, Warning, TEXT("Amount of Triangles found in Prim: %d"), Delaunay->Triangles.Num());
	//UE_LOG(LogTemp, Warning, TEXT("Amount of Edges found in Prim: %d"), Delaunay->Edges.Num());
}


#pragma endregion

#pragma region Getters and Setters


FVector AGridSpawner::GetGridSize()
{
	return GridSize;
}

TArray<TArray<TArray<int32>>> AGridSpawner::GetGridArray()
{
	return GridArray;
}

void AGridSpawner::SetCurrentPath(FPathData InCurrentPath)
{
	CurrentPath = InCurrentPath;
}

FPathData AGridSpawner::GetCurrentPath()
{
	return CurrentPath;
}


#pragma endregion



#pragma region << STEP_05: CLEANUP WORK AND FINISH >>

void AGridSpawner::DestroyAllGridCellsLeft()
{
	TArray<AGridActor*> CellPiecesLeft;
	for (AGridActor* GridActor : GridActors)
	{
		if (GridActor && GridActor->GetRoomType() == ERoomType::CellPiece)
		{
			CellPiecesLeft.Add(GridActor);
		}
	}

	for (AGridActor* CellPiece : CellPiecesLeft)
	{
		GridActors.Remove(CellPiece);
		CellPiece->Destroy();
	}
}

#pragma endregion


#pragma region << HELPER METHODS >>

bool AGridSpawner::IsOutOfBounds(FVector NodePosition)
{
	if (NodePosition.X < 0 || NodePosition.X >= GridSize.X || NodePosition.Y < 0 || NodePosition.Y >= GridSize.Y || NodePosition.Z < 0 || NodePosition.Z >= GridSize.Z)
	{
		return true;
	}

	return false;
}

void AGridSpawner::StartAfter(UObject* WorldContextObject, float InSeconds, TFunction<void()> CallbackFunction)
{
	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
		return;

	FTimerDelegate TimerCallback;
	TimerCallback.BindLambda(
		[CallbackFunction]()
		{
			CallbackFunction();
		});

	FTimerHandle Handle;
	World->GetTimerManager().SetTimer(Handle, TimerCallback, InSeconds, false);
}

bool AGridSpawner::SpaceIsOccupied()
{
	return false;
}

TArray<UGridNode*> AGridSpawner::GetNodesOfFloor(int32 Floor)
{
	return TArray<UGridNode*>();
}

void AGridSpawner::PrintVerticesByFloor()
{
	for (const TPair<int32, TMap<FVertex, ARoomActor*>>& Pair : VerticesByFloor)
	{
		// Log the key
		int32 Key = Pair.Key;
		UE_LOG(LogTemp, Warning, TEXT("Floor: %d"), Key);

		// Log the values in the TArray<FVertex>
		TArray<FVertex> VertexArray;
		Pair.Value.GenerateKeyArray(VertexArray);
		for (const FVertex& Vertex : VertexArray)
		{
			// Assuming FVertex has a member PositionInGrid of type FVector
			UE_LOG(LogTemp, Warning, TEXT("Value: PositionInGrid: %s"), *Vertex.Position.ToString());
			// Log other members of FVertex if needed
		}
	}
}

void AGridSpawner::PrintVerticesByFloor(int32 Floor)
{
	TArray<FVertex> VertexArray;
	VerticesByFloor[Floor].GenerateKeyArray(VertexArray);
	for (const FVertex& Vertex : VertexArray)
	{
		UE_LOG(LogTemp, Display, TEXT("Value: PositionInGrid: %s"), *Vertex.Position.ToString());
	}
}

// INTEGRATE FLOOR CHECK
TArray<UGridNode*> AGridSpawner::GetDoorNodesByRooms(ARoomActor* InRoom, int32 Floor)
{
	TArray<UGridNode*> NodesWithDoors;

	if (InRoom && InRoom->DoorPositions.Num() > 0)
	{
		for (UDoorPosition* Door : InRoom->DoorPositions)
		{
			FVector ConvDoorPos = InRoom->ConvertDoorNodePosition(Door);
			Door->SetParentNode(InRoom, ConvDoorPos);
		}

		int32 DoorNodes = 0;
		for (UGridNode* Node : InRoom->GetRoomSpaceNeeded().CoordinatesInGrid)
		{
			for (UDoorPosition* Door : InRoom->DoorPositions)
			{
				if (Door->GetParentNode() != nullptr)
				{
					if (Node->GetPositionInGrid().Z == Floor)
					{
						if (Node->GetPositionInGrid() == Door->GetParentNode()->GetPositionInGrid())
						{
							//UE_LOG(LogTemp, Display, TEXT("Coordinates match!"));
							DoorNodes++;
							NodesWithDoors.Add(Node);
						}
					}
					else
						UE_LOG(LogTemp, Warning, TEXT("Node: %s was on floor: %f, needed to be on floor: %d"), *Node->GetName(), Node->GetPositionInGrid().Z, Floor);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Doors parent is null."));
				}
			}
		}
		//UE_LOG(LogTemp, Warning, TEXT("Room: %s has %d Doors in it."), *InRoom->GetName(), DoorNodes);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No doors found."));
	}

	return NodesWithDoors;
}

UGridNode* AGridSpawner::GetNearestNode(const FVertex& DistantNode, TPair<ARoomActor*, TArray<UGridNode*>> NodesToCheck)
{
	UGridNode* NearestNode = nullptr;
	float ClosestDistance = FLT_MAX;

	if (NodesToCheck.Value.Num() <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No nodes to check the distance to."));
		return NearestNode;
	}

	//UE_LOG(LogTemp, Display, TEXT("Amount of noded to check: %d"), NodesToCheck.Value.Num());

	for (UGridNode* Node : NodesToCheck.Value)
	{
		//UE_LOG(LogTemp, Display, TEXT("Checking distance to node..."));
		float DistanceToDistantNode = FVector::Dist(Node->GetPositionInGrid(), DistantNode.Position);
		if (DistanceToDistantNode < ClosestDistance)
		{
			if (Node->GetDoor())
			{
				UDoorPosition* Door = Node->GetDoor();
				// DER OUT OF BOUNDS CHECK FÜR DIE NODE VOR DER TÜR GEHÖRT HIER REIN
				if (GetNodeBeforeDoor(Door, NodesToCheck.Key) != FVector(FLT_MIN, FLT_MIN, FLT_MIN))
				{
					ClosestDistance = DistanceToDistantNode;
					NearestNode = Node;
					//UE_LOG(LogTemp, Display, TEXT("New nearest distance found."));
				}
				else
					UE_LOG(LogTemp, Warning, TEXT("Door cant be used, because it is pointing out of bounds."));
			}
			else
				UE_LOG(LogTemp, Warning, TEXT("No door set on node."));
		}
	}

	if (NearestNode == nullptr)
		UE_LOG(LogTemp, Warning, TEXT("No node left to use."));

	return NearestNode;
}

FVector AGridSpawner::GetNodeBeforeDoor(UDoorPosition* Door, ARoomActor* Room, FVector OptDoorPosition)
{
	if (Door->GetParentNode() || OptDoorPosition != FVector::ZeroVector)
	{
		ESpawnRotation RoomRotation = Room->GetRoomSpaceNeeded().SpawnRotation;
		FVector NodePosition;

		if (OptDoorPosition != FVector::ZeroVector)
			NodePosition = OptDoorPosition;
		else
			NodePosition = Door->GetParentNode()->GetPositionInGrid();

		ESpawnRotation DoorRotation = Door->GetDoorData().SpawnRotation;

		FVector Offset = FVector::ZeroVector;

		switch (RoomRotation)
		{
		case ESpawnRotation::SOUTH:
			switch (DoorRotation)
			{
			case ESpawnRotation::SOUTH:
				Offset = FVector(GridCellActorSize.X, 0.0f, 0.0f);
				break;
			case ESpawnRotation::EAST:
				Offset = FVector(0.0f, -GridCellActorSize.X, 0.0f);
				break;
			case ESpawnRotation::WEST:
				Offset = FVector(0.0f, GridCellActorSize.X, 0.0f);
				break;
			default:
				Offset = FVector(-GridCellActorSize.X, 0.0f, 0.0f);
				break;
			}
			break;
		case ESpawnRotation::EAST:
			switch (DoorRotation)
			{
			case ESpawnRotation::SOUTH:
				Offset = FVector(0.0f, -GridCellActorSize.X, 0.0f);
				break;
			case ESpawnRotation::EAST:
				Offset = FVector(-GridCellActorSize.X, 0.0f, 0.0f);
				break;
			case ESpawnRotation::WEST:
				Offset = FVector(GridCellActorSize.X, 0.0f, 0.0f);
				break;
			default:
				Offset = FVector(0.0f, GridCellActorSize.X, 0.0f);
				break;
			}
			break;
		case ESpawnRotation::WEST:
			switch (DoorRotation)
			{
			case ESpawnRotation::SOUTH:
				Offset = FVector(0.0f, GridCellActorSize.X, 0.0f);
				break;
			case ESpawnRotation::EAST:
				Offset = FVector(GridCellActorSize.X, 0.0f, 0.0f);
				break;
			case ESpawnRotation::WEST:
				Offset = FVector(-GridCellActorSize.X, 0.0f, 0.0f);
				break;
			default:
				Offset = FVector(0.0f, -GridCellActorSize.X, 0.0f);
				break;
			}
			break;
		default:
			switch (DoorRotation)
			{
			case ESpawnRotation::SOUTH:
				Offset = FVector(-GridCellActorSize.X, 0.0f, 0.0f);
				break;
			case ESpawnRotation::EAST:
				Offset = FVector(0.0f, GridCellActorSize.X, 0.0f);
				break;
			case ESpawnRotation::WEST:
				Offset = FVector(0.0f, -GridCellActorSize.X, 0.0f);
				break;
			default:
				Offset = FVector(GridCellActorSize.X, 0.0f, 0.0f);
				break;
			}
			break;
		}

		FVector SpawnPosition = NodePosition + Offset;

		FVector CheckedForGrid = FVector(SpawnPosition.X / GridCellActorSize.X, SpawnPosition.Y / GridCellActorSize.Y, SpawnPosition.Z / GridCellActorSize.Z);

		if (!IsOutOfBounds(CheckedForGrid))
		{
			// convert position into grid-coordinates
			int32 XPos = FMath::RoundToInt(SpawnPosition.X / GridCellActorSize.X);
			int32 YPos = FMath::RoundToInt(SpawnPosition.Y / GridCellActorSize.Y);
			int32 ZPos = FMath::RoundToInt(SpawnPosition.Z / GridCellActorSize.Z);

			if (GridArray[XPos][YPos][ZPos] == 0)
			{
				UE_LOG(LogTemp, Display, TEXT("Door facing node can be used."));
				return SpawnPosition;
			}
			else
				UE_LOG(LogTemp, Warning, TEXT("Door facing node cant be used, since the space is allready occupied."));
		}
		else
			UE_LOG(LogTemp, Warning, TEXT("Door facing node with position X: %f, Y: %f, Z: %f is out of bounds."), SpawnPosition.X, SpawnPosition.Y, SpawnPosition.Z);
	}
	else
		UE_LOG(LogTemp, Display, TEXT("No parent node set."));
	
	return FVector(FLT_MIN, FLT_MIN, FLT_MIN);
	
	// LATER: open crosssection if blocked and crosssection available
	// -> if not, close door with wall mesh
}

bool AGridSpawner::DoorsHaveEnoughSpace(ARoomActor* RoomToCheck)
{
	TArray<UDoorPosition*> DoorPositions = RoomToCheck->GetAllDoors();

	for (UDoorPosition* Door : DoorPositions)
	{
		if (DoorPositions.Num() > 0)
		{
			UE_LOG(LogTemp, Display, TEXT("Room %s has %d doors in it."), *RoomToCheck->GetName(), DoorPositions.Num());

			FVector DoorPositionInGrid = RoomToCheck->ConvertDoorNodePosition(Door);

			FVector GridSpaceToCheck = GetNodeBeforeDoor(Door, RoomToCheck, DoorPositionInGrid);
			if (GridSpaceToCheck == FVector(FLT_MIN, FLT_MIN, FLT_MIN))
				return false;

			int32 X = FMath::RoundToInt(GridSpaceToCheck.X / GridCellActorSize.X);
			int32 Y = FMath::RoundToInt(GridSpaceToCheck.Y / GridCellActorSize.Y);
			int32 Z = FMath::RoundToInt(GridSpaceToCheck.Z / GridCellActorSize.Z);

			if (GridArray[X][Y][Z] != 0)
			{
				if (IsOutOfBounds(GridSpaceToCheck))
				{
					UE_LOG(LogTemp, Warning, TEXT("Node before door was out of bounds, changing room location."));
					return false;
				}

				UE_LOG(LogTemp, Warning, TEXT("Node before door was allready occupied, changing room location."));
				return false;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Room has no doors in it."));
			return false;
		}
	}

	return true;
}

bool AGridSpawner::DiffersOnMutlipleAxis(FVector ChildPos, FVector ParentPos, FVector CurrentPos)
{
	FVector PrevToCurrent = CurrentPos - ParentPos;
	FVector CurrentToNext = ChildPos - CurrentPos;

	bool MutliAxisDiffer = false;

	if (FMath::Abs(PrevToCurrent.X) > 0 && FMath::Abs(PrevToCurrent.Y) == 0 && FMath::Abs(CurrentToNext.X) > 0 && FMath::Abs(CurrentToNext.Y) == 0)
	{
		MutliAxisDiffer = false;
	}
	else if (FMath::Abs(PrevToCurrent.X) == 0 && FMath::Abs(PrevToCurrent.Y) > 0 && FMath::Abs(CurrentToNext.X) == 0 && FMath::Abs(CurrentToNext.Y) > 0)
	{
		MutliAxisDiffer = false;
	}
	else if ((PrevToCurrent.X != 0 && CurrentToNext.Y != 0) || (PrevToCurrent.Y != 0 && CurrentToNext.X != 0))
	{
		MutliAxisDiffer = true;
	}

	return MutliAxisDiffer;
}

FRotator AGridSpawner::GetCorridorSpawnRotation(FVector PiecePos, TSet<FVector> NeighbourPositions, ECorridorPiece CorridorPiece)
{
	FRotator Rotation = FRotator::ZeroRotator;

	TArray<FVector> Directions;
	for (const FVector& NeighbourPos : NeighbourPositions)
	{
		FVector Direction = NeighbourPos - PiecePos;
		Direction.Normalize();
		Directions.Add(Direction);
	}

	switch (CorridorPiece)
	{
	case ECorridorPiece::I_PIECE:
		if (Directions.Contains(FVector(1, 0, 0)) || Directions.Contains(FVector(-1, 0, 0)))
		{
			Rotation = FRotator(0, 0, 0);
		}
		else if (Directions.Contains(FVector(0, 1, 0)) || Directions.Contains(FVector(0, -1, 0)))
		{
			Rotation = FRotator(0, 90, 0);
		}
		else if (Directions.Contains(FVector(-1, 0, 0)) && Directions.Contains(FVector(0, -1, 0)))
		{
			Rotation = FRotator(0, 180, 0);
		}
		else if (Directions.Contains(FVector(0, -1, 0)) && Directions.Contains(FVector(1, 0, 0)))
		{
			Rotation = FRotator(0, 270, 0);
		}
		break;

	case ECorridorPiece::L_PIECE:
		if (Directions.Contains(FVector(0, 1, 0)) && Directions.Contains(FVector(-1, 0, 0)))
		{
			Rotation = FRotator(0, 0, 0);
		}
		else if (Directions.Contains(FVector(-1, 0, 0)) && Directions.Contains(FVector(0, -1, 0)))
		{
			Rotation = FRotator(0, 90, 0);
		}
		else if (Directions.Contains(FVector(0, -1, 0)) && Directions.Contains(FVector(1, 0, 0)))
		{
			Rotation = FRotator(0, 180, 0);
		}
		else if (Directions.Contains(FVector(1, 0, 0)) && Directions.Contains(FVector(0, 1, 0)))
		{
			Rotation = FRotator(0, 270, 0);
		}
		break;

	case ECorridorPiece::T_PIECE:
		if (!Directions.Contains(FVector(0, -1, 0)))
		{
			Rotation = FRotator(0, 0, 0);
		}
		else if (!Directions.Contains(FVector(1, 0, 0)))
		{
			Rotation = FRotator(0, 90, 0);
		}
		else if (!Directions.Contains(FVector(0, 1, 0)))
		{
			Rotation = FRotator(0, 180, 0);
		}
		else if (!Directions.Contains(FVector(-1, 0, 0))) 
		{
			Rotation = FRotator(0, 270, 0);
		}
		break;

	case ECorridorPiece::X_PIECE:
		Rotation = FRotator(0, 0, 0);
		break;

	default:
		break;
	}

	return Rotation;
}


FVector AGridSpawner::GetGridCellActorSize()
{
	return GridCellActorSize;
}

#pragma endregion
