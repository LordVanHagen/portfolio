// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "ActionGameTypes.h"
#include "InventoryList.generated.h"


class UInventoryItemInstance;

USTRUCT(BlueprintType)
struct FInventoryListItem : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadOnly)
	UInventoryItemInstance* ItemInstance = nullptr;

	FORCEINLINE bool operator==(const FInventoryListItem& Other) const { return ItemInstance == Other.ItemInstance; }
	FORCEINLINE bool operator!=(const FInventoryListItem& Other) const { return !(*this == Other); }
};

USTRUCT(BlueprintType)
struct FInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryListItem, FInventoryList>(Items, DeltaParams, *this);
	}

	void AddItem(TSubclassOf<UItemStaticData> InItemStaticDataClass);
	void AddItem(UInventoryItemInstance* InItemInstance);

	void RemoveItem(TSubclassOf<UItemStaticData> InItemStaticDataClass);
	void RemoveItemInstance(UInventoryItemInstance* InItemInstance);

	TArray<FInventoryListItem>& GetItemsRef() { return Items; }

protected:

	UPROPERTY(BlueprintReadOnly)
	TArray<FInventoryListItem> Items;
};

template<>
struct TStructOpsTypeTraits<FInventoryList> : public TStructOpsTypeTraitsBase2<FInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};