// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "AIAssessment/Component/IsekaiUIBridge.h"
#include "IsekaiCharacterBase.generated.h"

class UIsekaiCharacterMovementComponent;
class UIsekaiAttributeSet;
class UIsekaiAbilitySystemComponent;

/**
 * GAS-enabled base character class.
 *
 * Player and AI subclasses should inherit from this.
 * Does not own the ASC or AttributeSet; instead, holds pointers and wires them up
 * to the correct owning Actor (typically the PlayerState for players).
 */
UCLASS(Abstract)
class AIASSESSMENT_API AIsekaiCharacterBase : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	AIsekaiCharacterBase(const FObjectInitializer& ObjectInitializer);
	
	// IAbilitySystemInterface implementation
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	
	UFUNCTION(BlueprintPure, Category="Isekai|UI")
	UIsekaiUIBridge* GetUIBridge() const { return UIBridge; }
	UFUNCTION(BlueprintPure, Category="Isekai|Movement")
	UIsekaiCharacterMovementComponent* GetIsekaiCharacterMovement() const;
	
	
	// --- Generic Team Interface ---
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	
	/**
	 * Server-authoritative death entry point.
	 * Called when Health reaches 0. Safe to call multiple times; will early-out if already dead.
	 */
	virtual void HandleOutOfHealth();
	
	// Attribute Helper
	UFUNCTION(BlueprintPure, Category="Isekai|Attributes")
	float GetHealth() const;
	UFUNCTION(BlueprintPure, Category="Isekai|Attributes")
	float GetMaxHealth() const;
	UFUNCTION(BlueprintPure, Category="Isekai|Attributes")
	float GetStamina() const;
	UFUNCTION(BlueprintPure, Category="Isekai|Attributes")
	float GetMaxStamina() const;
	UFUNCTION(BlueprintPure, Category="Isekai|Attributes")
	bool IsDead() const { return bIsDead; }
	
	UFUNCTION(BlueprintImplementableEvent, Category="Isekai|Attributes")
	void BP_OnDeath();
protected:
	virtual void BeginPlay() override;
	/**
	 * Initializes AbilitySystem/AttributeSet pointers and actor info for this character.
	 * Typically called from the owning PlayerState once the ASC/attributes are created.
	 */
	void InitAbilitySystem(AActor* Owner, UIsekaiAbilitySystemComponent* InIsekaiASC, UIsekaiAttributeSet* InIsekaiAS);

	/** Called on server and clients when death state is entered to apply local visuals/physics. */
	virtual void HandleVisualsOnDeath();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UIsekaiUIBridge> UIBridge;

	UPROPERTY(ReplicatedUsing=OnRep_TeamID, EditAnywhere, BlueprintReadOnly, Category="Isekai|Team")
	TEnumAsByte<EIsekaiTeamID> TeamID;
	FGenericTeamId TeamIdStruct;
	
	UPROPERTY(Transient)
	TObjectPtr<UIsekaiAbilitySystemComponent> IsekaiAbilitySystemComponent;
	UPROPERTY(Transient)
	TObjectPtr<UIsekaiAttributeSet> IsekaiAttributeSet;
	
	UPROPERTY(ReplicatedUsing=OnRep_IsDead)
	bool bIsDead = false;
	
	/** Replication hook for bIsDead. Triggers local death visuals. */
	UFUNCTION()
	void OnRep_IsDead();
	UFUNCTION()
	void OnRep_TeamID();
};
