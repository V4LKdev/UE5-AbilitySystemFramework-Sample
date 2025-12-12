// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "IsekaiGameplayAbility.h"
#include "GA_Crouch.generated.h"

/**
 * Toggles Character Crouch state.
 * 
 * Logic:
 * - If Standing -> Crouch() and Apply GE_Crouch (Tag: State.Posture.Crouch)
 * - If Crouched -> UnCrouch() and Remove GE_Crouch
 * 
 * Policy: OnInputTriggered (Toggle)
 */
UCLASS()
class AIASSESSMENT_API UGA_Crouch : public UIsekaiGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Crouch();
	
protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		const FGameplayEventData* TriggerEventData
	) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual bool CanBeCanceled() const override;
};
