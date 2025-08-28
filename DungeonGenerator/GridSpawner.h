// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Delaunay/Delaunay.h"
#include "GridSpawner.generated.h"

class UGridNode;
class ARoomActor;
class ARoomActor;
class UDoorPosition;

UENUM()
enum class EBuildingStage : uint8
{
	GENERATE_GRID = 0,
	PLACE_ROOMS = 1,
	ROOM_CHECKS = 2,
	PLACE_STAIRS = 3,
	CONNECT_ROOMS = 4,
	DONE = 5
};

UENUM()
enum class EPathPiece : uint8
{
	STARTING_PIECE = 0,
	MIDDLE_PIECE = 1,
	END_PIECE = 2
};

UENUM()
enum class ECorridorPiece : uint8
{
	I_PIECE = 0,
	L_PIECE = 1,
	T_PIECE = 2,
	X_PIECE = 3
};

USTRUCT()
struct FFloorDetails
{
	GENERATED_USTRUCT_BODY();

	int32 HighestFloor;

	int32 LowestFloor;

	TMap<int32, bool> FloorHasStairs;

};

USTRUCT()
struct FPathData
{
	GENERATED_USTRUCT_BODY();

	TArray<UGridNode*> Path;

	UGridNode* OutgoingNode;

	UGridNode* IngoingNode;

};

USTRUCT()
struct FSpawnedCorridor
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY()
	UGridNode* PositionInGrid;

	UPROPERTY()
	ARoomActor* Corridor;

	FSpawnedCorridor()
		: PositionInGrid(nullptr), Corridor(nullptr)
	{}

	FSpawnedCorridor(UGridNode* InPositionInGrid, ARoomActor* InCorridor)
		: PositionInGrid(InPositionInGrid), Corridor(InCorridor)
	{}
};

UCLASS()
class SPOOPYGAME_API AGridSpawner : public AActor
{
	GENERATED_BODY()

	///// <<< Grid Initialisation and Contents variables >>> /////

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Initialisation | Grid", meta = (AllowPrivateAccess = "true"))
	FVector GridSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Initialisation | Grid", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AGridActor> GridCellActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Initialisation | Grid", meta = (AllowPrivateAccess = "true"))
	FVector GridCellActorSize = FVector(100, 100, 50);

	TArray<TArray<TArray<int32>>> GridArray;

	TArray<AGridActor*> GridActors;

	///// <<< Room Specific variables >>> /////

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Initialisation | Rooms", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<ARoomActor>> PossibleRoomsToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Initialisation | Rooms", meta = (AllowPrivateAccess = "true"))
	int32 AmountOfRoomsToSpawn;

	TArray<AGridActor*> SpawnedRooms;

	///// <<< Stairs Specific variables >>> /////

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Initialisation | Stairs", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<ARoomActor>> PossibleStairsToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Initialisation | Stairs", meta = (AllowPrivateAccess = "true"))
	int32 MaxAmountOfStairsPerFloor = 1;

	FFloorDetails FloorDetails;

	///// <<< Delaunay Specific variables >>> /////

	UDelaunay* Delaunay;

	///// <<< A* Specific variables >>> /////

	TMap<int32, TArray<ARoomActor*>> RoomsByFloors;

	TMap<int32, TMap<FVertex, ARoomActor*>> VerticesByFloor;

	UPROPERTY()
	TSet<UGridNode*> NodesWithDoorsUsed;

	FPathData CurrentPath;

	UPROPERTY(EditAnywhere, Category = "Grid Initialisation | Corridors", meta = (AllowPrivateAccess = "true"))
	float PathSpawnPercentage = 12.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Initialisation | Corridors", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ARoomActor> CorridorToSpawn;

	UPROPERTY()
	TArray<FSpawnedCorridor> SpawnedCorridors;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Initialisation | Corridors | Pieces", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> Corridor_I_Piece;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Initialisation | Corridors | Pieces", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> Corridor_L_Piece;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Initialisation | Corridors | Pieces", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> Corridor_T_Piece;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Initialisation | Corridors | Pieces", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> Corridor_X_Piece;

	///// <<< Toggle Variables >>> /////

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Initialisation | Toggle", meta = (AllowPrivateAccess = "true"))
	bool DestroyGridCellsAfterGeneration = false;

