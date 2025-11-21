// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "IsekaiGameplayAbility.h"
#include "IsekaiStaminaCostAbility.generated.h"

/**
 * Base class for any ability that spends stamina or reacts to stamina/exhaustion state.
 *
 * Examples: Jump, Sprint, melee attacks that use stamina, etc.
 * Centralizes stamina cost checks, cost application (instant and periodic), and regen blocking.
 */
UCLASS()
class AIASSESSMENT_API UIsekaiStaminaCostAbility : public UIsekaiGameplayAbility
{
	GENERATED_BODY()
public:
    UIsekaiStaminaCostAbility();

protected:
	// --- UGameplayAbility overrides ---
	
	/** Performs standard ability checks plus stamina-specific checks before activation. */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const override;
	/** Commits base ability cost/cooldown and applies configured stamina costs. */
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;
	/** Cleans up stamina cost effects and any regen-blocking state on ability end. */
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;
	
	// --- Config ---
	
    /** One-shot stamina cost paid when the ability activates (e.g. Jump). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stamina", meta=(ClampMin="0.0"))
    float InitialStaminaCost = 0.0f;

    /** Continuous drain in stamina units per second while the ability is active (e.g. Sprint). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stamina", meta=(ClampMin="0.0"))
    float StaminaDrainPerSecond = 0.0f;

    /** GE used for instant stamina cost (expects SetByCaller(Data.Cost.Stamina), Instant). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stamina")
    TSubclassOf<UGameplayEffect> InstantStaminaCostEffectClass;

    /** GE used for periodic stamina drain (expects SetByCaller(Data.Cost.Stamina), Periodic). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stamina")
    TSubclassOf<UGameplayEffect> PeriodicStaminaDrainEffectClass;

    /** GE that blocks stamina regen for StaminaRegenDelay seconds (grants State.Stamina.RegenBlocked.Delay). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stamina")
    TSubclassOf<UGameplayEffect> StaminaRegenDelayEffectClass;
	
	/** If true, allows stamina to go negative when paying costs (e.g. last-ditch actions). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stamina")
	bool bAllowCostOverdraw = false;
	
    // --- Stamina helpers ---

    /** Returns true if current stamina can afford RequiredStamina (optionally allowing overdraw). */
    bool HasEnoughStamina(float RequiredStamina, bool bAllowOverdraw = false) const;

    /** Applies InitialStaminaCost once; returns true on success or if no initial cost is configured. */
    bool ApplyInitialStaminaCost();

    /** Starts a periodic stamina drain effect and tracks its handle. */
    FActiveGameplayEffectHandle StartStaminaDrain();

    /** Stops all active stamina cost effects applied by this ability instance. */
    void StopAllStaminaCosts();

    /** Applies or refreshes the regen-delay GE, preventing stamina regen for a short time. */
    void ApplyStaminaRegenDelay() const;

    /** Adds or removes a tag indicating regen is blocked due to an active stamina ability. */
    void SetRegenBlockedByActiveAbility(bool bBlocked) const;

    /** Low-level helper that applies a stamina cost GE with SetByCaller(Data.Cost.Stamina) = -CostMagnitude. */
    FActiveGameplayEffectHandle ApplyStaminaCostEffectInternal(
        TSubclassOf<UGameplayEffect> EffectClass,
        float CostMagnitude,
        bool bTrackHandle
    );

    // --- Internal state ---

    /** Handle for the active periodic stamina drain effect, if any. */
    UPROPERTY(Transient)
    FActiveGameplayEffectHandle StaminaDrainEffectHandle;
};
