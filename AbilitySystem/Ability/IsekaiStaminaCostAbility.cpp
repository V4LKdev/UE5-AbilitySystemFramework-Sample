// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#include "IsekaiStaminaCostAbility.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "AIAssessment/NativeGameplayTags.h"
#include "AIAssessment/AbilitySystem/IsekaiAbilitySystemComponent.h"

// --- Ctor & Overrides ---
UIsekaiStaminaCostAbility::UIsekaiStaminaCostAbility()
{
    FGameplayTagContainer TagContainer;
    TagContainer.AddTag(Tags::Ability::Stamina);
    SetAssetTags(TagContainer);
}

bool UIsekaiStaminaCostAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Check if we have enough stamina for the initial cost
	return HasEnoughStamina(InitialStaminaCost, bAllowCostOverdraw);
}

bool UIsekaiStaminaCostAbility::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags)
{
	// First, check if we have enough stamina. This is a final check before committing.
	if (!HasEnoughStamina(InitialStaminaCost, bAllowCostOverdraw))
	{
		return false;
	}

	// Now, commit the ability with the base class. This will apply engine-level costs and cooldowns.
	if (!Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		return false;
	}

	// If the base commit was successful, apply our custom stamina cost.
	if (!ApplyInitialStaminaCost())
	{
		// This should ideally not fail if HasEnoughStamina passed, but as a safeguard:
		return false;
	}
	
	// After paying the cost, apply the regen Block
    SetRegenBlockedByActiveAbility(true);

	return true;
}

void UIsekaiStaminaCostAbility::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    // Stop any periodic drains
    StopAllStaminaCosts();

    // Clear regen-block-from-active-ability, if we set it
    SetRegenBlockedByActiveAbility(false);
    
    ApplyStaminaRegenDelay();

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// --- Stamina helpers ---
bool UIsekaiStaminaCostAbility::HasEnoughStamina(const float RequiredStamina, const bool bAllowOverdraw) const
{
    const UIsekaiAttributeSet* IsekaiAS = GetIsekaiAttributeSet();
    if (!IsekaiAS)
    {
        return false;
    }
    
    const float CurrentStamina = IsekaiAS->GetStamina();

    if (RequiredStamina <= 0.0f)
    {
        return true;
    }

    if (bAllowOverdraw)
    {
        // as long as we have some stamina, we allow the action
        return CurrentStamina > 0.0f;
    }

    return CurrentStamina >= RequiredStamina;
}

bool UIsekaiStaminaCostAbility::ApplyInitialStaminaCost()
{
    if (InitialStaminaCost <= 0.f || !InstantStaminaCostEffectClass)
    {
        return true;
    }

    const FActiveGameplayEffectHandle Handle =
        ApplyStaminaCostEffectInternal(
            InstantStaminaCostEffectClass,
            InitialStaminaCost,
            /*bTrackHandle*/ false  // instant; no need to clear later
        );
    
    return Handle.WasSuccessfullyApplied();
}

FActiveGameplayEffectHandle UIsekaiStaminaCostAbility::StartStaminaDrain()
{
    if (StaminaDrainPerSecond <= 0.f || !PeriodicStaminaDrainEffectClass)
    {
        return FActiveGameplayEffectHandle();
    }

    UIsekaiAbilitySystemComponent* IsekaiASC = GetIsekaiAbilitySystemComponent();
    if (!IsekaiASC)
    {
        return FActiveGameplayEffectHandle();
    }

    // Determine the GE's tick period from its CDO, so we can convert per-second cost to per-tick.
    const UGameplayEffect* DrainGECDO = PeriodicStaminaDrainEffectClass->GetDefaultObject<UGameplayEffect>();

    const float Period = (DrainGECDO && DrainGECDO->Period.Value > 0.f)
        ? DrainGECDO->Period.Value
        : 1.f; // fallback if misconfigured

    const float CostPerTick = StaminaDrainPerSecond * Period;

    StaminaDrainEffectHandle = ApplyStaminaCostEffectInternal(
        PeriodicStaminaDrainEffectClass,
        CostPerTick,
        /*bTrackHandle*/ true   // we want to remove this on EndAbility
    );

    return StaminaDrainEffectHandle;
}

