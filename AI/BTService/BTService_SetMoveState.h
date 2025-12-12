// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_SetMoveState.generated.h"


/**
 * Configures Character Movement properties (Speed, Rotation Mode) for a specific state.
 * Restores them when exiting the state.
 */
UCLASS()
class AIASSESSMENT_API UBTService_SetMoveState : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_SetMoveState();

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// --- Configuration ---
	
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (ClampMin = "0.0", ToolTip="Max Walk Speed to apply."))
	float TargetSpeed = 300.f;

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (ToolTip="If true, disables OrientToMovement and enables DesiredRotation (Strafing)."))
	bool bUseStrafing = false;

	// --- Internal State ---
	struct FMemoryData
	{
		float PreviousSpeed;
		bool bWasOrientingToMovement;
		bool bWasUsingDesiredRotation;
	};
	
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FMemoryData); }
};