// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "IsekaiStaminaCostAbility.h"
#include "GA_Jump.generated.h"

/**
 * Simple jump ability.
 *
 * - Triggered via an input mapped to Tags::Ability::Movement_Jump.
 * - Pays a one-shot stamina cost via UIsekaiStaminaCostAbility.
 * - Uses local prediction and ends immediately after triggering the engine jump.
 */
UCLASS()
class AIASSESSMENT_API UGA_Jump : public UIsekaiStaminaCostAbility
{
	GENERATED_BODY()
public:
	UGA_Jump();
	
protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const override;
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		const FGameplayEventData* TriggerEventData
	) override;
};
