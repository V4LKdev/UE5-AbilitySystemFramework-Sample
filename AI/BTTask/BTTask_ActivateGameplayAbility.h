// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ActivateGameplayAbility.generated.h"

class UAbilitySystemComponent;
/**
 * Activates a Gameplay Ability on the AI Pawn by Tag.
 * Can optionally wait for the ability to end before finishing the task.
 */
UCLASS()
class AIASSESSMENT_API UBTTask_ActivateGameplayAbility : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_ActivateGameplayAbility();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	UPROPERTY(EditAnywhere, Category = "Ability", meta = (Categories = "Ability"))
	FGameplayTag AbilityTag;

	UPROPERTY(EditAnywhere, Category = "Ability")
	bool bWaitForCompletion = true;
	
	struct FTaskMemory
	{
		TWeakObjectPtr<class UAbilitySystemComponent> ASC;
		FGameplayAbilitySpecHandle AbilityHandle;
	};

	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FTaskMemory); }
};