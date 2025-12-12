// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "BTDecorator_HasPatrolPathSet.h"

#include "AIAssessment/AI/IsekaiAIController.h"

UBTDecorator_HasPatrolPathSet::UBTDecorator_HasPatrolPathSet()
{
	NodeName = TEXT("Has Patrol Path Set?");
}

bool UBTDecorator_HasPatrolPathSet::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	if (AIsekaiAIController* AICon = Cast<AIsekaiAIController>(OwnerComp.GetAIOwner()))
	{
		return AICon->GetPatrolPath() != nullptr;
	}
	return false;
}
