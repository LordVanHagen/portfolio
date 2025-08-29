// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Graphs.generated.h"

class AGridActor;

USTRUCT(BlueprintType)
struct FVertex
{
	GENERATED_USTRUCT_BODY();

public:

	UPROPERTY(BlueprintReadWrite)
	FVector Position;

	FVertex() {}

	FVertex(const FVector& InPosition)
		: Position(InPosition)
	{}

	friend bool operator==(const FVertex& Left, const FVertex& Right)
	{
		return Left.Position == Right.Position;
	}

	bool operator!=(const FVertex& Other)
	{
		return !(Position == Other.Position);
	}

	friend uint32 GetTypeHash(const FVertex& Vertex)
	{
		return GetTypeHash(Vertex.Position);
	}

};

USTRUCT(BlueprintType)
struct FPrimEdge
{
	GENERATED_USTRUCT_BODY();
	
public:

	UPROPERTY(BlueprintReadWrite)
	FVertex U;

	UPROPERTY(BlueprintReadWrite)
	FVertex V;

	UPROPERTY(BlueprintReadWrite)
	float Distance;

	FPrimEdge() {}

	FPrimEdge(const FVertex& InU, const FVertex& InV)
		: U(InU), V(InV)
	{
		Distance = FVector::Distance(InU.Position, InV.Position);
	}

	friend bool operator==(const FPrimEdge& Left, const FPrimEdge& Right)
	{
		return (Left.U == Right.U && Left.V == Right.V)
			|| (Left.U == Right.V && Left.V == Right.U);
	}

	friend bool operator!=(const FPrimEdge& Left, const FPrimEdge& Right)
	{
		return !(Left == Right);
	}

	bool Equals(const FPrimEdge& Other)
	{
		return *this == Other;
	}

	friend uint32 GetTypeHash(const FPrimEdge& Edge)
	{
		return HashCombine(GetTypeHash(Edge.U), GetTypeHash(Edge.V));
	}

};
