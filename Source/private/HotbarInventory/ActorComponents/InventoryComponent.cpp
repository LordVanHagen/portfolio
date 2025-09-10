// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

#include "Inventory/InventoryItemInstance.h"
#include "Inventory/InventoryList.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

#include "GameplayTagsManager.h"

#include "DrawDebugHelpers.h"

FGameplayTag UInventoryComponent::EquipItemActorTag;
FGameplayTag UInventoryComponent::UnequipItemTag;
FGameplayTag UInventoryComponent::DropItemTag;
FGameplayTag UInventoryComponent::EquipNextTag;
FGameplayTag UInventoryComponent::EquipPreviousTag;

static TAutoConsoleVariable<int32> CVarShowInventory{
	TEXT("ShowDebugInventory"),
	0,
	TEXT("Draws debug info about inventory")
	TEXT(" 0: off/n")
	TEXT(" 1: on/n"),
	ECVF_Cheat
};

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);

	// small check to see if tags have allready been added and if so, dont allow them to be added again
	static bool bHandledAddingTags = false;
	{
		if (!bHandledAddingTags)
		{
			bHandledAddingTags = true;
			UGameplayTagsManager::Get().OnLastChanceToAddNativeTags().AddUObject(this, &UInventoryComponent::AddInventoryTags);
		}
	}
}

void UInventoryComponent::AddInventoryTags()
{
	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();

	UInventoryComponent::EquipItemActorTag = TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.EquipItemActor"), TEXT("Equip item from item actor event"));
	UInventoryComponent::UnequipItemTag = TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.UnequipItem"), TEXT("Unequip current item"));
	UInventoryComponent::DropItemTag = TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.DropItem"), TEXT("Drop equipped item"));
	UInventoryComponent::EquipNextTag = TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.EquipNext"), TEXT("Try equip next item"));
	UInventoryComponent::EquipPreviousTag = TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.EquipPrevious"), TEXT("Try equip previous item"));
	TagsManager.OnLastChanceToAddNativeTags().RemoveAll(this);
}


void UInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (GetOwner()->HasAuthority())
	{
		for (auto ItemClass : DefaultItems)
		{
			InventoryList.AddItem(ItemClass);
		}
	}

	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
	{
		ASC->GenericGameplayEventCallbacks.FindOrAdd(UInventoryComponent::EquipItemActorTag).AddUObject(this, &UInventoryComponent::GameplayEventCallback);
		ASC->GenericGameplayEventCallbacks.FindOrAdd(UInventoryComponent::UnequipItemTag).AddUObject(this, &UInventoryComponent::GameplayEventCallback);
		ASC->GenericGameplayEventCallbacks.FindOrAdd(UInventoryComponent::DropItemTag).AddUObject(this, &UInventoryComponent::GameplayEventCallback);
		ASC->GenericGameplayEventCallbacks.FindOrAdd(UInventoryComponent::EquipNextTag).AddUObject(this, &UInventoryComponent::GameplayEventCallback);
		ASC->GenericGameplayEventCallbacks.FindOrAdd(UInventoryComponent::EquipPreviousTag).AddUObject(this, &UInventoryComponent::GameplayEventCallback);
	}
}

