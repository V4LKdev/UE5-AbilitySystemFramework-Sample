// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "BTTask_CompleteSearch.h"

#include "AIController.h"
#include "AIAssessment/Character/IsekaiAICharacter.h"

UBTTask_CompleteSearch::UBTTask_CompleteSearch()
{
	NodeName = "Complete Search";
}

EBTNodeResult::Type UBTTask_CompleteSearch::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;
	
	const AIsekaiAICharacter* AIChar = Cast<AIsekaiAICharacter>(AICon->GetPawn());
	if (!AIChar) return EBTNodeResult::Failed;
	
	UAIStealthComponent* StealthComp = AIChar->GetStealthComponent();
	if (!StealthComp) return EBTNodeResult::Failed;
	
	
	StealthComp->CompleteSearch();
	
	return EBTNodeResult::Succeeded;
}

FString UBTTask_CompleteSearch::GetStaticDescription() const
{
	return TEXT("Unlocks the Alert Value decay logic.\n Clears the LKP from the blackboard.");
}