#if WITH_EDITORONLY_DATA
	///// <<< Debug Variables >>> /////
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DEBUG | Timer", meta = (AllowPrivateAccess = "true"))
	float GridInitTimer = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DEBUG | Timer", meta = (AllowPrivateAccess = "true"))
	float RoomSpawnTimer = 4.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DEBUG | Timer", meta = (AllowPrivateAccess = "true"))
	float StairSpawnTimer = 6.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DEBUG | Timer", meta = (AllowPrivateAccess = "true"))
	float FloorSpawnTimer = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DEBUG | Flags", meta = (AllowPrivateAccess = "true"))
	bool bDrawDelaunayEdges = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DEBUG | Timer", meta = (AllowPrivateAccess = "true"))
	float DelaunayEdgesDrawTimer = 3.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DEBUG | Timer", meta = (AllowPrivateAccess = "true"))
	float DelaunayEdgesDrawDuration = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DEBUG | Flags", meta = (AllowPrivateAccess = "true"))
	bool bDrawPrimEdges = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DEBUG | Timer", meta = (AllowPrivateAccess = "true"))
	float PrimEdgesDrawTimer = 3.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DEBUG | Timer", meta = (AllowPrivateAccess = "true"))
	float PrimEdgesDrawDuration = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DEBUG | Flags", meta = (AllowPrivateAccess = "true"))
	bool bDrawAllEdges = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DEBUG | Timer", meta = (AllowPrivateAccess = "true"))
	float AllEdgesDrawTimer = 3.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DEBUG | Timer", meta = (AllowPrivateAccess = "true"))
	float AllEdgesDrawDuration = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DEBUG | Timer", meta = (AllowPrivateAccess = "true"))
	float CorridorSpawnTimer = 3.f;
#endif

public:	
	AGridSpawner();

	FVector GetGridSize();

	TArray<TArray<TArray<int32>>> GetGridArray();

	void SetCurrentPath(FPathData InCurrentPath);

	FPathData GetCurrentPath();

protected:
	///// <<< Grid Initialisation >>> /////

	virtual void BeginPlay() override;

	void BeginBuildingSteps();

	///// <<< STEP 1 Spawning the Grid >>> /////

	void TryBuildingGrid();

	void CreateGridArray();

	void GenerateGrid();

	///// <<< STEP 2 Placing the Rooms >>> /////

	void TryPlacingRooms();

	bool CouldSpawnRoom();

	bool SpawnRoom(FVector InSpawnLocation);

	void SetNewRoomData(AGridActor* CurrentRoom, int32 Floor = INT_MIN);

	FVector GetNewRandomLocation(int32 Floor = INT_MIN);

	FRotator GetRandomRotation();

	///// <<< STEP 3 Placing the Stairs >>> /////

	void SetFloorDetails();

	void TryPlacingStairs();

	void LogFloorDetails();

	///// <<< STEP 4 Connecting the Rooms >>> /////

	void TryConnectingRooms();

	void Triangulate();

	void CreateConnections(TArray<FDelaunayEdge> InEdges, TMap<FVertex, ARoomActor*> RoomsByVertices);
	
	TSet<FPrimEdge> GenerateLoops(TArray<FPrimEdge> AllEdges, TSet<FPrimEdge> SelectedEdges);

	void SpawnCorridors(TSet<FPrimEdge> LoopedEdges, TMap<FVertex, ARoomActor*> RoomsByVertices);

	void ProcessPathData();

	void SpawnCorridorPiece(UGridNode* PieceToSpawn, UGridNode* ChildNode, UGridNode* ParentNode, EPathPiece PathPiece);

	TArray<FVector> GetNeighbouringPositions(FVector CurrentPos);

	///// <<< Helper Methods >>> /////

	void DestroyAllGridCellsLeft();

	void DrawDelaunayEdges(const TArray<FDelaunayEdge>& Edges);

	void DrawPrimEdges(const TArray<FPrimEdge>& Edges);

	void DrawAllEdges(const TSet<FPrimEdge>& Edges);

	bool SpaceIsOccupied();

	TArray<UGridNode*> GetNodesOfFloor(int32 Floor);

	void PrintVerticesByFloor();

	void PrintVerticesByFloor(int32 Floor);

	TArray<UGridNode*> GetDoorNodesByRooms(ARoomActor* InRoom, int32 Floor);

	UGridNode* GetNearestNode(const FVertex& DistantNode, TPair<ARoomActor*, TArray<UGridNode*>> NodesToCheck);

	FVector GetNodeBeforeDoor(UDoorPosition* Door, ARoomActor* Room, FVector OptDoorPosition = FVector::ZeroVector);

	bool DoorsHaveEnoughSpace(ARoomActor* RoomToCheck);

	bool DiffersOnMutlipleAxis(FVector ChildPos, FVector ParentPos, FVector CurrentPos);

	FRotator GetCorridorSpawnRotation(FVector PiecePos, TSet<FVector> NeighbourPositions, ECorridorPiece CorridorPiece);

public:

	//TArray<UGridNode*> GetValidNeighbours(UGridNode* Node, UGridNode* TargetNode);

	FVector GetGridCellActorSize();

	bool IsOutOfBounds(FVector NodePosition);

private:
	static void StartAfter(UObject* WorldContextObject, float InSeconds, TFunction<void()> CallbackFunction);
};

template<typename T>
static void DrawEdges(UObject* WorldContextObject, const TArray<T>& Edges, FColor Color, float DrawDuration)
{
	UWorld* World = WorldContextObject->GetWorld();
	UE_LOG(LogTemp, Warning, TEXT("Amount of Edges Drawn: %d"), Edges.Num());
	for (const auto& Edge : Edges)
	{
		DrawDebugLine(
			World,
			Edge.U.Position,
			Edge.V.Position,
			Color,
			false,
			DrawDuration,
			0,
			1.0f
		);
	}
};