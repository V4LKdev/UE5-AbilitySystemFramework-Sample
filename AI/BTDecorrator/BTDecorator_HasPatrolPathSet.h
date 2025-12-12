// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_HasPatrolPathSet.generated.h"

/**
 * 
 */
UCLASS()
class AIASSESSMENT_API UBTDecorator_HasPatrolPathSet : public UBTDecorator
{
	GENERATED_BODY()
public:
	UBTDecorator_HasPatrolPathSet();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
