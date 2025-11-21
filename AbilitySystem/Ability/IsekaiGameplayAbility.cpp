// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "IsekaiGameplayAbility.h"

#include "AIAssessment/NativeGameplayTags.h"
#include "AIAssessment/AbilitySystem/IsekaiAbilitySystemComponent.h"
#include "AIAssessment/Character/IsekaiCharacterBase.h"

UIsekaiGameplayAbility::UIsekaiGameplayAbility()
{
	ActivationBlockedTags.AddTag(Tags::State::Dead);
	ActivationBlockedTags.AddTag(Tags::State::AbilityInputBlocked);
}

AIsekaiCharacterBase* UIsekaiGameplayAbility::GetIsekaiCharacter() const
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	return ActorInfo ? Cast<AIsekaiCharacterBase>(ActorInfo->AvatarActor.Get()) : nullptr;
}

UIsekaiAbilitySystemComponent* UIsekaiGameplayAbility::GetIsekaiAbilitySystemComponent() const
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	return ActorInfo ? Cast<UIsekaiAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()) : nullptr;
}

const UIsekaiAttributeSet* UIsekaiGameplayAbility::GetIsekaiAttributeSet() const
{
	if (const UIsekaiAbilitySystemComponent* IsekaiASC = GetIsekaiAbilitySystemComponent())
	{
		return IsekaiASC->GetSet<UIsekaiAttributeSet>();
	}
	return nullptr;
}

void UIsekaiGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	if (!ActorInfo->IsLocallyControlled())
	{
		return;
	}
	
	if (ActivationPolicy == WhileInputActive)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}
