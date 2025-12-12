// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindClosestPatrolPoint.generated.h"

/**
 * 
 */
UCLASS()
class AIASSESSMENT_API UBTTask_FindClosestPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_FindClosestPatrolPoint();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector PatrolIndexKey;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector MoveToLocationKey;
	
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector PatrolDirectionKey;
	
	UPROPERTY(EditAnywhere, Category="Config")
	uint8 PatrolDirection = 1;
	
	UPROPERTY(EditAnywhere, Category="Config")
	bool bRandomizeDirection = false;
};
