// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "IsekaiUIBridge.h"

#include "AIStealthComponent.h"
#include "AIAssessment/AbilitySystem/IsekaiAttributeSet.h"
#include "AIAssessment/AbilitySystem/IsekaiAbilitySystemComponent.h"
#include "AIAssessment/NativeGameplayTags.h"


UIsekaiUIBridge::UIsekaiUIBridge()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UIsekaiUIBridge::InitAbilitySystem(UIsekaiAbilitySystemComponent* InASC, UIsekaiAttributeSet* InAttributeSet)
{
	if (!InAttributeSet || !InASC)
	{
		return;
	}
	
	AbilitySystem = InASC;
	AttributeSet = InAttributeSet;
	
	// Seed Cache
	CachedHealth = AttributeSet->GetHealth();
	CachedMaxHealth = AttributeSet->GetMaxHealth();
	CachedStamina = AttributeSet->GetStamina();
	CachedMaxStamina = AttributeSet->GetMaxStamina();
	bIsExhausted = AbilitySystem->HasMatchingGameplayTag(Tags::State::Exhausted);
	bIsSprinting = AbilitySystem->HasMatchingGameplayTag(Tags::State::Movement_Sprinting);
	
	BroadcastHealth();
	BroadcastStamina();
	BroadcastExhausted();
	BroadcastSprinting();
	
	// Subscribe to changes
	HealthChangedHandle = 
		AbilitySystem->GetGameplayAttributeValueChangeDelegate(UIsekaiAttributeSet::GetHealthAttribute())
		.AddUObject(this, &ThisClass::HandleHealthChanged);
	
	MaxHealthChangedHandle = 
		AbilitySystem->GetGameplayAttributeValueChangeDelegate(UIsekaiAttributeSet::GetMaxHealthAttribute())
		.AddUObject(this, &ThisClass::HandleMaxHealthChanged);
	
	StaminaChangedHandle = 
		AbilitySystem->GetGameplayAttributeValueChangeDelegate(UIsekaiAttributeSet::GetStaminaAttribute())
		.AddUObject(this, &ThisClass::HandleStaminaChanged);
	
	MaxStaminaChangedHandle = 
		AbilitySystem->GetGameplayAttributeValueChangeDelegate(UIsekaiAttributeSet::GetMaxStaminaAttribute())
		.AddUObject(this, &ThisClass::HandleMaxStaminaChanged);
	
	// Tag Events
	
	ExhaustedTagHandle = 
		AbilitySystem->RegisterGameplayTagEvent(Tags::State::Exhausted)
		.AddUObject(this, &ThisClass::HandleExhaustedTagChanged);
	
	SprintingTagHandle = 
		AbilitySystem->RegisterGameplayTagEvent(Tags::State::Movement_Sprinting)
		.AddUObject(this, &ThisClass::HandleSprintingTagChanged);
}

void UIsekaiUIBridge::InitStealthComponent(UAIStealthComponent* InStealthComponent)
{
	if (!InStealthComponent)
	{
		return;
	}
	
	StealthComponent = InStealthComponent;
	
	CachedStealthStateData = InStealthComponent->GetCurrentStealthStateData();
	BroadcastStealthStateData();
	
	// Unbind first in case of re-init
	if (StealthStateDataChangedHandle.IsValid())
	{
		InStealthComponent->OnStealthUpdate.Remove(StealthStateDataChangedHandle);
		StealthStateDataChangedHandle.Reset();
	}
	
	StealthStateDataChangedHandle =
		InStealthComponent->OnStealthUpdate.AddUObject(this, &ThisClass::HandleStealthStateDataChanged);
	
}

