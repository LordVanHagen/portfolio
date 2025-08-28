// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/Delaunay/Delaunay.h"

#include "Graphs.h"

UDelaunay* UDelaunay::Triangulate(const TArray<FVertex>& InVertices, int32 Floor, float StandardRoomSizeZ)
{
	UDelaunay* Delaunay = NewObject<UDelaunay>();
	Delaunay->Vertices = InVertices;

    if (Delaunay->Vertices.Num() == 2)
    {
	    Delaunay->UseSingleEdge();
        return Delaunay;
    }
    else if (Delaunay->Vertices.Num() == 3)
    {
        Delaunay->UseSingleTriangle(Floor, StandardRoomSizeZ);
        return Delaunay;
    }

    Delaunay->Triangulate(Floor, StandardRoomSizeZ);

	return Delaunay;
}

void UDelaunay::UseSingleEdge()
{
    UE_LOG(LogTemp, Warning, TEXT("Single Edge added."));
    FDelaunayEdge Edge = FDelaunayEdge(Vertices[0], Vertices[1]);
    Edges.Add(Edge);
}

void UDelaunay::UseSingleTriangle(int32 Floor, float StandardRoomSizeZ)
{
    UE_LOG(LogTemp, Warning, TEXT("Single Triangle added."));

    FVertex V1 = Vertices[0];
    FVertex V2 = Vertices[1];
    FVertex V3 = Vertices[2];

    FDelaunayEdge Edge1 = FDelaunayEdge(V1, V2);
    FDelaunayEdge Edge2 = FDelaunayEdge(V2, V3);
    FDelaunayEdge Edge3 = FDelaunayEdge(V3, V1);

    float VertexHeight = (Floor * StandardRoomSizeZ) + (StandardRoomSizeZ / 2);

    FTriangle Triangle = FTriangle(V1, V2, V3, VertexHeight);
    Triangles.Add(Triangle);

    Edges.Add(Edge1);
    Edges.Add(Edge2);
    Edges.Add(Edge3);

}

void UDelaunay::Triangulate(int32 Floor, float StandardRoomSizeZ)
{
    UE_LOG(LogTemp, Warning, TEXT("Triangulation started."));
    // Init min and max values
    float MinX = Vertices[0].Position.X;
    float MinY = Vertices[0].Position.Y;
    float MaxX = MinX;
    float MaxY = MinY;

    // Finding the min and max values
    for (const FVertex& Vertex : Vertices)
    {
        if (Vertex.Position.X < MinX) MinX = Vertex.Position.X;
        if (Vertex.Position.X > MaxX) MaxX = Vertex.Position.X;
        if (Vertex.Position.Y < MinY) MinY = Vertex.Position.Y;
        if (Vertex.Position.Y > MaxY) MaxY = Vertex.Position.Y;
    }

    float DeltaX = MaxX - MinX;
    float DeltaY = MaxY - MinY;
    float DeltaMax = FMath::Max(DeltaX, DeltaY) * 2;

    float VertexHeight = (Floor * StandardRoomSizeZ) + (StandardRoomSizeZ / 2);

    FVertex P1 = FVertex(FVector(MinX - 1, MinY - 1, VertexHeight));
    FVertex P2 = FVertex(FVector(MinX - 1, MaxY + DeltaMax, VertexHeight));
    FVertex P3 = FVertex(FVector(MaxX + DeltaMax, MinY - 1, VertexHeight));

    Triangles.Add(FTriangle(P1, P2, P3, VertexHeight));

    for (const FVertex& Vertex : Vertices)
    {
        TArray<FDelaunayEdge> Polygon;

        for (FTriangle& Triangle : Triangles)
        {
            if (Triangle.CircumCircleContains(Vertex.Position))
            {
                Triangle.bIsBad = true;
                Polygon.Add(FDelaunayEdge(Triangle.A, Triangle.B));
                Polygon.Add(FDelaunayEdge(Triangle.B, Triangle.C));
                Polygon.Add(FDelaunayEdge(Triangle.C, Triangle.A));
            }
        }

        // Remove all triangles marked with "IsBad"
        Triangles.RemoveAll([](const FTriangle& Triangle)
            {
                return Triangle.bIsBad;
            });

        // Remove all edges marked with "IsBad" from the Polygon
        Polygon.RemoveAll([](const FDelaunayEdge& Edge)
            {
                return Edge.bIsBad;
            });

        for (int i = 0; i < Polygon.Num(); i++)
        {
            for (int j = i + 1; j < Polygon.Num(); j++)
            {
                if (FDelaunayEdge::AlmostEqual(Polygon[i], Polygon[j]))
                {
                    Polygon[i].bIsBad = true;
                    Polygon[j].bIsBad = true;
                }
            }
        }

        // Remove all edges marked with "IsBad" from the Polygon
        Polygon.RemoveAll([](const FDelaunayEdge& Edge)
            {
                return Edge.bIsBad;
            });

        for (const FDelaunayEdge& Edge : Polygon)
        {
            Triangles.Add(FTriangle(Edge.U, Edge.V, Vertex, VertexHeight));
        }
    }

    // Remove all triangles that have a point that was part of the initial super-triangle
    Triangles.RemoveAll([&](const FTriangle& Triangle)
        {
            return Triangle.ContainsVertex(P1.Position) || Triangle.ContainsVertex(P2.Position) || Triangle.ContainsVertex(P3.Position);
        });

    TSet<FDelaunayEdge> EdgeSet;

    for (const FTriangle& Triangle : Triangles)
    {
        // Creating the Edges AB, BC and CA for the current triangle
        FDelaunayEdge AB = FDelaunayEdge(Triangle.A, Triangle.B);
        FDelaunayEdge BC = FDelaunayEdge(Triangle.B, Triangle.C);
        FDelaunayEdge CA = FDelaunayEdge(Triangle.C, Triangle.A);

        // Addin the Edge AB, if not allready part of the EdgeSet
        if (!EdgeSet.Contains(AB))
        {
            EdgeSet.Add(AB);
            Edges.Add(AB);
        }

        // Addin the Edge BC, if not allready part of the EdgeSet
        if (!EdgeSet.Contains(BC))
        {
            EdgeSet.Add(BC);
            Edges.Add(BC);
        }

        // Addin the Edge CA, if not allready part of the EdgeSet
        if (!EdgeSet.Contains(CA))
        {
            EdgeSet.Add(CA);
            Edges.Add(CA);
        }
    }
}