bool UInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FInventoryListItem& Item : InventoryList.GetItemsRef())
	{
		UInventoryItemInstance* ItemInstance = Item.ItemInstance;

		if (IsValid(ItemInstance))
		{
			WroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

void UInventoryComponent::AddItem(TSubclassOf<UItemStaticData> InItemStaticDataClass)
{
	if (GetOwner()->HasAuthority())
	{
		InventoryList.AddItem(InItemStaticDataClass);
	}
}

void UInventoryComponent::AddItemInstance(UInventoryItemInstance* InItemInstance)
{
	if (GetOwner()->HasAuthority())
	{
		InventoryList.AddItem(InItemInstance);
	}
}

void UInventoryComponent::RemoveItem(TSubclassOf<UItemStaticData> InItemStaticDataClass)
{
	if (GetOwner()->HasAuthority())
	{
		InventoryList.RemoveItem(InItemStaticDataClass);
	}
}

void UInventoryComponent::RemoveItemInstance(UInventoryItemInstance* InItemInstance)
{
	if (GetOwner()->HasAuthority())
	{
		InventoryList.RemoveItemInstance(InItemInstance);
	}
}

void UInventoryComponent::EquipItem(TSubclassOf<UItemStaticData> InItemStaticDataClass)
{
	if (GetOwner()->HasAuthority())
	{
		for (auto Item : InventoryList.GetItemsRef())
		{
			if (Item.ItemInstance->ItemStaticDataClass == InItemStaticDataClass)
			{
				Item.ItemInstance->OnEquipped(GetOwner());

				CurrentItem = Item.ItemInstance;

				break;
			}
		}
	}
}

void UInventoryComponent::EquipItemInstance(UInventoryItemInstance* InItemInstance)
{
	if (GetOwner()->HasAuthority())
	{
		for (auto Item : InventoryList.GetItemsRef())
		{
			if (Item.ItemInstance == InItemInstance)
			{
				Item.ItemInstance->OnEquipped(GetOwner());
				CurrentItem = Item.ItemInstance;
				break;
			}
		}
	}
}

void UInventoryComponent::UnequipItem()
{
	if (GetOwner()->HasAuthority())
	{
		if (IsValid(CurrentItem))
		{
			CurrentItem->OnUnequipped();

			CurrentItem = nullptr;
		}
	}
}

void UInventoryComponent::DropItem()
{
	if (GetOwner()->HasAuthority())
	{
		if (IsValid(CurrentItem))
		{
			UInventoryItemInstance* ToDrop = CurrentItem;

			ToDrop->OnDropped();
			RemoveItemInstance(ToDrop);

			CurrentItem = nullptr;
		}
	}
}

void UInventoryComponent::EquipNext()
{
	TArray<FInventoryListItem>& Items = InventoryList.GetItemsRef();

	const int32 Num = Items.Num();
	if (Num == 0) return;
	if (Num == 1 && CurrentItem) return;

	int32 StartIndex = 0;
	int32 CurrentIndex = INDEX_NONE;
	for (int32 i = 0; i < Num; ++i)
	{
		if (Items[i].ItemInstance == CurrentItem)
		{
			CurrentIndex = i;
			break;
		}
	}

	StartIndex = (CurrentIndex == INDEX_NONE) ? 0 : (CurrentIndex + 1) % Num;

	int32 Tried = 0;
	int32 Index = StartIndex;
	UInventoryItemInstance* TargetItem = nullptr;

	while (Tried < Num)
	{
		const FInventoryListItem& Candidate = Items[Index];
		if (Candidate.ItemInstance && Candidate.ItemInstance->GetItemStaticData())
		{
			TargetItem = Candidate.ItemInstance;
			break;
		}
		Index = (Index + 1) % Num;
		++Tried;
	}

	if (!TargetItem || TargetItem == CurrentItem) return;

	UnequipItem();
	EquipItemInstance(TargetItem);
}


void UInventoryComponent::EquipPrevious()
{
	TArray<FInventoryListItem>& Items = InventoryList.GetItemsRef();

	const bool bNoItems = Items.Num() == 0;
	const bool bOneAndEquipped = Items.Num() == 1 && CurrentItem;

	if (bNoItems || bOneAndEquipped) return;

	UInventoryItemInstance* TargetItem = nullptr;
	int32 CurrentItemIndex = INDEX_NONE;

	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (Items[i].ItemInstance == CurrentItem)
		{
			CurrentItemIndex = i;
			break;
		}
	}

	if (CurrentItemIndex != INDEX_NONE)
	{
		const int32 PrevIndex = (CurrentItemIndex - 1 + Items.Num()) % Items.Num();

		if (Items.IsValidIndex(PrevIndex))
		{
			TargetItem = Items[PrevIndex].ItemInstance;
		}
	}
	else
	{
		TargetItem = Items.Last().ItemInstance;
	}

	if (TargetItem == nullptr || TargetItem == CurrentItem)
	{
		return;
	}

	UnequipItem();
	EquipItemInstance(TargetItem);
}


UInventoryItemInstance* UInventoryComponent::GetEquippedItem() const
{
	return CurrentItem;
}


int32 UInventoryComponent::GetCurrentItemIndex()
{
	int32 Index = -1;
	if (!IsValid(CurrentItem))
		return Index;
	
	TArray<FInventoryListItem>& Items = InventoryList.GetItemsRef();

	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (IsValid(Items[i].ItemInstance) && Items[i].ItemInstance == CurrentItem)
			Index = i;
	}

	return Index;
}

