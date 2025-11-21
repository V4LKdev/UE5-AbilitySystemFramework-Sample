// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IsekaiUIBridge.generated.h"


class UIsekaiAttributeSet;
class UIsekaiAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFloatAttributeChangedSignature, float, NewValue, float, MaxValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBoolStateChangedSignature, bool, bNewValue);

/**
 * UI bridge component for GAS attributes and state.
 *
 * Subscribes to AttributeSet and ASC events (attributes + tags) and exposes
 * clean Blueprint delegates and getters for UI widgets to bind against.
 */
UCLASS(ClassGroup=(UI), meta=(BlueprintSpawnableComponent))
class AIASSESSMENT_API UIsekaiUIBridge : public UActorComponent
{
	GENERATED_BODY()

public:
	UIsekaiUIBridge();
	
	/** Initializes the bridge from the owning actor's ASC and AttributeSet. */
	void InitFromAbilitySystem(UIsekaiAbilitySystemComponent* InASC, UIsekaiAttributeSet* InAttributeSet);
	
	// Delegates for UI Binding
	
	UPROPERTY(BlueprintAssignable, Category="Isekai|UI")
	FOnFloatAttributeChangedSignature OnHealthChanged;
	UPROPERTY(BlueprintAssignable, Category="Isekai|UI")
	FOnFloatAttributeChangedSignature OnStaminaChanged;
	
	UPROPERTY(BlueprintAssignable, Category="Isekai|UI")
	FOnBoolStateChangedSignature OnExhaustedChanged;
	UPROPERTY(BlueprintAssignable, Category="Isekai|UI")
	FOnBoolStateChangedSignature OnSprintingChanged;
	
	// BP Getters
	UFUNCTION(BlueprintPure, Category="Isekai|UI")
	float GetHealth() const { return CachedHealth; }
	
	UFUNCTION(BlueprintPure, Category="Isekai|UI")
	float GetMaxHealth() const { return CachedMaxHealth; }
	
	UFUNCTION(BlueprintPure, Category="Isekai|UI")
	float GetHealthNormalized() const
	{
		return (CachedMaxHealth > 0.f) ? CachedHealth / CachedMaxHealth : 0.f;
	}
	
	UFUNCTION(BlueprintPure, Category = "Isekai|UI")
	float GetStamina() const { return CachedStamina; }

	UFUNCTION(BlueprintPure, Category = "Isekai|UI")
	float GetMaxStamina() const { return CachedMaxStamina; }

	UFUNCTION(BlueprintPure, Category = "Isekai|UI")
	float GetStaminaNormalized() const
	{
		return (CachedMaxStamina > 0.f) ? CachedStamina / CachedMaxStamina : 0.f;
	}

	UFUNCTION(BlueprintPure, Category = "Isekai|UI")
	bool IsExhausted() const { return bIsExhausted; }

	UFUNCTION(BlueprintPure, Category = "Isekai|UI")
	bool IsSprinting() const { return bIsSprinting; }
	
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Internal handlers
	void HandleHealthChanged(const struct FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const struct FOnAttributeChangeData& Data);
	void HandleStaminaChanged(const struct FOnAttributeChangeData& Data);
	void HandleMaxStaminaChanged(const struct FOnAttributeChangeData& Data);

	void HandleExhaustedTagChanged(const struct FGameplayTag Tag, int32 NewCount);
	void HandleSprintingTagChanged(const struct FGameplayTag Tag, int32 NewCount);

	// Broadcast helpers
	void BroadcastHealth();
	void BroadcastStamina();
	void BroadcastExhausted();
	void BroadcastSprinting();
	
	
	UPROPERTY()
	TObjectPtr<UIsekaiAbilitySystemComponent> AbilitySystem = nullptr;

	UPROPERTY()
	TObjectPtr<UIsekaiAttributeSet> AttributeSet = nullptr;

	// Cached values for UI
	UPROPERTY(VisibleInstanceOnly, Category = "Isekai|UI")
	float CachedHealth = 0.f;

	UPROPERTY(VisibleInstanceOnly, Category = "Isekai|UI")
	float CachedMaxHealth = 0.f;

	UPROPERTY(VisibleInstanceOnly, Category = "Isekai|UI")
	float CachedStamina = 0.f;

	UPROPERTY(VisibleInstanceOnly, Category = "Isekai|UI")
	float CachedMaxStamina = 0.f;

	UPROPERTY(VisibleInstanceOnly, Category = "Isekai|UI")
	bool bIsExhausted = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Isekai|UI")
	bool bIsSprinting = false;
	
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
	FDelegateHandle StaminaChangedHandle;
	FDelegateHandle MaxStaminaChangedHandle;

	FDelegateHandle ExhaustedTagHandle;
	FDelegateHandle SprintingTagHandle;
};
