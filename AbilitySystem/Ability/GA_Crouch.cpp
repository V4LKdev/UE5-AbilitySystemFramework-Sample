// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "GA_Crouch.h"

#include "AIAssessment/IsekaiLoggingChannels.h"
#include "AIAssessment/NativeGameplayTags.h"
#include "AIAssessment/Character/IsekaiCharacterBase.h"
#include "AIAssessment/Character/IsekaiCharacterMovementComponent.h"
#include "GameFramework/Character.h"

UGA_Crouch::UGA_Crouch()
{
	FGameplayTagContainer CombinedTags = GetAssetTags();
	CombinedTags.AddTag(Tags::Ability::Movement_Crouch);
	SetAssetTags(CombinedTags);
	
	ActivationOwnedTags.AddTag(Tags::State::Movement_Crouching);
	
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationPolicy = OnInputTriggered;
	
	CancelAbilitiesWithTag.AddTag(Tags::Ability::Movement_Sprint);
	BlockAbilitiesWithTag.AddTag(Tags::Ability::Movement_Sprint);
	BlockAbilitiesWithTag.AddTag(Tags::Ability::Movement_Jump);
}

bool UGA_Crouch::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	
	const ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	
	// Can only crouch if on ground
	if (!Character || !Character->GetCharacterMovement()->IsMovingOnGround())
	{
		return false;
	}
	
	return Character && Character->CanCrouch();
}

void UGA_Crouch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}
		
		ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());
		
		Character->Crouch();
		
		UE_LOG(LogIsekaiAbilitySystem, VeryVerbose, TEXT("Crouch Ability activated on %s"), *Character->GetName());
	}
}

void UGA_Crouch::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsLocallyControlled())
	{
		const UIsekaiCharacterMovementComponent* IsekaiCMC = nullptr;
		if (const AIsekaiCharacterBase* IsekaiChar = GetIsekaiCharacter())
		{
			IsekaiCMC = IsekaiChar->GetIsekaiCharacterMovement();
		}

		if (IsValid(IsekaiCMC) && IsekaiCMC->CanUncrouch())
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		}
	}
}

bool UGA_Crouch::CanBeCanceled() const
{
	if (!Super::CanBeCanceled())
	{
		return false;
	}
	
	// Allow canceling only if we can uncrouch
	const UIsekaiCharacterMovementComponent* IsekaiCMC = nullptr;
	if (const AIsekaiCharacterBase* IsekaiChar = GetIsekaiCharacter())
	{
		IsekaiCMC = IsekaiChar->GetIsekaiCharacterMovement();
	}
	
	return IsValid(IsekaiCMC) && IsekaiCMC->CanUncrouch();
}


void UGA_Crouch::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	const UIsekaiCharacterMovementComponent* IsekaiCMC = nullptr;
	AIsekaiCharacterBase* IsekaiChar = GetIsekaiCharacter();
	if (IsekaiChar)
	{
		IsekaiCMC = IsekaiChar->GetIsekaiCharacterMovement();
	}
		
	if (IsValid(IsekaiCMC) && IsekaiCMC->CanUncrouch())
	{
		IsekaiChar->UnCrouch();
		UE_LOG(LogIsekaiAbilitySystem, VeryVerbose, TEXT("Crouch Ability ended on %s"), *IsekaiChar->GetName());
	}
}
