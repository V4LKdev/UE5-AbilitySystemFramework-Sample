// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "IsekaiPlayerState.h"

#include "AIAssessment/IsekaiLoggingChannels.h"
#include "AIAssessment/NativeGameplayTags.h"
#include "AIAssessment/AbilitySystem/IsekaiAbilitySystemComponent.h"
#include "AIAssessment/AbilitySystem/IsekaiAttributeSet.h"
#include "AIAssessment/AbilitySystem/Ability/IsekaiGameplayAbility.h"

AIsekaiPlayerState::AIsekaiPlayerState()
{
	// Update more frequently for attribute changes
	SetNetUpdateFrequency(100.f);
	
	IsekaiAbilitySystemComponent = CreateDefaultSubobject<UIsekaiAbilitySystemComponent>(TEXT("IsekaiAbilitySystemComponent"));
	IsekaiAttributeSet = CreateDefaultSubobject<UIsekaiAttributeSet>(TEXT("IsekaiAttributeSet"));
	
	IsekaiAbilitySystemComponent->SetIsReplicated(true);
	IsekaiAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

UAbilitySystemComponent* AIsekaiPlayerState::GetAbilitySystemComponent() const
{
	return IsekaiAbilitySystemComponent;
}

void AIsekaiPlayerState::ApplyStartupAbilitySets()
{
	if (!HasAuthority() || !IsekaiAbilitySystemComponent)
	{
		return;
	}
	
	if (bStartupAbilitySetsApplied)
	{
		return;
	}
	
	for (UIsekaiAbilitySet* AbilitySet : StartupAbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->GiveToAbilitySystem(IsekaiAbilitySystemComponent, &StartupAbilitySetHandles, this);
		}
	}
	bStartupAbilitySetsApplied = true;
}

void AIsekaiPlayerState::ResetForRespawn()
{
	if (!HasAuthority() || !IsekaiAbilitySystemComponent || !IsekaiAttributeSet)
	{
		return;
	}
	
	// Remove lingering transient effects
	FGameplayTagContainer PersistentTags;
	PersistentTags.AddTag(Tags::Effect::PersistentAcrossRespawn);

	const FGameplayEffectQuery RemoveNonPersistentQuery = FGameplayEffectQuery::MakeQuery_MatchNoEffectTags(PersistentTags);
	IsekaiAbilitySystemComponent->RemoveActiveEffects(RemoveNonPersistentQuery);
	
	// Clear transient state effects handles
	IsekaiAttributeSet->InvalidateTransientEffectHandles();
	
	// Clear death tag
	IsekaiAbilitySystemComponent->RemoveReplicatedLooseGameplayTag(Tags::State::Dead);
	
	// Restore core attributes using your default-attributes GE
	if (RespawnAttributesEffect)
	{
		FGameplayEffectContextHandle Ctx = IsekaiAbilitySystemComponent->MakeEffectContext();
		Ctx.AddSourceObject(this);

		FGameplayEffectSpecHandle Spec = IsekaiAbilitySystemComponent->MakeOutgoingSpec(RespawnAttributesEffect, 1.f, Ctx);
		if (Spec.IsValid())
		{
			IsekaiAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}
	else
	{
		// Fallback: direct set
		IsekaiAttributeSet->SetHealth(IsekaiAttributeSet->GetMaxHealth());
		IsekaiAttributeSet->SetStamina(IsekaiAttributeSet->GetMaxStamina());
		
		UE_LOG(LogIsekaiAbilitySystem, Warning, TEXT("RespawnAttributesEffect is not set on %s; defaulting to direct attribute set."), *GetName());
	}

	// Safety net: kill any lingering actives
	IsekaiAbilitySystemComponent->CancelAllAbilities();

	// Push state quickly
	IsekaiAbilitySystemComponent->ForceReplication();
	ForceNetUpdate();
}
