// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class AGridSpawner;
class UGridNode;

class SPOOPYGAME_API Pathfinding
{

	AGridSpawner* Grid;

public:

	Pathfinding();

	void Initialize(AGridSpawner* InGrid);

	void FindPath(UGridNode* StartingNode, UGridNode* TargetNode, UGridNode* OutgoingNode, UGridNode* IngoingNode);

	void RetracePath(UGridNode* StartingNode, UGridNode* TargetNode, UGridNode* OutgoingNode, UGridNode* IngoingNode);

	int32 GetDistance(UGridNode* NodeA, UGridNode* NodeB);

	// Debugging
private:

	void PrintPathData(TArray<UGridNode*> InPath);

	TArray<UGridNode*> GetNeighbours(UGridNode* Node, UGridNode* TargetNode, TArray<UGridNode*> OpenSet, TSet<UGridNode*> ClosedSet);

	int Partition(TArray<UGridNode*>& OpenSet, int low, int high);

	void QuickSort(TArray<UGridNode*>& OpenSet, int low, int high);

	void SortOpenSet(TArray<UGridNode*>& Openset);

};
