// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Character/AI/TurretBehaviour.h"
#include "SDTurretController.generated.h"

struct FAIStimulus;
class ASDTurret;

/**
 * 
 */
UCLASS()
class SUPERDEFENCE_API ASDTurretController : public AAIController, public ITurretBehaviour
{
	GENERATED_BODY()
public:
	ASDTurretController();
	UFUNCTION(BlueprintCallable)
	virtual bool Controller_TurretSelectTarget() override;

	FORCEINLINE TArray<AActor*> GetAllTargetActors() const { return AllTargetActors; }

protected:
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void OnPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus);
	UFUNCTION()
	void OnPerceptionForgotten(AActor* TargetActor);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Setup")
	TObjectPtr<UBehaviorTree> BehvaviorTree;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Setup")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Behavior")
	TObjectPtr<ASDTurret> PossessedTurret;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Behavior")
	TArray<AActor*> AllTargetActors;

private:
	void RemoveTargetActor(AActor* TargetActor);

	void SetTargetActor(AActor* TargetActor);

	AActor* ChooseTargetActor();

	UPROPERTY(VisibleAnywhere, Category = "AI|Setup")
	AActor* LastTargetActor;

	UPROPERTY(EditAnywhere, Category = "AI|Setup")
	FName CurrentTargetBlackboardValueName;

	UPROPERTY(EditAnywhere, Category = "AI|Setup")
	int32 TargetSelectRetryCount;

	bool bDrawDebug = false;
};
