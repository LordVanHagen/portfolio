// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../SpoopyGameTypes.h"
#include "DoorActor.generated.h"

UCLASS()
class SPOOPYGAME_API ADoorActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door Information", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* DoorMesh;
	
public:	
	// Sets default values for this actor's properties
	ADoorActor();

};
