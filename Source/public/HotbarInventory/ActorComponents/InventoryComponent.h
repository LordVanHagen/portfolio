// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/InventoryList.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "../Public/UI/WidgetController/InventoryWidgetController.h"
#include "InventoryComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryChangedSignature, FInventoryItemsPayload);


UCLASS(BlueprintType, Blueprintable)
class ACTIONGAME_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	virtual void InitializeComponent() override;

	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	UFUNCTION(BlueprintCallable)
	void AddItem(TSubclassOf<UItemStaticData> InItemStaticDataClass);

	UFUNCTION(BlueprintCallable)
	void AddItemInstance(UInventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable)
	void RemoveItem(TSubclassOf<UItemStaticData> InItemStaticDataClass);

	UFUNCTION(BlueprintCallable)
	void RemoveItemInstance(UInventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable)
	void EquipItem(TSubclassOf<UItemStaticData> InItemStaticDataClass);

	UFUNCTION(BlueprintCallable)
	void EquipItemInstance(UInventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable)
	void UnequipItem();

	UFUNCTION(BlueprintCallable)
	void DropItem();

	UFUNCTION(BlueprintCallable)
	void EquipNext();

	UFUNCTION(BlueprintCallable)
	void EquipPrevious();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UInventoryItemInstance* GetEquippedItem() const;

	UFUNCTION()
	FORCEINLINE FInventoryList GetInventoryList() const { return InventoryList; }

	UFUNCTION() int32 GetCurrentItemIndex();

	virtual void GameplayEventCallback(const FGameplayEventData* Payload);

	static FGameplayTag EquipItemActorTag;
	static FGameplayTag	UnequipItemTag;
	static FGameplayTag	DropItemTag;
	static FGameplayTag EquipNextTag;
	static FGameplayTag EquipPreviousTag;

	FOnInventoryChangedSignature OnInventoryChanged;

	void UpdateInventoryUI();

protected:

	UFUNCTION()
	void AddInventoryTags();

	UPROPERTY(ReplicatedUsing = OnRep_InventoryList)
	FInventoryList InventoryList;

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UItemStaticData>> DefaultItems;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentItem)
	UInventoryItemInstance* CurrentItem = nullptr;

	FDelegateHandle TagDelegateHandle;

	void HandleGameplayEventInternal(FGameplayEventData Payload);

	UFUNCTION(Server, Reliable)
	void Server_HandleGameplayEvent(FGameplayEventData Payload);

	UFUNCTION()
	void OnRep_InventoryList();

	UFUNCTION()
	void OnRep_CurrentItem();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