void UInventoryComponent::GameplayEventCallback(const FGameplayEventData* Payload)
{
	ENetRole NetRole = GetOwnerRole();

	if (NetRole == ROLE_Authority)
	{
		HandleGameplayEventInternal(*Payload);
	}
	else if (NetRole == ROLE_AutonomousProxy)
	{
		Server_HandleGameplayEvent(*Payload);
	}
}


void UInventoryComponent::HandleGameplayEventInternal(FGameplayEventData Payload)
{
	ENetRole NetRole = GetOwnerRole();
	if (NetRole == ROLE_Authority)
	{
		FGameplayTag EventTag = Payload.EventTag;
		if (EventTag == UInventoryComponent::EquipItemActorTag)
		{
			if (const UInventoryItemInstance* ItemInstance = Cast<UInventoryItemInstance>(Payload.OptionalObject))
			{
				AddItemInstance(const_cast<UInventoryItemInstance*>(ItemInstance));

				if (Payload.Instigator)
				{
					Cast<AActor>(Payload.Instigator)->Destroy();
				}
			}
		}
		else if (EventTag == UInventoryComponent::UnequipItemTag)
		{
			UnequipItem();
		}
		else if (EventTag == UInventoryComponent::DropItemTag)
		{
			DropItem();
		}
		else if (EventTag == UInventoryComponent::EquipNextTag)
		{
			EquipNext();
		}
		else if (EventTag == UInventoryComponent::EquipPreviousTag)
		{
			EquipPrevious();
		}

		UpdateInventoryUI();
	}
}

void UInventoryComponent::Server_HandleGameplayEvent_Implementation(FGameplayEventData Payload)
{
	HandleGameplayEventInternal(Payload);
}

void UInventoryComponent::UpdateInventoryUI()
{
	FInventoryItemsPayload NewestPayload;
	NewestPayload.InventoryList = InventoryList;
	NewestPayload.CurrentItemIndex = GetCurrentItemIndex();
	OnInventoryChanged.Broadcast(NewestPayload);
}

void UInventoryComponent::OnRep_InventoryList()
{
	UpdateInventoryUI();
}

void UInventoryComponent::OnRep_CurrentItem()
{
	UpdateInventoryUI();
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const bool bShowDebug = CVarShowInventory.GetValueOnGameThread() != 0;
	if (bShowDebug)
	{
		for (FInventoryListItem& Item : InventoryList.GetItemsRef())
		{
			UInventoryItemInstance* ItemInstance = Item.ItemInstance;
			if (!IsValid(ItemInstance))
				continue;

			const UItemStaticData* ItemStaticData = ItemInstance->GetItemStaticData();
			if (!IsValid(ItemStaticData))
				continue;

			if (IsValid(ItemInstance) && IsValid(ItemStaticData))
			{
				GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Blue,FString::Printf(TEXT("Item %s"), *ItemStaticData->Name.ToString()));
			}
		}
	}
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UInventoryComponent, InventoryList);
	DOREPLIFETIME(UInventoryComponent, CurrentItem);
}