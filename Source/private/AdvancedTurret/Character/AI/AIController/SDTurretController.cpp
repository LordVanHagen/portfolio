// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AI/AIController/SDTurretController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Character/SDTurret.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Interaction/CombatInterface.h"

ASDTurretController::ASDTurretController()
{
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 2000.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(2.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

    //NOTE: TimeUntilNextUpdate on AISense regulates the Tick intervalls for the sight perception
    
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ASDTurretController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    PossessedTurret = CastChecked<ASDTurret>(InPawn);
#if IF_WITH_EDITOR
    bDrawDebug = PossessedTurret->GetDrawDebug();
#endif
    
    // Runs selected BehaviorTree and sets Blackboard for us, which can now be accessed via Blackboard.Get()
    RunBehaviorTree(BehvaviorTree);
    
    if (IsValid(AIPerceptionComponent))
    {
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ASDTurretController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this, &ASDTurretController::OnPerceptionForgotten);
    }
}

void ASDTurretController::OnPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus)
{
    TSubclassOf<UAISense> SenseClassByStimulus = UAIPerceptionSystem::GetSenseClassForStimulus(GetWorld(), Stimulus);

    if (SenseClassByStimulus == UAISense_Sight::StaticClass())
    {
        if (Stimulus.WasSuccessfullySensed() && IsValid(TargetActor))
            AllTargetActors.AddUnique(TargetActor);
    }
}

void ASDTurretController::OnPerceptionForgotten(AActor* TargetActor)
{
    RemoveTargetActor(TargetActor);

#if IF_WITH_EDITOR
    if(bDrawDebug)
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple, FString::Printf(TEXT("Forgot actor: %s"), *GetNameSafe(TargetActor)));
#endif
}

void ASDTurretController::RemoveTargetActor(AActor* TargetActor)
{
    if (TargetActor == PossessedTurret->GetCurrentTarget())
    {
        PossessedTurret->SetCurrentTarget(nullptr);
        Blackboard.Get()->ClearValue(CurrentTargetBlackboardValueName);
    }

    AllTargetActors.RemoveSingle(TargetActor);
}


void ASDTurretController::SetTargetActor(AActor* TargetActor)
{
    if (!IsValid(TargetActor))
        return;

    PossessedTurret->SetCurrentTarget(TargetActor);
    Blackboard.Get()->SetValueAsObject(CurrentTargetBlackboardValueName, TargetActor);
    LastTargetActor = TargetActor;
}

AActor* ASDTurretController::ChooseTargetActor()
{
    //TODO: Implement advanced target selection here

    if (!AllTargetActors.IsEmpty())
    {
        int32 Retries = 0;

        while (Retries <= TargetSelectRetryCount && !AllTargetActors.IsEmpty())
        {
            const int32 RandomIndex = FMath::RandRange(0, AllTargetActors.Num() - 1);
            if (IsValid(AllTargetActors[RandomIndex]))
            {
                return AllTargetActors[RandomIndex];
            }
            else
            {
                AllTargetActors.RemoveSingle(AllTargetActors[RandomIndex]);
                Retries++;
                continue;
            }
        }
    }

    return nullptr;
}

bool ASDTurretController::Controller_TurretSelectTarget()
{
    if (IsValid(LastTargetActor) && PossessedTurret->GetCurrentTarget() == LastTargetActor)
    {
        SetTargetActor(LastTargetActor);
        return true;
    }

    AActor* SelectedTarget = ChooseTargetActor();

    if (IsValid(SelectedTarget))
    {
        SetTargetActor(SelectedTarget);
        return true;
    }

    return false;
}