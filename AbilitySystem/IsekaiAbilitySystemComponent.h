// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "IsekaiAbilitySystemComponent.generated.h"

/**
 * Custom Ability System Component for Isekai.
 *
 * Responsibilities:
 * - Map input gameplay tags to ability specs and drive activation based on activation policy.
 * - Forward input pressed/released events to abilities for prediction-safe handling.
 * - Provide a central hook for shutting down all abilities when the owner runs out of health.
 */
UCLASS(ClassGroup=(GAS), meta=(BlueprintSpawnableComponent))
class AIASSESSMENT_API UIsekaiAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UIsekaiAbilitySystemComponent();
	
	/**
	 * Registers that the given input tag was pressed this frame.
	 * Called by the owning PlayerController from input bindings.
	 */
	void AbilityInputTagPressed(const FGameplayTag& InputTag);

	/**
	 * Registers that the given input tag was released this frame.
	 * Called by the owning PlayerController from input bindings.
	 */
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	
	/**
	 * Processes all pending input tags (pressed/held/released) and activates/updates abilities accordingly.
	 * Should be called once per frame from the owning PlayerController (PostProcessInput).
	 */
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	
	/**
	 * Called by the AttributeSet when Health reaches zero.
	 * Clears all buffered input and cancels all active abilities on this ASC.
	 */
	void HandleOutOfHealth();

protected:
	/** Forwards input-pressed events to active abilities using generic replicated events. */
	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	/** Forwards input-released events to active abilities using generic replicated events. */
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;
	
	/** Clears all internal input handle arrays (pressed/held/released). */
	void ClearAbilityInput();
	
	/** Spec handles for abilities whose input tag was pressed this frame. */
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;
	
	/** Spec handles for abilities whose input tag was released this frame. */
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;
	
	/** Spec handles for abilities whose input is currently held down. */
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
};
