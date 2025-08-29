// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/ItemActor.h"

#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Inventory/InventoryItemInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SphereComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "ActionGameCharacter.h"
#include "ActorComponents/InventoryComponent.h"

// Sets default values
AItemActor::AItemActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(RootComponent);
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AItemActor::OnSphereOverlap);
}

void AItemActor::OnEquipped()
{
	ItemState = EItemState::Equipped;

	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereComponent->SetGenerateOverlapEvents(false);
}

void AItemActor::OnUnequipped()
{
	ItemState = EItemState::None;

	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereComponent->SetGenerateOverlapEvents(false);
}

void AItemActor::OnDropped()
{
	ItemState = EItemState::Dropped;

	GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	if (AActor* ActorOwner = GetOwner())
	{
		const FVector Location = GetActorLocation();
		const FVector Forward = ActorOwner->GetActorForwardVector();

		const float droppItemDist = 100.f;
		const float droppItemTraceDist = 1000.f;

		const FVector TraceStart = Location + Forward * droppItemDist;
		const FVector TraceEnd = TraceStart - FVector::UpVector * droppItemTraceDist;

		TArray<AActor*> ActorsToIgnore = { ActorOwner };
		FHitResult TraceHit;

		static const auto CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("ShowDebugInventory"));
		const bool bShowInventory = CVar->GetInt() > 0;

		FVector TargetLocation = TraceEnd;

		EDrawDebugTrace::Type DebugDrawType = bShowInventory ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
		if (UKismetSystemLibrary::LineTraceSingleByProfile(this, TraceStart, TraceEnd, TEXT("WorldStatic"), true, ActorsToIgnore, DebugDrawType, TraceHit, true))
		{
			if (TraceHit.bBlockingHit)
			{
				TargetLocation = TraceHit.Location;
			}
		}

		SetActorLocation(TargetLocation);

		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SphereComponent->SetGenerateOverlapEvents(true);
	}
}

bool AItemActor::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	WroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);

	return WroteSomething;
}

void AItemActor::Init(UInventoryItemInstance* InItemInstance)
{
	ItemInstance = InItemInstance;
}

// Called when the game starts or when spawned
void AItemActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		if (!IsValid(ItemInstance) && IsValid(ItemStaticDataClass))
		{
			ItemInstance = NewObject<UInventoryItemInstance>();
			ItemInstance->Init(ItemStaticDataClass);

			SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			SphereComponent->SetGenerateOverlapEvents(true);
		}
	}
}

void AItemActor::OnRep_ItemState()
{
	switch (ItemState)
	{
	case EItemState::Equipped:
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComponent->SetGenerateOverlapEvents(false);
		break;
	default:
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SphereComponent->SetGenerateOverlapEvents(true);
		break;
	}
}

void AItemActor::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		if (const AActionGameCharacter* Character = Cast<AActionGameCharacter>(OtherActor))
		{
			if (const UInventoryComponent* InventoryComponent = Character->GetInventoryComponent())
			{
				if (InventoryComponent->GetInventoryList().GetItemsRef().Num() >= 5)
					return;
			}
		}
		
		FGameplayEventData EventPayload;
		EventPayload.Instigator = this;
		EventPayload.OptionalObject = ItemInstance;
		EventPayload.EventTag = UInventoryComponent::EquipItemActorTag;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OtherActor, UInventoryComponent::EquipItemActorTag, EventPayload);
	}
}

// Called every frame
void AItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AItemActor, ItemInstance);
	DOREPLIFETIME(AItemActor, ItemState);
}