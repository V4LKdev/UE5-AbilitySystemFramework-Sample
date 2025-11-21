// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AIAssessment/AbilitySystem/IsekaiAbilitySet.h"
#include "GameFramework/PlayerState.h"
#include "IsekaiPlayerState.generated.h"

class UGameplayEffect;
class UIsekaiGameplayAbility;
class UIsekaiAttributeSet;
class UIsekaiAbilitySystemComponent;

/**
 * PlayerState that owns the Ability System Component and Attribute Set for the player.
 *
 * Lives for the duration of the connection and persists across pawn respawns.
 * Responsible for:
 * - Creating and replicating the Isekai ASC and AttributeSet.
 * - Applying startup ability sets once on the server.
 * - Resetting attributes/effects for a fresh life on respawn.
 */
UCLASS()
class AIASSESSMENT_API AIsekaiPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	AIsekaiPlayerState();
	
	// IAbilitySystemInterface implementation
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	/** Typed accessor for this player's Isekai ASC. */
	UIsekaiAbilitySystemComponent* GetIsekaiAbilitySystemComponent() const {
		return IsekaiAbilitySystemComponent;
	}
	/** Typed accessor for this player's Isekai attribute set. */
	UIsekaiAttributeSet* GetIsekaiAttributeSet() const {
		return IsekaiAttributeSet;
	}
	
	/**
	 * Server-only: grants all configured StartupAbilitySets to the ASC.
	 * Safe to call multiple times; will only apply once.
	 */
	void ApplyStartupAbilitySets();
	
	/**
	 * Server-only: clears transient effects and restores core attributes for a fresh life.
	 * Used as part of the respawn flow.
	 */
	void ResetForRespawn();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Isekai|AbilitySystem")
	TObjectPtr<UIsekaiAbilitySystemComponent> IsekaiAbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Isekai|AbilitySystem")
	TObjectPtr<UIsekaiAttributeSet> IsekaiAttributeSet;
	
	UPROPERTY(EditDefaultsOnly, Category="Isekai|AbilitySystem")
	TArray<TObjectPtr<UIsekaiAbilitySet>> StartupAbilitySets;
	
	UPROPERTY(EditDefaultsOnly, Category="Isekai|AbilitySystem")
	TSubclassOf<UGameplayEffect> RespawnAttributesEffect;
	
	FIsekaiAbilitySetGrantedHandles StartupAbilitySetHandles;
	
	UPROPERTY()
	bool bStartupAbilitySetsApplied = false;
};
