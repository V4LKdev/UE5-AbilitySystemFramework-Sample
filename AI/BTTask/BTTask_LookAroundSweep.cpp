// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "BTTask_LookAroundSweep.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

UBTTask_LookAroundSweep::UBTTask_LookAroundSweep()
{
	NodeName = TEXT("Look Around Sweep");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

uint16 UBTTask_LookAroundSweep::GetInstanceMemorySize() const
{
	return sizeof(FLookSweepMemory);
}

EBTNodeResult::Type UBTTask_LookAroundSweep::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;
	
	const APawn* Pawn = AICon->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	FLookSweepMemory* Memory = CastInstanceNodeMemory<FLookSweepMemory>(NodeMemory);
	
	// 1. Snapshot the "Forward" direction at start of task
	Memory->BaseForward = Pawn->GetActorForwardVector();
	Memory->CurrentTime = 0.f;
	
	// Apply Duration
	Memory->MaxTime = TotalDuration;

	return EBTNodeResult::InProgress;
}

void UBTTask_LookAroundSweep::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FLookSweepMemory* Memory = CastInstanceNodeMemory<FLookSweepMemory>(NodeMemory);
	AAIController* AICon = OwnerComp.GetAIOwner();
	
	// Safety Checks
	if (!AICon || !AICon->GetPawn())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 1. Update Time
	Memory->CurrentTime += DeltaSeconds;

	// 2. Check Finish Condition
	if (TotalDuration > 0.f && Memory->CurrentTime >= Memory->MaxTime)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 3. Calculate Sine Sweep
	// Sine goes -1 to 1. Map to -Angle to +Angle.
	const float SineVal = FMath::Sin(Memory->CurrentTime * SweepSpeed);
	const float CurrentAngle = SineVal * HalfAngle;

	// 4. Calculate Vector
	const FVector RotatedDir = Memory->BaseForward.RotateAngleAxis(CurrentAngle, FVector::UpVector);
	const FVector TargetPos = AICon->GetPawn()->GetActorLocation() + (RotatedDir * LookDistance);

	// 5. DIRECTLY Update Focus
	AICon->SetFocalPoint(TargetPos, EAIFocusPriority::Gameplay);
}

void UBTTask_LookAroundSweep::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	// Clean up focus so we don't get stuck staring at the last calculated point
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		AICon->ClearFocus(EAIFocusPriority::Gameplay);
	}
	
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

FString UBTTask_LookAroundSweep::GetStaticDescription() const
{
	return FString::Printf(TEXT("Directly sweeps Focal Point +/- %.0f deg\nDuration: %.1fs"), 
		HalfAngle, 
		TotalDuration);
}