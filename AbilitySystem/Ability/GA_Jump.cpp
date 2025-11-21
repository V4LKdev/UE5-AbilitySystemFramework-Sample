// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "GA_Jump.h"
#include "AIAssessment/NativeGameplayTags.h"
#include "GameFramework/Character.h"
#include "AIAssessment/Character/IsekaiCharacterBase.h"
#include "AIAssessment/IsekaiLoggingChannels.h"

UGA_Jump::UGA_Jump()
{
	// --- Tags ---
	FGameplayTagContainer CombinedTags = GetAssetTags();
	CombinedTags.AddTag(Tags::Ability::Movement_Jump);
	// CombinedTags.AddTag(Tags::Ability::Stamina); - Added by parent class
	SetAssetTags(CombinedTags);

	// --- Instancing ---
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// --- Networking ---
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	// --- Activation Policy ---
	ActivationPolicy = OnInputTriggered;

	// --- Stamina ---
	// Values should be configured in the Blueprint.
	InitialStaminaCost = 20.0f;
	bAllowCostOverdraw = true;
}

bool UGA_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// First, check if the parent class (IsekaiStaminaCostAbility) can activate.
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const ACharacter* Character = GetIsekaiCharacter();
	return (Character && Character->CanJump());
}

void UGA_Jump::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, 
	const FGameplayEventData* TriggerEventData)
{
	// Only proceed if we have authority or prediction key
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		// --- Commit the ability (pays costs, puts on cooldown, etc) ---
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			UE_LOG(LogIsekaiAbilitySystem, Verbose,
				TEXT("UGA_Jump: CommitAbility failed for %s (likely not enough stamina)"),
				*GetName());
			// Ability failed to commit (e.g., not enough stamina). Cancel it.
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}

		// --- Perform the jump ---
		ACharacter* Character = GetIsekaiCharacter();
		if (Character)
		{
			UE_LOG(LogIsekaiAbilitySystem, Verbose,
				TEXT("UGA_Jump activated for character %s"),
				*Character->GetName());
			Character->Jump();
		}
	}
	
	// The ability's job is done the moment it triggers the jump.
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