void UIsekaiStaminaCostAbility::StopAllStaminaCosts()
{
    if (!StaminaDrainEffectHandle.IsValid())
    {
        return;
    }

    if (UIsekaiAbilitySystemComponent* IsekaiASC = GetIsekaiAbilitySystemComponent())
    {
        IsekaiASC->RemoveActiveGameplayEffect(StaminaDrainEffectHandle);
    }
    
    StaminaDrainEffectHandle.Invalidate();
}

void UIsekaiStaminaCostAbility::ApplyStaminaRegenDelay() const
{
    if (!StaminaRegenDelayEffectClass)
    {
        return;
    }

    UIsekaiAbilitySystemComponent* IsekaiASC = GetIsekaiAbilitySystemComponent();
    const UIsekaiAttributeSet* IsekaiAS = GetIsekaiAttributeSet();
    
    if (!IsekaiASC || !IsekaiAS)
    {
        return;
    }

    const float Delay = IsekaiAS->GetStaminaRegenDelay();

    FGameplayEffectContextHandle Context = IsekaiASC->MakeEffectContext();
    Context.AddSourceObject(this);

    FGameplayEffectSpecHandle SpecHandle =
        IsekaiASC->MakeOutgoingSpec(StaminaRegenDelayEffectClass, GetAbilityLevel(), Context);

    if (!SpecHandle.IsValid())
    {
        return;
    }

    if (Delay > 0.f)
    {
        SpecHandle.Data->SetDuration(Delay, /*bLockDuration*/ true);
    }

    // GE will grant State.Stamina.RegenBlocked.Delay and auto-clear when it expires.
    ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);
}

void UIsekaiStaminaCostAbility::SetRegenBlockedByActiveAbility(bool bBlocked) const
{
    UIsekaiAbilitySystemComponent* IsekaiASC = GetIsekaiAbilitySystemComponent();
    if (!IsekaiASC)
    {
        return;
    }

    if (bBlocked)
    {
        IsekaiASC->AddLooseGameplayTag(Tags::State::Stamina_RegenBlocked_ActiveAbility);
    }
    else
    {
        IsekaiASC->RemoveLooseGameplayTag(Tags::State::Stamina_RegenBlocked_ActiveAbility);
    }
}

FActiveGameplayEffectHandle UIsekaiStaminaCostAbility::ApplyStaminaCostEffectInternal(
    const TSubclassOf<UGameplayEffect> EffectClass,
    const float CostMagnitude,
    const bool bTrackHandle)
{
    if (!EffectClass)
    {
        return FActiveGameplayEffectHandle();
    }

    UIsekaiAbilitySystemComponent* IsekaiASC = GetIsekaiAbilitySystemComponent();
    if (!IsekaiASC)
    {
        return FActiveGameplayEffectHandle();
    }

    FGameplayEffectContextHandle Context = IsekaiASC->MakeEffectContext();
    Context.AddSourceObject(this);

    const FGameplayEffectSpecHandle SpecHandle =
        IsekaiASC->MakeOutgoingSpec(EffectClass, GetAbilityLevel(), Context);

    if (!SpecHandle.IsValid())
    {
        return FActiveGameplayEffectHandle();
    }

    // We treat CostMagnitude as a positive number, but the GE expects a negative modifier to reduce Stamina.
    SpecHandle.Data->SetSetByCallerMagnitude(Tags::Data::Cost_Stamina, -CostMagnitude); // so we negate here

    FActiveGameplayEffectHandle Handle =
        ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);


    if (!Handle.WasSuccessfullyApplied())
    {
        return FActiveGameplayEffectHandle();
    }

    if (bTrackHandle)
    {
        StaminaDrainEffectHandle = Handle;
    }
    
    return Handle;
}

