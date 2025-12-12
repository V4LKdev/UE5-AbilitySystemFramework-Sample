// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "BTService_SetMoveState.h"

#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTService_SetMoveState::UBTService_SetMoveState()
{ 
	NodeName = "Set Movement State";
	
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	bNotifyTick = false; 
}

void UBTService_SetMoveState::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	const AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return;

	ACharacter* Char = Cast<ACharacter>(AICon->GetPawn());
	if (!Char) return;

	FMemoryData* MyMemory = CastInstanceNodeMemory<FMemoryData>(NodeMemory);
	
	if (UCharacterMovementComponent* CMC = Char->GetCharacterMovement())
	{
		// 1. Snapshot Current State
		MyMemory->PreviousSpeed = CMC->MaxWalkSpeed;
		MyMemory->bWasOrientingToMovement = CMC->bOrientRotationToMovement;
		MyMemory->bWasUsingDesiredRotation = CMC->bUseControllerDesiredRotation;

		// 2. Apply Speed
		CMC->MaxWalkSpeed = TargetSpeed;
		
		// 3. Apply Rotation Mode
		if (bUseStrafing)
		{
			// Strafing Mode:
			// - Don't rotate towards velocity vector
			CMC->bOrientRotationToMovement = false;
			CMC->bUseControllerDesiredRotation = true;
		}
		else
		{
			// Free Roam Mode:
			// - Face the direction of travel
			CMC->bOrientRotationToMovement = true;
			CMC->bUseControllerDesiredRotation = false;
		}
	}
}

void UBTService_SetMoveState::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	const AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return;

	const ACharacter* Char = Cast<ACharacter>(AICon->GetPawn());
	if (!Char) return;

	const FMemoryData* MyMemory = CastInstanceNodeMemory<FMemoryData>(NodeMemory);

	// Restore State
	if (UCharacterMovementComponent* CMC = Char->GetCharacterMovement())
	{
		CMC->MaxWalkSpeed = MyMemory->PreviousSpeed;
		CMC->bOrientRotationToMovement = MyMemory->bWasOrientingToMovement;
		CMC->bUseControllerDesiredRotation = MyMemory->bWasUsingDesiredRotation;
	}
}
