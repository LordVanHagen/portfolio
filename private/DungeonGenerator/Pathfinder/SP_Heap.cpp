// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/Pathfinder/SP_Heap.h"
#include "..//GridActor.h"

SP_Heap::SP_Heap(int32 MaxHeapSize)
{
	Items.SetNumZeroed(MaxHeapSize);
	CurrentItemCount = 0;

	UE_LOG(LogTemp, Display, TEXT("New heap created with max heapsize: %d"), Items.Num());
}

void SP_Heap::Add(UGridNode* Item)
{
	Item->SetHeapIndex(CurrentItemCount);
	Items[CurrentItemCount] = Item;
	SortUp(Item);
	CurrentItemCount++;

	UE_LOG(LogTemp, Display, TEXT("New item: %s with was added to heap with index: %d"), *Item->GetName(), Item->GetHeapIndex());
}

UGridNode* SP_Heap::RemoveFirst()
{
	UGridNode* FirstItem = Items[0];
	CurrentItemCount--;
	Items[0] = Items[CurrentItemCount];
	Items[0]->SetHeapIndex(0);
	SortDown(Items[0]);
	return FirstItem;
}

void SP_Heap::UpdateItem(UGridNode* Item)
{
	SortUp(Item);
}

int32 SP_Heap::Count()
{
	return CurrentItemCount;
}

bool SP_Heap::Contains(UGridNode* Item)
{
	int32 Index = Item->GetHeapIndex();

	UE_LOG(LogTemp, Display, TEXT("Checking contains for item: %s with heapindex: %d"), *Item->GetName(), Item->GetHeapIndex());

	if (Index >= 0 && Index <= CurrentItemCount && Items[Index] != nullptr)
	{
		if (Items[Index]->Equals(*Item))
		{
			UE_LOG(LogTemp, Error, TEXT("Item %s with heapindex = %d equals the item of the heapindex: %s with heapindex: %d."), *Item->GetName(), Item->GetHeapIndex(), *Items[Item->GetHeapIndex()]->GetName(), Items[Item->GetHeapIndex()]->GetHeapIndex());
		}

		return Items[Item->GetHeapIndex()]->Equals(*Item);
	}

	return false;
}

void SP_Heap::SortDown(UGridNode* Item)
{
	while (true)
	{
		int32 ChildIndexLeft = Item->GetHeapIndex() * 2 + 1;
		int32 ChildIndexRight = Item->GetHeapIndex() * 2 + 2;
		int32 SwapIndex = 0;

		if (ChildIndexLeft < CurrentItemCount)
		{
			SwapIndex = ChildIndexLeft;

			if (ChildIndexRight < CurrentItemCount)
			{
				if (Items[ChildIndexLeft]->CompareTo(*Items[ChildIndexRight]) < 0)
				{
					SwapIndex = ChildIndexRight;
				}
			}

			if (Item->CompareTo(*Items[SwapIndex]) < 0)
			{
				Swap(Item, Items[SwapIndex]);
			}
			else
			{
				return;
			}
		}
		else
		{
			return;
		}
	}
}

void SP_Heap::SortUp(UGridNode* Item)
{
	int32 ParentIndex = (Item->GetHeapIndex() - 1) / 2;

	while (true)
	{
		UGridNode* ParentItem = Items[ParentIndex];
		if (Item->CompareTo(*ParentItem) > 0)
		{
			Swap(Item, ParentItem);
		}
		else
		{
			break;
		}

		ParentIndex = (Item->GetHeapIndex() - 1) / 2;
	}
}

void SP_Heap::Swap(UGridNode* ItemA, UGridNode* ItemB)
{
	Items[ItemA->GetHeapIndex()] = ItemB;
	Items[ItemB->GetHeapIndex()] = ItemA;
	int32 ItemAIndex = ItemA->GetHeapIndex();
	ItemA->SetHeapIndex(ItemB->GetHeapIndex());
	ItemB->SetHeapIndex(ItemAIndex);
}
