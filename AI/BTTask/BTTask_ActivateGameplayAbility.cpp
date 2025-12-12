// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "BTTask_ActivateGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"

UBTTask_ActivateGameplayAbility::UBTTask_ActivateGameplayAbility()
{
	NodeName = "Activate Ability";
	bNotifyTick = true; // We use Tick to check for completion
}

EBTNodeResult::Type UBTTask_ActivateGameplayAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	APawn* Pawn = AICon->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!ASC) return EBTNodeResult::Failed;

	// 1. Try Activate
	if (!ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag)))
	{
		// Failed to start (Cooldown? Cost? Tag Missing?)
		return EBTNodeResult::Failed;
	}

	// 2. Fire and Forget?
	if (!bWaitForCompletion)
	{
		return EBTNodeResult::Succeeded;
	}

	// 3. Find the Handle we just activated
	FTaskMemory* MyMemory = CastInstanceNodeMemory<FTaskMemory>(NodeMemory);
	MyMemory->ASC = ASC;
	MyMemory->AbilityHandle = FGameplayAbilitySpecHandle();

	// We look for an Active ability with this Tag.
	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->GetAssetTags().HasTag(AbilityTag) && Spec.IsActive())
		{
			MyMemory->AbilityHandle = Spec.Handle;
			return EBTNodeResult::InProgress;
		}
	}
	
	// If we got here, the ability started but finished instantly (or failed silently).
	return EBTNodeResult::Succeeded;
}

void UBTTask_ActivateGameplayAbility::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FTaskMemory* MyMemory = CastInstanceNodeMemory<FTaskMemory>(NodeMemory);
	
	if (!MyMemory->ASC.IsValid() || !MyMemory->AbilityHandle.IsValid())
	{
		// Something broke or handle invalid -> Finish
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
	
	// Poll: Is the ability still active?
	FGameplayAbilitySpec* Spec = MyMemory->ASC->FindAbilitySpecFromHandle(MyMemory->AbilityHandle);
	if (!Spec || !Spec->IsActive())
	{
		// Spec is gone or inactive -> We are done.
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_ActivateGameplayAbility::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FTaskMemory* MyMemory = CastInstanceNodeMemory<FTaskMemory>(NodeMemory);
	
	if (MyMemory->ASC.IsValid() && MyMemory->AbilityHandle.IsValid())
	{
		// Force Cancel the ability if the Tree aborts
		MyMemory->ASC->CancelAbilityHandle(MyMemory->AbilityHandle);
	}

	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_ActivateGameplayAbility::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
	
	// Cleanup if needed (Memory is auto-cleared, but good for safety)
	FTaskMemory* MyMemory = CastInstanceNodeMemory<FTaskMemory>(NodeMemory);
	MyMemory->AbilityHandle = FGameplayAbilitySpecHandle();
	MyMemory->ASC.Reset();
}