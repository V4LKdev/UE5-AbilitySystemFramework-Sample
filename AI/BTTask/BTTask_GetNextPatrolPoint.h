// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_GetNextPatrolPoint.generated.h"

/**
 * Calculates the next waypoint on the assigned Patrol Spline.
 * Includes NavMesh projection and reachability tests to prevent AI getting stuck.
 */
UCLASS()
class AIASSESSMENT_API UBTTask_GetNextPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_GetNextPatrolPoint();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
protected:
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector PatrolIndexKey;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector MoveToLocationKey;
	
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector PatrolDirectionKey;
	
	/** If true, verifies the path exists before committing. */
	UPROPERTY(EditAnywhere, Category="Config")
	bool bCheckReachability = true;
};