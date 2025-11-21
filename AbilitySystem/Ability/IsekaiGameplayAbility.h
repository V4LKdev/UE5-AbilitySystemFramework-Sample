// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AIAssessment/AbilitySystem/IsekaiAttributeSet.h"
#include "IsekaiGameplayAbility.generated.h"

class UIsekaiAbilitySystemComponent;
class AIsekaiCharacterBase;

/**
 * How this ability chooses to react to input and lifecycle.
 */
UENUM()
enum EIsekaiAbilityActivationPolicy : uint8
{
	OnInputTriggered UMETA(DisplayName = "On Input Triggered"),
	WhileInputActive UMETA(DisplayName = "While Input Active"),
	OnSpawn UMETA(DisplayName = "On Spawn") // For passives
};

/**
 * Base gameplay ability for the Isekai project.
 *
 * Adds:
 * - A configurable activation policy (OnInputTriggered/WhileInputActive/OnSpawn).
 * - Typed accessors to the owning Isekai character, ASC, and attribute set.
 * - Default input handling that ends WhileInputActive abilities on local input release.
 */
UCLASS()
class AIASSESSMENT_API UIsekaiGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UIsekaiGameplayAbility();
	
	/** Returns this ability's activation policy used by the custom ASC when processing input. */
	EIsekaiAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	
protected:
	/** Typed convenience accessor for the owning Isekai character (AvatarActor). */
	AIsekaiCharacterBase* GetIsekaiCharacter() const;
	/** Typed convenience accessor for the owning Isekai ASC. */
	UIsekaiAbilitySystemComponent* GetIsekaiAbilitySystemComponent() const;
	/** Typed convenience accessor for the shared Isekai attribute set. */
	const UIsekaiAttributeSet* GetIsekaiAttributeSet() const;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Isekai|Ability")
	TEnumAsByte<EIsekaiAbilityActivationPolicy> ActivationPolicy = OnInputTriggered;
	
	/**
	 * Called when input is released for this ability.
	 * For WhileInputActive abilities on the locally controlled pawn, this will end the ability.
	 */
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
};
