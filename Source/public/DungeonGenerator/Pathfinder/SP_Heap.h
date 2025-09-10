// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../GridActor.h"
#include "SP_Heap.generated.h"

class UGridNode;

UINTERFACE()
class UHeapIndex : public UInterface
{
	GENERATED_BODY()
};
class SPOOPYGAME_API IHeapIndex
{
	GENERATED_BODY()

	int32 HeapIndex;

public:

	virtual int32 GetHeapIndex() const = 0;
	virtual void SetHeapIndex(int32 Index) = 0;
	virtual int32 CompareTo(const UGridNode* Other) const = 0;

};

class SPOOPYGAME_API SP_Heap
{
	TArray<UGridNode*> Items;
	int32 CurrentItemCount;

public:
	SP_Heap(int32 MaxHeapSize);

	void Add(UGridNode* Item);

	UGridNode* RemoveFirst();

	void UpdateItem(UGridNode* Item);

	int32 Count();

	bool Contains(UGridNode* Item);

	void SortDown(UGridNode* Item);

	void SortUp(UGridNode* Item);

	void Swap(UGridNode* ItemA, UGridNode* ItemB);

};
