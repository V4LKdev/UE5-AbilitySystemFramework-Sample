// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIStealthComponent.h"
#include "AIAssessment/AI/IsekaiAITypes.h"
#include "Components/ActorComponent.h"
#include "IsekaiUIBridge.generated.h"


class UAIStealthComponent;
class UIsekaiAttributeSet;
class UIsekaiAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFloatAttributeChangedSignature, float, NewValue, float, MaxValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBoolStateChangedSignature, bool, bNewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFloatValueChangedSignature, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStealthStateChangedSignature, const FStealthStateData&, NewData);

/**
 * UI Data Bridge.
 * 
 * DESIGN:
 * Serves as a View Model between the Data Layer (ASC, AttributeSet, StealthComponent) and the View Layer (Widgets).
 * 
 * HYBRID USAGE:
 * - On Players: InitAbilitySystem() is called. InitStealthComponent() is NOT called.
 * - On AI: Both are called.
 */
UCLASS(ClassGroup=(UI), meta=(BlueprintSpawnableComponent))
class AIASSESSMENT_API UIsekaiUIBridge : public UActorComponent
{
	GENERATED_BODY()

public:
	UIsekaiUIBridge();
	
	// --- Initialization ---
	/** Initializes the bridges ASC bindings from the owning actor's ASC and AttributeSet. */
	void InitAbilitySystem(UIsekaiAbilitySystemComponent* InASC, UIsekaiAttributeSet* InAttributeSet);
	/** Initializes the bridge's StealthComponent bindings. */
	void InitStealthComponent(UAIStealthComponent* InStealthComponent);
	
	// --- Bindable Events ---
	UPROPERTY(BlueprintAssignable, Category="Isekai|UI")
	FOnFloatAttributeChangedSignature OnHealthChanged;
	UPROPERTY(BlueprintAssignable, Category="Isekai|UI")
	FOnFloatAttributeChangedSignature OnStaminaChanged;
	
	UPROPERTY(BlueprintAssignable, Category="Isekai|UI")
	FOnBoolStateChangedSignature OnExhaustedChanged;
	UPROPERTY(BlueprintAssignable, Category="Isekai|UI")
	FOnBoolStateChangedSignature OnSprintingChanged;
	
	UPROPERTY(BlueprintAssignable, Category="Isekai|UI")
	FOnStealthStateChangedSignature OnStealthStateDataChanged;
	
	// --- Getters for UI Binding ---
	UFUNCTION(BlueprintPure, Category="Isekai|UI")
	float GetHealth() const { return CachedHealth; }
	
	UFUNCTION(BlueprintPure, Category="Isekai|UI")
	float GetMaxHealth() const { return CachedMaxHealth; }
	
	UFUNCTION(BlueprintPure, Category="Isekai|UI")
	float GetHealthNormalized() const{ return (CachedMaxHealth > 0.f) ? CachedHealth / CachedMaxHealth : 0.f; }
	
	UFUNCTION(BlueprintPure, Category = "Isekai|UI")
	float GetStamina() const { return CachedStamina; }

	UFUNCTION(BlueprintPure, Category = "Isekai|UI")
	float GetMaxStamina() const { return CachedMaxStamina; }

	UFUNCTION(BlueprintPure, Category = "Isekai|UI")
	float GetStaminaNormalized() const{ return (CachedMaxStamina > 0.f) ? CachedStamina / CachedMaxStamina : 0.f; }

	UFUNCTION(BlueprintPure, Category = "Isekai|UI")
	bool IsExhausted() const { return bIsExhausted; }

	UFUNCTION(BlueprintPure, Category = "Isekai|UI")
	bool IsSprinting() const { return bIsSprinting; }
	
	UFUNCTION(BlueprintPure, Category = "Isekai|UI")
	float GetAlertValue() const { return CachedStealthStateData.AlertValue; }
	
	UFUNCTION(BlueprintPure, Category = "Isekai|UI")
	EStealthState GetStealthState() const { return CachedStealthStateData.CurrentState; }
	
	UFUNCTION(BlueprintPure, Category = "Isekai|UI")
	FString GetStealthStateAsString() const;
	
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// --- Event Handlers ---
	void HandleHealthChanged(const struct FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const struct FOnAttributeChangeData& Data);
	void HandleStaminaChanged(const struct FOnAttributeChangeData& Data);
	void HandleMaxStaminaChanged(const struct FOnAttributeChangeData& Data);
	void HandleExhaustedTagChanged(const struct FGameplayTag Tag, int32 NewCount);
	void HandleSprintingTagChanged(const struct FGameplayTag Tag, int32 NewCount);
	void HandleStealthStateDataChanged(const FStealthStateData& NewData);

	// --- Broadcast Helpers ---
	void BroadcastHealth();
	void BroadcastStamina();
	void BroadcastExhausted();
	void BroadcastSprinting();
	void BroadcastStealthStateData();
	
	// --- Internal State ---
	TWeakObjectPtr<UIsekaiAbilitySystemComponent> AbilitySystem;
	TWeakObjectPtr<UIsekaiAttributeSet> AttributeSet;
	TWeakObjectPtr<UAIStealthComponent> StealthComponent;

	// --- State Cache ---
	float CachedHealth = 0.f;
	float CachedMaxHealth = 0.f;
	float CachedStamina = 0.f;
	float CachedMaxStamina = 0.f;
	bool bIsExhausted = false;
	bool bIsSprinting = false;
	FStealthStateData CachedStealthStateData;
	
	// --- Delegate Handles ---
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
	FDelegateHandle StaminaChangedHandle;
	FDelegateHandle MaxStaminaChangedHandle;
	FDelegateHandle ExhaustedTagHandle;
	FDelegateHandle SprintingTagHandle;
	FDelegateHandle StealthStateDataChangedHandle;
};
