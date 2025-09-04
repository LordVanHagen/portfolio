// Fill out your copyright notice in the Description page of Project Settings.


#include "Squads/DEBUG/SquadTestEnemy.h"

#include "Components/CapsuleComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Squads/MovementHelper.h"
#include "Squads/Wrappers/SquadMember.h"
#include "SUPERDEFENCE/SUPERDEFENCE.h"

ASquadTestEnemy::ASquadTestEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	RootComponent = CapsuleComponent;
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SkeletalMesh->SetupAttachment(RootComponent);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(RootComponent);
	StaticMesh->SetVisibility(false, true);
	
	StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("StimuliSourceComponent");
    StimuliSourceComponent->bAutoRegister = true;
    StimuliSourceComponent->PrimaryComponentTick.bCanEverTick = false;
    StimuliSourceComponent->PrimaryComponentTick.bStartWithTickEnabled = false;
}

void ASquadTestEnemy::BeginPlay()
{
	Super::BeginPlay();

	PrimaryActorTick.TickInterval = TickInterval;

	if (!HasAuthority())
		PrimaryActorTick.bCanEverTick = false;
}

void ASquadTestEnemy::GoTo(const FSquadCommandPayload& SquadCommandPayload)
{
	SetAndValidateSquadCommandData(SquadCommandPayload);
	PreparingToStartCommand(true, true);
}

void ASquadTestEnemy::Attack(const FSquadCommandPayload& SquadCommandPayload)
{
	SetAndValidateSquadCommandData(SquadCommandPayload);
	PreparingToStartCommand(false, true);
}

void ASquadTestEnemy::Swarm(const FSquadCommandPayload& SquadCommandPayload)
{
	SetAndValidateSquadCommandData(SquadCommandPayload);
	PreparingToStartCommand(true, true);
}

void ASquadTestEnemy::Flee(const FSquadCommandPayload& SquadCommandPayload)
{
	SetAndValidateSquadCommandData(SquadCommandPayload);
	PreparingToStartCommand(true, true);
}

void ASquadTestEnemy::FollowPath()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("FollowPath Command called on %s"), *this->GetName()));
}

void ASquadTestEnemy::JumpDown()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("JumpDown Command called on %s"), *this->GetName()));
}

void ASquadTestEnemy::SetMemberWrapper(USquadMember* InMemberWrapper)
{
	if (IsValid(InMemberWrapper))
		MemberWrapper = InMemberWrapper;
}

void ASquadTestEnemy::Die()
{
	if (HasAuthority())
	{
		if (MemberWrapper)
			MemberWrapper->Die();
		
		bIsDead = true;
		
		UnregisterAllStimuli();
		Multicast_HandleDeath();
	}
}

bool ASquadTestEnemy::IsDead()
{
	return bIsDead;
}

void ASquadTestEnemy::ReceiveDamage(const FGameplayEffectSpecHandle& DamageEffectSpecHandle)
{
	if (HasAuthority())
	{
		const float ReceivedDamage = USquadDamageHelper::GetDamage(Stats, DamageEffectSpecHandle);
		Stats.CurrentHealth -= ReceivedDamage;
		if (Stats.CurrentHealth <= 0 && !bIsDead)
			Die();
	}
}

FVector ASquadTestEnemy::GetCombatSocketLocation()
{
	if (AimSocketName != NAME_None)
		return SkeletalMesh->GetSocketLocation(AimSocketName);

	UE_LOG(LogTemp, Warning, TEXT("GetCombatSocketLocation returned NULL"));
	return FVector::ZeroVector;
}

FVector ASquadTestEnemy::GetTargetVelocity() const
{
	return Velocity;
}

