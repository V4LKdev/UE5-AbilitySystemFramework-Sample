// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "IsekaiCancelAbilityTagsGameplayEffectComponent.h"

#include "AbilitySystemComponent.h"

bool UIsekaiCancelAbilityTagsGameplayEffectComponent::OnActiveGameplayEffectAdded(
	FActiveGameplayEffectsContainer& ActiveGEContainer, FActiveGameplayEffect& ActiveGE) const
{

	UAbilitySystemComponent* ASC = ActiveGEContainer.Owner;
	if (!ASC)
	{
		return true; // continue processing
	}
	
	FGameplayTagContainer TagsToCancel;
	CanceledAbilityTags.ApplyTo(TagsToCancel);
	
	if (TagsToCancel.IsEmpty())
	{
		return true;
	}
	
	const FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();
	const bool bIsAuthority = ActiveGEContainer.IsNetAuthority();
	const bool bIsLocallyControlled = ActorInfo && ActorInfo->IsLocallyControlled();
	
	// 1 Server
	if (bIsAuthority)
	{
		ASC->CancelAbilities(&TagsToCancel);
		return true;
	}
	
	// 2 Client (local prediction)
	if (bIsLocallyControlled)
	{
		ASC->CancelAbilities(&TagsToCancel);
	}
	
	return true;
}

void UIsekaiCancelAbilityTagsGameplayEffectComponent::SetAndApplyCanceledAbilityTagChanges(
	const FInheritedTagContainer& InCanceledAbilityTags)
{
	CanceledAbilityTags = InCanceledAbilityTags;
	
	CanceledAbilityTags.UpdateInheritedTagProperties(nullptr);
}
