// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "IsekaiAbilitySystemComponent.h"

#include "Ability/IsekaiGameplayAbility.h"
#include "AIAssessment/NativeGameplayTags.h"
#include "AIAssessment/IsekaiLoggingChannels.h"

// --- Ctor & Overrides ---
UIsekaiAbilitySystemComponent::UIsekaiAbilitySystemComponent()
{

}

void UIsekaiAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);
	
	if (!Spec.IsActive())
	{
		return;
	}
	
	if (const UGameplayAbility* Ability = Spec.GetPrimaryInstance())
	{
		FPredictionKey PredKey = Ability->GetCurrentActivationInfo().GetActivationPredictionKey();
		
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, PredKey);
	}
}

void UIsekaiAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);
	
	if (!Spec.IsActive())
	{
		return;
	}
	
	if (const UGameplayAbility* Ability = Spec.GetPrimaryInstance())
	{
		FPredictionKey PredKey = Ability->GetCurrentActivationInfo().GetActivationPredictionKey();
		
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, PredKey);
	}
}

// --- Public API ---
void UIsekaiAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		UE_LOG(LogIsekaiAbilitySystem, Verbose, TEXT("AbilityInputTagPressed called with invalid tag"));
		return;
	}
	
	ABILITYLIST_SCOPE_LOCK();
	
	int32 MatchedAbilities = 0;

	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.Ability)
		{
			continue;
		}
		
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			MatchedAbilities++;
			InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
			InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
		}
	}

	if (MatchedAbilities == 0)
	{
		UE_LOG(LogIsekaiAbilitySystem, Verbose,
			TEXT("AbilityInputTagPressed: No abilities bound to input tag %s on %s"),
			*InputTag.ToString(),
			*GetName());
	}
}

void UIsekaiAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		UE_LOG(LogIsekaiAbilitySystem, Verbose, TEXT("AbilityInputTagReleased called with invalid tag"));
		return;
	}
	
	ABILITYLIST_SCOPE_LOCK();

	int32 MatchedAbilities = 0;

	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.Ability)
		{
			continue;
		}

		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			MatchedAbilities++;
			InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
			InputHeldSpecHandles.Remove(AbilitySpec.Handle);
		}
	}

	if (MatchedAbilities == 0)
	{
		UE_LOG(LogIsekaiAbilitySystem, Verbose,
			TEXT("AbilityInputTagReleased: No abilities bound to input tag %s on %s"),
			*InputTag.ToString(),
			*GetName());
	}
}

void UIsekaiAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if (HasMatchingGameplayTag(Tags::State::AbilityInputBlocked))
	{
		ClearAbilityInput();
		return;
	}
	
	ABILITYLIST_SCOPE_LOCK();
	
	TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	
	// 1. Handle held inputs
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(SpecHandle))
		{
			const UIsekaiGameplayAbility* AbilityCDO =
				Cast<UIsekaiGameplayAbility>(Spec->Ability);
			
			if (AbilityCDO && AbilityCDO->GetActivationPolicy() == EIsekaiAbilityActivationPolicy::WhileInputActive)
			{
				if (!Spec->IsActive())
				{
					AbilitiesToActivate.AddUnique(SpecHandle);
				}
			}
		}
	}
	
	// 2. Handle newly pressed inputs
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (Spec->Ability)
			{
				Spec->InputPressed = true;
				
				if (Spec->IsActive())
				{
					AbilitySpecInputPressed(*Spec);
				}
				else
				{
					const UIsekaiGameplayAbility* AbilityCDO =
						Cast<UIsekaiGameplayAbility>(Spec->Ability);
					
					if (AbilityCDO && AbilityCDO->GetActivationPolicy() == EIsekaiAbilityActivationPolicy::OnInputTriggered)
					{
						AbilitiesToActivate.AddUnique(SpecHandle);
					}
				}
			}
		}
	}
	
	// 3. Activate Queued Abilities
	for (const FGameplayAbilitySpecHandle& SpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(SpecHandle);
	}
	
	// 4. Handle released inputs
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (Spec->Ability)
			{
				Spec->InputPressed = false;
				
				if (Spec->IsActive())
				{
					AbilitySpecInputReleased(*Spec);
				}
			}
		}
	}
	

	// Clear pressed and released inputs
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UIsekaiAbilitySystemComponent::HandleOutOfHealth()
{
	UE_LOG(LogIsekaiAbilitySystem, Log,
		TEXT("UIsekaiAbilitySystemComponent::HandleOutOfHealth on %s - clearing input and cancelling abilities."),
		*GetName());

	ClearAbilityInput();
	CancelAllAbilities();
}

// --- Private API ---
void UIsekaiAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}