const FSquadMemberPositionInfo* ASquadTestEnemy::GetPositionInfo(const int32& InSquadID)
{
	if (PositionInfo.IsInitialized())
		return &PositionInfo;

	PositionInfo.CurrentPosition = GetActorLocation();
	PositionInfo.CurrentRotation = GetActorRotation();
	PositionInfo.MoveSpeed = Stats.MaxMovementSpeed;
	PositionInfo.TargetAcceptanceRadius = ArrivalTolerance;
	PositionInfo.SquadID = InSquadID;
	PositionInfo.SetIsInitialized(true);

	return &PositionInfo;

	// REMEMBER TO SET bIsInit TO FALSE BEFORE CALCULATING A NEW PATH
}

void ASquadTestEnemy::Move(const FComputedLocationInfo& ComputedLocation)
{
	if (ComputedLocation.HasMoved())
	{
		PositionInfo.CurrentPosition = ComputedLocation.Location;
		PositionInfo.CurrentRotation = ComputedLocation.Rotation;
		SetActorLocationAndRotation(ComputedLocation.Location, ComputedLocation.Rotation);
	}
}

void ASquadTestEnemy::SetVisibilitySetting(const EVisibilitySetting InVisibility)
{
	switch (InVisibility)
	{
	case SkeletalMeshSetting:
		SkeletalMesh->SetVisibility(true, true);
		SkeletalMesh->SetComponentTickEnabled(true);
		SkeletalMesh->bPauseAnims = false;
		SkeletalMesh->bNoSkeletonUpdate = false;
		StaticMesh->SetVisibility(false, true);
		break;
	case StaticMeshSetting:
		SkeletalMesh->SetVisibility(false, true);
		SkeletalMesh->SetComponentTickEnabled(false);
		SkeletalMesh->bPauseAnims = true;
		SkeletalMesh->bNoSkeletonUpdate = true;
		StaticMesh->SetVisibility(true, true);
		break;
	case NiagaraSystemSetting:
		//SkeletalMesh->SetVisibility(false);
		//StaticMesh->SetVisibility(false);
		UE_LOG(LogTemp, Warning, TEXT("NiagaraSystemSetting for VisibilitySetting not implemented yet."));
		break;
	}
}

void ASquadTestEnemy::InitStats()
{
	if (HasAuthority())
	{
		if (const FEnemyStats* BaseStatsForType = USquadDamageHelper::GetStatsForType(this, SquadEnemyTypeTag))
		{
			Stats = *BaseStatsForType;
			Stats.CurrentHealth = Stats.MaxHealth;
			Stats.CurrentArmor = Stats.MaxArmor;
		}
		else
			UE_LOG(LogTemp, Error, TEXT("No base stats found for enemy tag %s on enemy %s"), *SquadEnemyTypeTag.ToString(), *this->GetName());
		RegisterStartingSenses();
		bIsDead = false;
	}
}

void ASquadTestEnemy::PreparingToStartCommand(const bool bShouldMove, const bool bUseTick)
{
	bIsOnTheMove = bShouldMove;
	SetActorTickEnabled(bUseTick);
}

void ASquadTestEnemy::HasFinishedCommand()
{
	bIsOnTheMove = false;
	CurrentSquadCommandData.bIsSet = false;
	
	UE_LOG(LogTemp, Warning, TEXT("%s stopped ticking"), *GetName());
}

void ASquadTestEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CurrentSquadCommandData.bIsSet || bIsOnTheMove == false || bIsDead)
	{	
		PreparingToStartCommand(false, false);
		return;
	}

	if (bIsOnTheMove)
	{
		MoveToLocation(DeltaTime);	
	}
}

void ASquadTestEnemy::SetAndValidateSquadCommandData(const FSquadCommandPayload& SquadCommandPayload)
{
	CurrentSquadCommandData = SquadCommandPayload;
	CurrentSquadCommandData.bIsSet = true;
}

void ASquadTestEnemy::Multicast_HandleDeath_Implementation()
{
	SkeletalMesh->SetSimulatePhysics(true);
	SkeletalMesh->SetEnableGravity(true);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	SkeletalMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	SkeletalMesh->SetCollisionResponseToChannel(ECC_Projectile, ECR_Ignore);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SkeletalMesh->AddImpulse(FVector(0.0f, 0.0f, 100.0f), NAME_None, true);

	SetLifeSpan(5.0f);
}