void UIsekaiUIBridge::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AbilitySystem.IsValid())
	{
		if (HealthChangedHandle.IsValid())
		{
			AbilitySystem->GetGameplayAttributeValueChangeDelegate(UIsekaiAttributeSet::GetHealthAttribute())
			.Remove(HealthChangedHandle);
			HealthChangedHandle.Reset();
		}
		if (MaxHealthChangedHandle.IsValid())
		{
			AbilitySystem->GetGameplayAttributeValueChangeDelegate(UIsekaiAttributeSet::GetMaxHealthAttribute())
			.Remove(MaxHealthChangedHandle);
			MaxHealthChangedHandle.Reset();
		}
		if (StaminaChangedHandle.IsValid())
		{
			AbilitySystem->GetGameplayAttributeValueChangeDelegate(UIsekaiAttributeSet::GetStaminaAttribute())
			.Remove(StaminaChangedHandle);
			StaminaChangedHandle.Reset();
		}
		if (MaxStaminaChangedHandle.IsValid())
		{
			AbilitySystem->GetGameplayAttributeValueChangeDelegate(UIsekaiAttributeSet::GetMaxStaminaAttribute())
			.Remove(MaxStaminaChangedHandle);
			MaxStaminaChangedHandle.Reset();
		}
		if (ExhaustedTagHandle.IsValid())
		{
			AbilitySystem->UnregisterGameplayTagEvent(ExhaustedTagHandle, Tags::State::Exhausted);
			ExhaustedTagHandle.Reset();
		}
		if (SprintingTagHandle.IsValid())
		{
			AbilitySystem->UnregisterGameplayTagEvent(SprintingTagHandle, Tags::State::Movement_Sprinting);
			SprintingTagHandle.Reset();
		}
	}
	
	if (StealthComponent.IsValid())
	{
		if (StealthStateDataChangedHandle.IsValid())
		{
			StealthComponent->OnStealthUpdate.Remove(StealthStateDataChangedHandle);
			StealthStateDataChangedHandle.Reset();
		}
	}
	
	Super::EndPlay(EndPlayReason);
}



FString UIsekaiUIBridge::GetStealthStateAsString() const
{
	switch (CachedStealthStateData.CurrentState)
	{
	case EStealthState::None:
		return TEXT("None");
	case EStealthState::Idle:
		return TEXT("Idle");
	case EStealthState::Suspicious:
		return TEXT("Suspicious");
	case EStealthState::Searching:
		return TEXT("Searching");
	case EStealthState::Alerted:
		return TEXT("Alerted");
	default:
		return TEXT("Unknown");
	}
}


void UIsekaiUIBridge::HandleHealthChanged(const struct FOnAttributeChangeData& Data)
{
	if (FMath::IsNearlyEqual(CachedHealth, Data.NewValue))
	{
		return;
	}
	
	CachedHealth = Data.NewValue;
	BroadcastHealth();
}

void UIsekaiUIBridge::HandleMaxHealthChanged(const struct FOnAttributeChangeData& Data)
{
	if (FMath::IsNearlyEqual(CachedMaxHealth, Data.NewValue))
	{
		return;
	}
	
	CachedMaxHealth = Data.NewValue;
	BroadcastHealth();
}

void UIsekaiUIBridge::HandleStaminaChanged(const struct FOnAttributeChangeData& Data)
{
	if (FMath::IsNearlyEqual(CachedStamina, Data.NewValue))
	{
		return;
	}
	
	CachedStamina = Data.NewValue;
	BroadcastStamina();
}

void UIsekaiUIBridge::HandleMaxStaminaChanged(const struct FOnAttributeChangeData& Data)
{
	if (FMath::IsNearlyEqual(CachedMaxStamina, Data.NewValue))
	{
		return;
	}
	
	CachedMaxStamina = Data.NewValue;
	BroadcastStamina();
}

void UIsekaiUIBridge::HandleExhaustedTagChanged(const struct FGameplayTag Tag, int32 NewCount)
{
	bIsExhausted = (NewCount > 0);
	BroadcastExhausted();
}

void UIsekaiUIBridge::HandleSprintingTagChanged(const struct FGameplayTag Tag, int32 NewCount)
{
	bIsSprinting = (NewCount > 0);
	BroadcastSprinting();
}

void UIsekaiUIBridge::HandleStealthStateDataChanged(const FStealthStateData& NewData)
{
	CachedStealthStateData = NewData;
	BroadcastStealthStateData();
}

// Broadcast helpers

void UIsekaiUIBridge::BroadcastHealth()
{
	OnHealthChanged.Broadcast(CachedHealth, CachedMaxHealth);
}

void UIsekaiUIBridge::BroadcastStamina()
{
	OnStaminaChanged.Broadcast(CachedStamina, CachedMaxStamina);
}

void UIsekaiUIBridge::BroadcastExhausted()
{
	OnExhaustedChanged.Broadcast(bIsExhausted);
}

void UIsekaiUIBridge::BroadcastSprinting()
{
	OnSprintingChanged.Broadcast(bIsSprinting);
}


void UIsekaiUIBridge::BroadcastStealthStateData()
{
	OnStealthStateDataChanged.Broadcast(CachedStealthStateData);
}


