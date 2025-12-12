// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "IsekaiAIController.h"

#include "IsekaiBlackboardKeys.h"
#include "AIAssessment/IsekaiLoggingChannels.h"
#include "AIAssessment/Actor/IsekaiPatrolPath.h"
#include "AIAssessment/Character/IsekaiAICharacter.h"
#include "AIAssessment/Component/AIStealthComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"

AIsekaiAIController::AIsekaiAIController()
{
	SetupPerceptionSystem();
}

void AIsekaiAIController::SetupPerceptionSystem()
{
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(FName("PerceptionComponent"));
	
	// Configure Sight
	auto* Sight = CreateDefaultSubobject<UAISenseConfig_Sight>(FName("SightConfig"));
	Sight->SightRadius = 2000.f;
	Sight->LoseSightRadius = 2200.f;
	Sight->PeripheralVisionAngleDegrees = 100.f;
	Sight->DetectionByAffiliation.bDetectEnemies = true;
	Sight->DetectionByAffiliation.bDetectNeutrals = true;
	Sight->DetectionByAffiliation.bDetectFriendlies = false;
	Sight->PointOfViewBackwardOffset = 50.f;
	Sight->NearClippingRadius = 50.f;
	
	// Configure Hearing
	auto* Hearing = CreateDefaultSubobject<UAISenseConfig_Hearing>(FName("HearingConfig"));
	Hearing->HearingRange = 3000.f;
	Hearing->DetectionByAffiliation.bDetectFriendlies = false;
	Hearing->DetectionByAffiliation.bDetectNeutrals = true;
	Hearing->DetectionByAffiliation.bDetectEnemies = true;
	
	// Apply
	PerceptionComponent->ConfigureSense(*Sight);
	PerceptionComponent->ConfigureSense(*Hearing);
	PerceptionComponent->SetDominantSense(Sight->GetSenseImplementation());
	
	// Default Team to Enemy
	SetGenericTeamId(FGenericTeamId(EIsekaiTeamID::Enemy));
}

void AIsekaiAIController::BeginPlay()
{
	Super::BeginPlay();
	
	if (!IsValid(PerceptionComponent) || !HasAuthority())
	{
		return;
	}
	
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ThisClass::OnTargetPerceptionUpdated);
}

void AIsekaiAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	ControlledAICharacter = Cast<AIsekaiAICharacter>(InPawn);
	
	if (ControlledAICharacter.IsValid())
	{
		InitAIBehavior();
		
		SetGenericTeamId(ControlledAICharacter->GetGenericTeamId());
	}
	else
	{
		UE_LOG(LogIsekaiAI, Error, TEXT("Possessed Pawn is not of Type AIsekaiAICharacter!"));
	}
}

void AIsekaiAIController::OnUnPossess()
{
	ControlledAICharacter.Reset();
	
	Super::OnUnPossess();
}

void AIsekaiAIController::HandlePawnDeath()
{
	// Stop the brain
	if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent))
	{
		BTComp->StopTree(EBTStopMode::Safe);
	}
	
	StopMovement();

	// Stop perception
	if (IsValid(PerceptionComponent))
	{
		PerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(), false);
		PerceptionComponent->SetSenseEnabled(UAISense_Hearing::StaticClass(), false);
		PerceptionComponent->OnTargetPerceptionUpdated.RemoveAll(this);
	}
	
	ClearFocus(EAIFocusPriority::Gameplay);
	
	ResetBlackboard();
	
	Destroy();
}

void AIsekaiAIController::InitAIBehavior()
{
	if (!ControlledAICharacter.IsValid()) return;
	
	// Run Behavior Tree
	if (UBehaviorTree* BT = ControlledAICharacter.Get()->GetAIBehaviorTree())
	{
		if (!RunBehaviorTree(BT))
		{
			UE_LOG(LogIsekaiAI, Error, TEXT("RunBehaviorTree failed"));
			return;
		}
	}
	else
	{
		UE_LOG(LogIsekaiAI, Error, TEXT("No Behavior Tree assigned to AI Character"));
		return;
	}
	
	// Seed Blackboard values
	ResetBlackboard();
	
	// Cache Patrol Path if assigned
	if (ControlledAICharacter.IsValid())
	{
		if (ControlledAICharacter->GetPatrolPath().IsValid())
		{
			CachedPatrolPath = ControlledAICharacter->GetPatrolPath().Get();
		}
		else if (!ControlledAICharacter->GetPatrolPath().IsNull())
		{
			// Try to load it
			if (AIsekaiPatrolPath* LoadedPath = ControlledAICharacter->GetPatrolPath().LoadSynchronous())
			{
				CachedPatrolPath = LoadedPath;
			}
		}
	}
	
	// Initialize Stealth Component on the Server
	if (UAIStealthComponent* StealthComp = ControlledAICharacter.Get()->GetStealthComponent())
	{
		StealthComp->Init(this, GetBlackboardComponent(), ControlledAICharacter->GetAlertTuning());
	}
	
	if (AIsekaiCharacterBase* AIPawn = Cast<AIsekaiCharacterBase>(GetPawn()))
	{
		// Sync Team ID from Pawn to Controller
		SetGenericTeamId(AIPawn->GetGenericTeamId());
	}
}

void AIsekaiAIController::OnTargetPerceptionUpdated(AActor* InTargetActor, FAIStimulus InStimulus)
{
	if (!HasAuthority() || !ControlledAICharacter.IsValid())
	{
		return;
	}

	UAIStealthComponent* StealthComponent = ControlledAICharacter.Get()->GetStealthComponent();
	if (!IsValid(StealthComponent))
	{
		return;
	}
	
	// Forward to Stealth Component
	const FAISenseID SightID = UAISense::GetSenseID<UAISense_Sight>();
	const FAISenseID HearingID = UAISense::GetSenseID<UAISense_Hearing>();
	
	if (InStimulus.Type == SightID)
	{
		StealthComponent->HandleSightStimulus(InTargetActor, InStimulus);
	}
	else if (InStimulus.Type == HearingID)
	{
		StealthComponent->HandleHearingStimulus(InTargetActor, InStimulus);
	}
	else
	{
		// Unknown sense
	}
}

void AIsekaiAIController::ResetBlackboard()
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!IsValid(BB) || !GetPawn())
	{
		return;
	}
	
	BB->SetValueAsObject(BBKeys::GSelfActor, GetPawn());
	BB->ClearValue(BBKeys::GTargetActor);
	
	BB->ClearValue(BBKeys::GLastKnownPosition);
	BB->ClearValue(BBKeys::GStimulusLocation);
	BB->ClearValue(BBKeys::GMoveToLocation);
	
	BB->SetValueAsBool(BBKeys::GHasLOS, false);
	BB->SetValueAsFloat(BBKeys::GAlertLevel, 0.f);
	BB->SetValueAsEnum(BBKeys::GStealthState, static_cast<uint8>(EStealthState::Idle));
	
	BB->SetValueAsInt(BBKeys::GPatrolIndex, -1);
}