void ASquadTestEnemy::RegisterStartingSenses()
{
	if (StimuliSourceComponent)
	{
		for (auto Sense : StartingSenses)
		{
			StimuliSourceComponent->RegisterForSense(Sense);
			RegisteredSenses.AddUnique(Sense);
		}
		StimuliSourceComponent->RegisterWithPerceptionSystem();
	}
}

void ASquadTestEnemy::RegisterStimulus(const TSubclassOf<UAISense>& SenseToRegister)
{
	if (StimuliSourceComponent && !RegisteredSenses.Contains(SenseToRegister))
	{
		StimuliSourceComponent->RegisterForSense(SenseToRegister);
		RegisteredSenses.AddUnique(SenseToRegister);
	}
}

void ASquadTestEnemy::UnregisterStimulus(const TSubclassOf<UAISense>& SenseToRegister)
{
	if (StimuliSourceComponent && RegisteredSenses.Contains(SenseToRegister))
	{
		StimuliSourceComponent->UnregisterFromSense(SenseToRegister);
		RegisteredSenses.RemoveSingle(SenseToRegister);
	}
}

void ASquadTestEnemy::UnregisterAllStimuli()
{
	if (StimuliSourceComponent)
	{
		for (const auto Sense : RegisteredSenses)
		{
			StimuliSourceComponent->UnregisterFromSense(Sense);
		}
		RegisteredSenses.Empty();
		StimuliSourceComponent->UnregisterFromPerceptionSystem();
	}
}

// Needs to be handles elsewhere
void ASquadTestEnemy::MoveToLocation(const float InDeltaTime)
{
	if (!CurrentSquadCommandData.bIsSet)
		return;

	if (CurrentTargetLocation == FVector::ZeroVector || (CurrentPath && CurrentPath->bPointReached))
	{
		if (CurrentSquadCommandData.PathPoints.IsEmpty())
		{
			CurrentTargetLocation = CurrentSquadCommandData.TargetLocation;
			CurrentPath = nullptr;
		}
		else
			CurrentTargetLocation = GetNextValidTargetPoint();	
	}
	
	const FVector CurrentLocation = GetActorLocation();
	FVector Direction = CurrentTargetLocation - CurrentLocation;

	if (const float Distance = Direction.Size(); Distance < ArrivalTolerance)
	{
		if (CurrentPath)
			CurrentPath->bPointReached = true;

		const FVector NewTargetLocation = GetNextValidTargetPoint();
		if  (NewTargetLocation ==  FVector::ZeroVector)
		{
			HasFinishedCommand();
			return;	
		}
		
		CurrentTargetLocation = NewTargetLocation;
		Direction = CurrentTargetLocation - CurrentLocation;
	}

	if (!Direction.IsNearlyZero())
	{
		Direction.Normalize();
		const FVector DeltaMovement = Direction * Stats.MaxMovementSpeed * InDeltaTime;
		const FVector NewLocation = CurrentLocation + DeltaMovement;

		const FRotator NewRotation = Direction.Rotation();
		const FRotator SmoothedRotation = FMath::RInterpTo(GetActorRotation(), NewRotation, InDeltaTime, 5.f);

		Velocity = Direction * Stats.MaxMovementSpeed;
		SetActorLocationAndRotation(NewLocation, SmoothedRotation);
	}
}

// Needs to be handled elsewhere
FVector ASquadTestEnemy::GetNextValidTargetPoint()
{
	const float SocketZOffset = GetCombatSocketLocation().Z - SkeletalMesh->GetComponentLocation().Z;
	
	for (FPathPointData& PathPoint : CurrentSquadCommandData.PathPoints)
	{
		if (PathPoint.bPointReached)
			continue;
		
		CurrentPath = &PathPoint;
		FVector HeightAdjustment = PathPoint.PointLocation;
		HeightAdjustment.Z += SocketZOffset;
		return HeightAdjustment;
	}

	return FVector::ZeroVector;
}

