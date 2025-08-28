// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Graphs.h"
#include "SpoopyGameStatics.h"
#include "Delaunay.generated.h"

USTRUCT()
struct FTriangle
{
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY()
    FVertex A;

    UPROPERTY()
    FVertex B;

    UPROPERTY()
    FVertex C;

    UPROPERTY()
    bool bIsBad = false;

    UPROPERTY()
    float VertexHeight;

    FTriangle() {}

    FTriangle(const FVertex& InA, const FVertex& InB, const FVertex& InC, float InVertexHeight)
        : A(InA), B(InB), C(InC), VertexHeight(InVertexHeight)
    {}

    bool ContainsVertex(const FVector& InV) const
    {
        return FVector::DistSquared(InV, A.Position) < 0.01f
            || FVector::DistSquared(InV, B.Position) < 0.01f
            || FVector::DistSquared(InV, C.Position) < 0.01f;
    }

    bool CircumCircleContains(const FVector& InV) const
    {
        FVector AVector = A.Position;
        FVector BVector = B.Position;
        FVector CVector = C.Position;

        float AB = AVector.SizeSquared();
        float CD = BVector.SizeSquared();
        float EF = CVector.SizeSquared();

        float CircumX = (AB * (CVector.Y - BVector.Y) + CD * (AVector.Y - CVector.Y) + EF * (BVector.Y - AVector.Y))
            / (AVector.X * (CVector.Y - BVector.Y) + BVector.X * (AVector.Y - CVector.Y) + CVector.X * (BVector.Y - AVector.Y));
        float CircumY = (AB * (CVector.X - BVector.X) + CD * (AVector.X - CVector.X) + EF * (BVector.X - AVector.X))
            / (AVector.Y * (CVector.X - BVector.X) + BVector.Y * (AVector.X - CVector.X) + CVector.Y * (BVector.X - AVector.X));



        FVector Circum = FVector(CircumX / 2, CircumY / 2, VertexHeight);

        float CircumRadius = FVector::DistSquared(AVector, Circum);
        float Dist = FVector::DistSquared(InV, Circum);
        return Dist <= CircumRadius;
    }

    friend bool operator==(const FTriangle& Left, const FTriangle& Right)
    {
        return (Left.A == Right.A || Left.A == Right.B || Left.A == Right.C)
            && (Left.B == Right.A || Left.B == Right.B || Left.B == Right.C)
            && (Left.C == Right.A || Left.C == Right.B || Left.C == Right.C);
    }

    friend bool operator!=(const FTriangle& Left, const FTriangle& Right)
    {
        return !(Left == Right);
    }

    bool Equals(const FTriangle& Other)
    {
        return *this == Other;
    }

    friend uint32 GetTypeHash(const FTriangle& Triangle)
    {
        return HashCombine(HashCombine(GetTypeHash(Triangle.A), GetTypeHash(Triangle.B)), GetTypeHash(Triangle.C));
    }

};

USTRUCT()
struct FDelaunayEdge
{
    GENERATED_USTRUCT_BODY();

public:

    FVertex U;

    FVertex V;

    bool bIsBad = false;

    FDelaunayEdge() {}

    FDelaunayEdge(const FVertex& InU, const FVertex& InV)
        : U(InU), V(InV)
    {}

    friend bool operator==(const FDelaunayEdge& Left, const FDelaunayEdge& Right)
    {
        return (Left.U == Right.U || Left.U == Right.V)
            && (Left.V == Right.U || Left.V == Right.V);
    }

    friend bool operator!=(const FDelaunayEdge& Left, const FDelaunayEdge& Right)
    {
        return !(Left == Right);
    }

    bool Equals(const FDelaunayEdge& Other)
    {
        return *this == Other;
    }

    friend uint32 GetTypeHash(const FDelaunayEdge& Edge)
    {
        return HashCombine(GetTypeHash(Edge.U), GetTypeHash(Edge.V));
    }

    static bool AlmostEqual(const FDelaunayEdge& Left, const FDelaunayEdge Right)
    {
        return USpoopyGameStatics::AlmostEqual(Left.U, Right.U) && USpoopyGameStatics::AlmostEqual(Left.V, Right.V)
            || USpoopyGameStatics::AlmostEqual(Left.U, Right.V) && USpoopyGameStatics::AlmostEqual(Left.V, Right.U);
    }

};

UCLASS()
class SPOOPYGAME_API UDelaunay : public UObject
{
	GENERATED_BODY()

public:

    TArray<FVertex> Vertices;
    TArray<FDelaunayEdge> Edges;
    TArray<FTriangle> Triangles;

    UDelaunay() {}

    static UDelaunay* Triangulate(const TArray<FVertex>& InVertices, int32 Floor, float StandardRoomSizeZ);

private:

    void UseSingleEdge();

    void UseSingleTriangle(int32 Floor, float StandardRoomSizeZ);

    void Triangulate(int32 Floor, float StandardRoomSizeZ);
	
};
