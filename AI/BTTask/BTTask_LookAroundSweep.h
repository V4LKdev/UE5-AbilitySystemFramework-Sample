// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_LookAroundSweep.generated.h"

/**
 * Sweeps the AI's focal point back and forth in a cone.
 * Directly updates the AIController's Focus, no Blackboard Key needed.
 */
UCLASS()
class AIASSESSMENT_API UBTTask_LookAroundSweep : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_LookAroundSweep();

	// --- BT Interface ---
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;

	// --- Configuration ---
	UPROPERTY(EditAnywhere, Category="Config", meta=(ClampMin="10.0", UIMin="10.0", UIMax="180.0"))
	float HalfAngle = 45.f;

	UPROPERTY(EditAnywhere, Category="Config", meta=(ClampMin="1.0"))
	float LookDistance = 1000.f;

	UPROPERTY(EditAnywhere, Category="Config", meta=(ClampMin="0.1", ToolTip="How fast the look target sweeps back and forth."))
	float SweepSpeed = 2.0f;

	UPROPERTY(EditAnywhere, Category="Config", meta=(ClampMin="0.0", ToolTip="How long to perform this task. 0 = Infinite."))
	float TotalDuration = 3.0f;
	
	// --- Internal Memory ---
	struct FLookSweepMemory
	{
		FVector BaseForward;
		float CurrentTime;
		float MaxTime;
		// Store previous focus to optionally restore it? 
		// Usually for a task like this, clearing focus on exit is safer.
	};
};