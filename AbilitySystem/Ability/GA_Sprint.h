// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "IsekaiStaminaCostAbility.h"
#include "GA_Sprint.generated.h"

class UIsekaiCharacterMovementComponent;
class UAbilityTask_ApplyRootMotionConstantForce;
/**
 * Sprint ability.
 *
 * - Press/hold input to increase movement speed at the cost of stamina.
 * - Optionally applies an initial root-motion dash when activated.
 * - Drains stamina over time via StaminaDrainPerSecond while sprinting and moving.
 * - Cancels when input is released, movement state changes, or exhaustion triggers.
 */
UCLASS()
class AIASSESSMENT_API UGA_Sprint : public UIsekaiStaminaCostAbility
{
	GENERATED_BODY()
public:
	UGA_Sprint();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash")
	bool bUseInitialDash = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash", meta = (EditCondition = "bUseInitialDash"))
	float InitialDashStrength = 1500.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash", meta = (EditCondition = "bUseInitialDash"))
	float InitialDashDuration = 0.2f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash", meta = (EditCondition = "bUseInitialDash"))
	float InitialDashEndAirVelocityMultiplier = 0.8f;
	
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
private:
	FDelegateHandle MovementStateChangedHandle;
	
	// Called when movement mode changes (e.g., walking, falling, swimming)
	UFUNCTION()
	void HandleMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode);
	
	// Called when IsMoving changes
	void HandleMovementStateChanged(bool bIsMoving);
	
	void ApplyInitialDash();
	
	bool bDuringInitialDash = false;
	bool bEndAbilityQueued = false;
	bool bWasEndAbilityCancelled = false;
	bool bReplicateQueuedEndAbility = false;
	
	UFUNCTION()
	void OnInitialDashFinished();
	
	bool GetSprintContext(AIsekaiCharacterBase*& OutChar, UIsekaiCharacterMovementComponent*& OutMoveComp) const;
};