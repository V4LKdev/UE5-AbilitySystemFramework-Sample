// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "GameplayEffect_State_Dead.h"

#include "AIAssessment/NativeGameplayTags.h"
#include "GameplayEffectComponents/BlockAbilityTagsGameplayEffectComponent.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UGameplayEffect_State_Dead::UGameplayEffect_State_Dead()
{
}

void UGameplayEffect_State_Dead::PostInitProperties()
{
	Super::PostInitProperties();
	
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}
	
	// 1. Grant tags to target
	UTargetTagsGameplayEffectComponent& TargetTagsComp = FindOrAddComponent<UTargetTagsGameplayEffectComponent>();
	
	FInheritedTagContainer TagChanges;
	
	TagChanges.Added.AddTag(Tags::State::Dead);
	TagChanges.Added.AddTag(Tags::State::Stamina_RegenBlocked);
	TagChanges.Added.AddTag(Tags::State::Health_RegenBlocked);
	
	TargetTagsComp.SetAndApplyTargetTagChanges(TagChanges);
	
	// 2. Block all stamina abilities
	UBlockAbilityTagsGameplayEffectComponent& BlockTagsComp = FindOrAddComponent<UBlockAbilityTagsGameplayEffectComponent>();
	
	FInheritedTagContainer BlockedTags;
	
	// All abilities
	BlockedTags.Added.AddTag(Tags::Ability::Ability);
	
	BlockTagsComp.SetAndApplyBlockedAbilityTagChanges(BlockedTags);
	
	// Active abilities are already canceled in the ASC HandleOutOfHealth function
}
