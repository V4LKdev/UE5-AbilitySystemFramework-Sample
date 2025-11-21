// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "IsekaiAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

class UIsekaiAbilitySystemComponent;
/**
 * Core shared attributes for player- and AI-controlled characters.
 *
 * Owns health, stamina and their regen parameters, and drives stateful gameplay effects
 * such as Dead and Exhausted when thresholds are reached on the server.
 */
UCLASS()
class AIASSESSMENT_API UIsekaiAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UIsekaiAttributeSet();
	
	// UAttributeSet overrides
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	
	/**
	 * Clears transient gameplay effect handles (Dead / Exhausted) used across respawns.
	 * Does not modify attributes or remove active effects; see PlayerState for full respawn logic.
	 */
	void InvalidateTransientEffectHandles();
	
	
	// Health
	UPROPERTY(BlueprintReadOnly, Category="Isekai Attributes", ReplicatedUsing=OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UIsekaiAttributeSet, Health)
	
	UPROPERTY(BlueprintReadOnly, Category="Isekai Attributes", ReplicatedUsing=OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UIsekaiAttributeSet, MaxHealth)
	
	// Stamina
	UPROPERTY(BlueprintReadOnly, Category="Isekai Attributes", ReplicatedUsing=OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UIsekaiAttributeSet, Stamina)
	
	UPROPERTY(BlueprintReadOnly, Category="Isekai Attributes", ReplicatedUsing=OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UIsekaiAttributeSet, MaxStamina)
	
	
	UPROPERTY(BlueprintReadOnly, Category="Isekai Attributes", ReplicatedUsing=OnRep_HealthRegenRate)
	FGameplayAttributeData HealthRegenRate;
	ATTRIBUTE_ACCESSORS(UIsekaiAttributeSet, HealthRegenRate)
	
	UPROPERTY(BlueprintReadOnly, Category="Isekai Attributes", ReplicatedUsing=OnRep_HealthRegenDelay)
	FGameplayAttributeData HealthRegenDelay;
	ATTRIBUTE_ACCESSORS(UIsekaiAttributeSet, HealthRegenDelay)
	
	
	UPROPERTY(BlueprintReadOnly, Category="Isekai Attributes", ReplicatedUsing=OnRep_StaminaRegenRate)
	FGameplayAttributeData StaminaRegenRate;
	ATTRIBUTE_ACCESSORS(UIsekaiAttributeSet, StaminaRegenRate)
	
	UPROPERTY(BlueprintReadOnly, Category="Isekai Attributes", ReplicatedUsing=OnRep_StaminaRegenDelay)
	FGameplayAttributeData StaminaRegenDelay;
	ATTRIBUTE_ACCESSORS(UIsekaiAttributeSet, StaminaRegenDelay)
	
protected:
	// Replication notification callbacks
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_HealthRegenRate(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_HealthRegenDelay(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_StaminaRegenRate(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_StaminaRegenDelay(const FGameplayAttributeData& OldValue);
	
private:
	/**
	* Server-only: applies/removes the Exhausted state GE based on stamina thresholds.
	* @param TargetASC ASC that owns the attributes (PlayerState for players).
	* @param NewStamina clamped stamina after GE execution.
	*/
	void HandleExhaustionStateChange(UIsekaiAbilitySystemComponent* TargetASC, float NewStamina);
	
	/**
	* Server-only: applies the Dead state GE once when health reaches zero.
	* @param TargetASC ASC that owns the attributes.
	*/
	void HandleOutOfHealth(UIsekaiAbilitySystemComponent* TargetASC);
	
	FActiveGameplayEffectHandle ExhaustedStateEffectHandle;
	FActiveGameplayEffectHandle DeadStateEffectHandle;
};
