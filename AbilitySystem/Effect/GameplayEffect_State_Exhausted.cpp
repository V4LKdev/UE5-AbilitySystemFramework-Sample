// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "GameplayEffect_State_Exhausted.h"

#include "AIAssessment/NativeGameplayTags.h"
#include "AIAssessment/AbilitySystem/EffectComponent/IsekaiCancelAbilityTagsGameplayEffectComponent.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "GameplayEffectComponents/BlockAbilityTagsGameplayEffectComponent.h"

UGameplayEffect_State_Exhausted::UGameplayEffect_State_Exhausted()
{
}

void UGameplayEffect_State_Exhausted::PostInitProperties()
{
	Super::PostInitProperties();
	
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}
	
	
	// 1. Grant tags to target
	UTargetTagsGameplayEffectComponent& TargetTagsComp = FindOrAddComponent<UTargetTagsGameplayEffectComponent>();
	
	FInheritedTagContainer TagChanges;
	
	TagChanges.Added.AddTag(Tags::State::Exhausted);
	
	TargetTagsComp.SetAndApplyTargetTagChanges(TagChanges);
	
	
	// 2. Block all stamina abilities
	UBlockAbilityTagsGameplayEffectComponent& BlockTagsComp = FindOrAddComponent<UBlockAbilityTagsGameplayEffectComponent>();
	
	FInheritedTagContainer BlockedTags;
	
	BlockedTags.Added.AddTag(Tags::Ability::Stamina);
	
	BlockTagsComp.SetAndApplyBlockedAbilityTagChanges(BlockedTags);
	
	
	// 3. Cancel continuous stamina cost abilities on apply
	UIsekaiCancelAbilityTagsGameplayEffectComponent& CancelTagsComp = FindOrAddComponent<UIsekaiCancelAbilityTagsGameplayEffectComponent>();
	
	FInheritedTagContainer CanceledTags;
	
	CanceledTags.Added.AddTag(Tags::Ability::Stamina_Drain);
	
	CancelTagsComp.SetAndApplyCanceledAbilityTagChanges(CanceledTags);
}
