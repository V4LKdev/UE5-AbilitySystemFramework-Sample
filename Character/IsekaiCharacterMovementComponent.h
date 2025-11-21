// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIAssessment/AbilitySystem/IsekaiAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "IsekaiCharacterMovementComponent.generated.h"


class UIsekaiAbilitySystemComponent;
class AIsekaiCharacterBase;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnIsMovingChangedSignature, bool bIsMoving);

/**
 * Custom CharacterMovementComponent for Isekai characters.
 *
 * Owns movement-related tuning (walk/sprint/exhausted speeds) and integrates
 * with the Ability System via tags (e.g., Exhausted) and delegates (OnIsMovingChanged).
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AIASSESSMENT_API UIsekaiCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UIsekaiCharacterMovementComponent();
	
	// === Public API for abilities / character ===
	
	/** Initialize cached references to the owning character and its ASC. */
	void InitializeIsekaiReferences(AIsekaiCharacterBase* InCharacter, UIsekaiAbilitySystemComponent* InASC);

	/** Begin sprinting (if allowed). Should be called by a sprint ability. */
	void StartSprinting();

	/** Stop sprinting. */
	void StopSprinting();

	/** Returns true if this character currently wants to sprint. */
	bool IsSprinting() const { return bWantsToSprint; }

	/** Manually set exhaustion state based on ASC tags. */
	void SetIsExhausted(bool bNewExhausted);

	bool IsExhausted() const { return bIsExhausted; }

	// === UMovementComponent / UCharacterMovementComponent ===
	
	/** Combines base walk speed with sprint/exhausted modifiers and root motion. */
	virtual float GetMaxSpeed() const override;
	
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	
	/** Convenience accessor for the current sprint speed (BaseWalkSpeed * SprintSpeedMultiplier). */
	float GetSprintSpeed() const
	{
		return BaseWalkSpeed * SprintSpeedMultiplier;
	}
	
	/** True if current velocity exceeds MovingThreshold. */
	bool IsMoving() const { return bIsMoving; }
	
	/** Broadcast whenever IsMoving changes between true/false. */
	FOnIsMovingChangedSignature OnIsMovingChanged;
	
protected:
	virtual void AddInputVector(FVector WorldVector, bool bForce = false) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	bool IsRootMotionActive() const;
	
	// === Configurable base numbers ===

	/** Baseline ground speed for this character (no sprint, no debuffs). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Isekai|Movement", meta = (ClampMin = "0.0"))
	float BaseWalkSpeed = 600.f;

	/** Multiplier applied when sprinting */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Isekai|Movement", meta = (ClampMin = "1.0"))
	float SprintSpeedMultiplier = 1.5f;

	/** Multiplier applied when exhausted*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Isekai|Movement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ExhaustedSpeedMultiplier = 0.7f;
	

	// === Internal state flags ===

	/** Set by abilities / input when player wants to sprint. */
	UPROPERTY(Transient)
	uint8 bWantsToSprint : 1;

	/** True if exhausted */
	UPROPERTY(Transient)
	uint8 bIsExhausted : 1;

	// === Cached references ===

	UPROPERTY(Transient)
	TObjectPtr<AIsekaiCharacterBase> CachedIsekaiCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UIsekaiAbilitySystemComponent> CachedASC;
	
	UPROPERTY(Transient)
	bool bIsMoving;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Isekai|Movement")
	float MovingThreshold = 5.f;
	
	FDelegateHandle ExhaustedTagChangedHandle;
	void HandleExhaustedTagChanged(const FGameplayTag Tag, int32 NewCount);
};
