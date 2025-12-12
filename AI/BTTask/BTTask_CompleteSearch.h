// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CompleteSearch.generated.h"

/**
 * Signals the Stealth Component that the AI has finished its physical search pattern
 * and the Alert Value is allowed to decay.
 */
UCLASS()
class AIASSESSMENT_API UBTTask_CompleteSearch : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_CompleteSearch();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
