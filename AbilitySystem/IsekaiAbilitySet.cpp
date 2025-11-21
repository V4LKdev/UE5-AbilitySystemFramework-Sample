// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "IsekaiAbilitySet.h"

#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "IsekaiAbilitySystemComponent.h"
#include "Ability/IsekaiGameplayAbility.h"

void FIsekaiAbilitySetGrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FIsekaiAbilitySetGrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		EffectHandles.Add(Handle);
	}
}

void FIsekaiAbilitySetGrantedHandles::AddAttributeSet(UAttributeSet* AttributeSet)
{
	if (AttributeSet)
	{
		GrantedAttributeSets.Add(AttributeSet);
	}
}

void FIsekaiAbilitySetGrantedHandles::TakeFromAbilitySystem(UIsekaiAbilitySystemComponent* AbilitySystemComponent)
{
	if (!AbilitySystemComponent || !AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		return;
	}
	
	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			AbilitySystemComponent->ClearAbility(Handle);
		}
	}
	AbilitySpecHandles.Reset();
	
	for (const FActiveGameplayEffectHandle& Handle : EffectHandles)
	{
		if (Handle.IsValid())
		{
			AbilitySystemComponent->RemoveActiveGameplayEffect(Handle);
		}
	}
	EffectHandles.Reset();
	
	if (GrantedAttributeSets.Num() > 0)
	{
		for (UAttributeSet* AttributeSet : GrantedAttributeSets)
		{
			if (AttributeSet)
			{
				AbilitySystemComponent->RemoveSpawnedAttribute(AttributeSet);
			}
		}
		GrantedAttributeSets.Reset();
	}
}

void FIsekaiAbilitySetGrantedHandles::Reset()
{
	AbilitySpecHandles.Reset();
	EffectHandles.Reset();
	GrantedAttributeSets.Reset();
}

void UIsekaiAbilitySet::GiveToAbilitySystem(UIsekaiAbilitySystemComponent* ASC,
	FIsekaiAbilitySetGrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	if (!ASC || !ASC->IsOwnerActorAuthoritative())
	{
		return;
	}
	
	// Attribute Sets
	for (const FIsekaiGrantedAttributeSet& AttributeSetInfo : AttributeSets)
	{
		if (!*AttributeSetInfo.AttributeSetClass)
		{
			continue;
		}
		
		UAttributeSet* NewSet = NewObject<UAttributeSet>(ASC->GetOwner(), AttributeSetInfo.AttributeSetClass);
		
		ASC->AddAttributeSetSubobject(NewSet);
		
		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAttributeSet(NewSet);
		}
	}
	
	// Abilities
	for (const FIsekaiGrantedAbility& AbilityInfo : Abilities)
	{
		if (!*AbilityInfo.AbilityClass)
		{
			continue;
		}
		
		const int32 AbilityLevel = FMath::Max(1, AbilityInfo.AbilityLevel);
		
		FGameplayAbilitySpec AbilitySpec(
			AbilityInfo.AbilityClass, 
			AbilityLevel, 
			INDEX_NONE, 
			SourceObject);
		
		if (AbilityInfo.InputTag.IsValid())
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityInfo.InputTag);
		}
		
		const FGameplayAbilitySpecHandle SpecHandle = ASC->GiveAbility(AbilitySpec);
		
		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(SpecHandle);
		}
	}
	
	// Effects
	for (const FIsekaiGrantedEffect& EffectInfo : Effects)
	{
		if (!*EffectInfo.EffectClass)
		{
			continue;
		}
		
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		if (SourceObject)
		{
			ContextHandle.AddSourceObject(SourceObject);
		}
		
		const float EffectLevel = FMath::Max(1, EffectInfo.EffectLevel);
		
		FGameplayEffectSpecHandle SpecHandle =
			ASC->MakeOutgoingSpec(EffectInfo.EffectClass, EffectLevel, ContextHandle);
		
		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle EffectHandle =
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			
			if (OutGrantedHandles && EffectHandle.IsValid())
			{
				OutGrantedHandles->AddGameplayEffectHandle(EffectHandle);
			}
		}
	}
}
