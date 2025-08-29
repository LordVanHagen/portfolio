// Header-Datei (.h)
#pragma once

#include "CoreMinimal.h"
#include "DungeonGenerator/GridActor.h"
#include "Delaunay/Graphs.h"
#include "RoomActor.generated.h"

class UDoorPosition;

UCLASS()
class SPOOPYGAME_API ARoomActor : public AGridActor
{
    GENERATED_BODY()

    int AmountOfNonOptionalDoors;

    TMap<int32, FVertex> Vertices;

    UPROPERTY()
    bool bCOMWasGenerated = false;

public:
    ARoomActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Information", meta = (AllowPrivateAccess = "true"))
    TArray<UDoorPosition*> DoorPositions;

    UPROPERTY()
    bool bIsSet = false;

    UFUNCTION(BlueprintNativeEvent)
    void CalculateCenterOfMass();

    UFUNCTION(BlueprintNativeEvent)
    void SpawnDoorsInRoom();

    UFUNCTION(BlueprintCallable)
    FVector GetNodePositionBasedOnRotation(FVector InUsedGridInRoom, ESpawnRotation InSpawnRotation);

    FVector ConvertDoorNodePosition(UDoorPosition* InDoorPosition);
    
    // Only temporary for debugging purposes
    int GetAmountOfNonOptionalDoors();

    bool GetCOMWasGenerated();

    void SetCOMWasGenerated(bool Condition);

    UFUNCTION(BlueprintImplementableEvent)
    TArray<UDoorPosition*> GetAllDoors();

    UFUNCTION(BlueprintCallable)
    void SetVertexLocation(FVector VertexLocation, int32 Floor);

    TMap<int32, FVertex> GetAllVertices() const;

    FVertex* GetVertexByFloor(int32 Floor);

    /*UFUNCTION(BlueprintCallable)
    void SpawnRoomsBasedOnDoorPositions();*/

    UFUNCTION(BlueprintCallable)
    FVector GetLocationBasedOnRotation(UDoorPosition* InDoorPosition);

protected:

    UFUNCTION(BlueprintCallable)
    void AddDoorPosition(UDoorPosition* DoorPositionToAdd);

    /*UFUNCTION(BlueprintCallable)
    FVector CalculateLocationInGridActor(UDoorPosition* InDoorPosition);*/
    
};