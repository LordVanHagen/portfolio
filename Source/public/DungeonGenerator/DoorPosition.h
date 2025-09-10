// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Core.h"
#include "Components/SceneComponent.h"
#include "..//SpoopyGameTypes.h"
#include "GridActor.h"
#include "DoorPosition.generated.h"

UENUM(BlueprintType)
enum class EDoorOrientation : uint8
{
	Ingoing UMETA(DisplayName = "Ingoing"),
	Outgoing UMETA(DisplayName = "Outgoing")
};

USTRUCT(BlueprintType)
struct FDoorData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door Information", meta = (AllowPrivateAccess = "true"))
	FVector UsedGridInRoom = FVector(0.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door Information", meta = (AllowPrivateAccess = "true"))
	ESpawnRotation SpawnRotation = ESpawnRotation::NORTH;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door Information", meta = (AllowPrivateAccess = "true"))
	EDoorOrientation DoorOrientation = EDoorOrientation::Outgoing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door Information", meta = (AllowPrivateAccess = "true"))
	bool IsOptional = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door Information", meta = (AllowPrivateAccess = "true"))
	bool IsBlocked = false;
};


UCLASS(Blueprintable)
class SPOOPYGAME_API UDoorPosition : public USceneComponent
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door Data", meta = (AllowPrivateAccess = "true"))
	FDoorData DoorData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Data", meta = (AllowPrivateAccess = "true"))
	FRotator DoorRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Data", meta = (AllowPrivateAccess = "true"))
	bool bDoorIsUsed = false;


public:	

	// Sets default values for this component's properties
	UDoorPosition();

	UGridNode* ParentNode;

	FDoorData GetDoorData();

	UGridNode* GetParentNode();

	UFUNCTION(BlueprintCallable, Category="Door Data")
	void SetParentNode(AGridActor* InParent, FVector InDoorLocation);

	void SetDoorIsUsed(bool InDoorIsUsed);

	bool GetDoorIsUsed();

};
