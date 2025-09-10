// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SquadFactory.generated.h"

struct FSquadCommandPayload;
class USquadCommand;
class USquadMember;
class USquadLeader;

/**
 * 
 */
UCLASS()
class SUPERDEFENCE_API USquadFactory : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "SquadFactory|Commands")
	static USquadCommand* GetCommand(UObject* WorldContextObject, const FName CommandName);
	
	UFUNCTION(BlueprintCallable, Category = "SquadFactory|Leader")
	static USquadLeader* CreateSquadLeader(AActor* Outer);

	UFUNCTION(BlueprintCallable, Category = "SquadFactory|Member", meta = (ToolTip = "This should only be called on an UObject that implemented the ICommandableInterface!"))
	static USquadMember* CreateSquadMember(UObject* CommandableObject, USquadLeader* Leader);

	UFUNCTION(BlueprintCallable, Category = "SquadFactory|Commands")
	static void RegisterCommands();
	
	UFUNCTION(BlueprintCallable, Category = "SquadFactory|Commands")
	static void ClearCommandMap();
	
private:
	static TMap<FName, USquadCommand*> CommandMap;
};
