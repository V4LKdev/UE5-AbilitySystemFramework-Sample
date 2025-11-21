// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "IsekaiAttributeSet.h"

#include "IsekaiAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AIAssessment/NativeGameplayTags.h"
#include "AIAssessment/Character/IsekaiCharacterBase.h"
#include "AIAssessment/IsekaiLoggingChannels.h"
#include "Effect/GameplayEffect_State_Dead.h"
#include "Effect/GameplayEffect_State_Exhausted.h"

UIsekaiAttributeSet::UIsekaiAttributeSet()
{
}

void UIsekaiAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UIsekaiAttributeSet, Health,				COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UIsekaiAttributeSet, MaxHealth,			COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UIsekaiAttributeSet, Stamina,			COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UIsekaiAttributeSet, MaxStamina,			COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UIsekaiAttributeSet, HealthRegenRate,	COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UIsekaiAttributeSet, HealthRegenDelay,	COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UIsekaiAttributeSet, StaminaRegenRate,	COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UIsekaiAttributeSet, StaminaRegenDelay,	COND_None, REPNOTIFY_Always);
}

void UIsekaiAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	// Clamping
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	// Keep Above Zero
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(1.0f, NewValue);
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
	else if (Attribute == GetStaminaRegenRateAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
	else if (Attribute == GetStaminaRegenDelayAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
}

void UIsekaiAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	AActor* TargetActor = nullptr;
	UIsekaiAbilitySystemComponent* TargetIsekaiASC = nullptr;
	
	if (Data.Target.AbilityActorInfo.IsValid())
	{
		TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		TargetIsekaiASC = Cast<UIsekaiAbilitySystemComponent>(Data.Target.AbilityActorInfo->AbilitySystemComponent.Get());
	}
	
	AIsekaiCharacterBase* TargetCharacter = Cast<AIsekaiCharacterBase>(TargetActor); 
	
	const FGameplayAttribute& Attribute = Data.EvaluatedData.Attribute;
	
	// Health
	if (Attribute == GetHealthAttribute())
	{
		const float NewHealth = FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth());
		SetHealth(NewHealth);
		
		if (NewHealth <= 0.0f)
		{
			HandleOutOfHealth(TargetIsekaiASC);
			
			if (TargetCharacter)
			{
				TargetCharacter->HandleOutOfHealth();
			}
		}
	}
	
	// Max Health
	else if (Attribute == GetMaxHealthAttribute())
	{
		const float NewMaxHealth = FMath::Max(1.0f, GetMaxHealth());
		SetMaxHealth(NewMaxHealth);
		
		// Adjust current Health to not exceed new MaxHealth
		if (GetHealth() > NewMaxHealth)
		{
			SetHealth(NewMaxHealth);
		}
	}
	
	// Stamina
	else if (Attribute == GetStaminaAttribute())
	{
		const float NewStamina = FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina());
		SetStamina(NewStamina);
		
		 if (TargetIsekaiASC)
		 {
		 	HandleExhaustionStateChange(TargetIsekaiASC, NewStamina);
		 }
	}
	
	// Max Stamina
	else if (Attribute == GetMaxStaminaAttribute())
	{
		const float NewMaxStamina = FMath::Max(0.0f, GetMaxStamina());
		SetMaxStamina(NewMaxStamina);
		
		// Adjust current Stamina to not exceed new MaxStamina
		if (GetStamina() > NewMaxStamina)
		{
			SetStamina(NewMaxStamina);
		}
	}
}


// --- Custom Handlers ---
void UIsekaiAttributeSet::HandleExhaustionStateChange(UIsekaiAbilitySystemComponent* TargetIsekaiASC, const float NewStamina)
{
	if (!TargetIsekaiASC || !TargetIsekaiASC->GetOwner()->HasAuthority())
	{
		return;
	}
	
	const bool bHasExhausted = TargetIsekaiASC->HasMatchingGameplayTag(Tags::State::Exhausted);
		 	
	// Hit zero stamina -> apply exhausted state
	if (NewStamina <= 0.0f && !bHasExhausted)
	{
		UE_LOG(LogIsekaiAbilitySystem, Log,
			TEXT("Applying Exhausted state: ASC=%s Stamina=%.2f"),
			*TargetIsekaiASC->GetName(),
			NewStamina);

		FGameplayEffectContextHandle Ctx = TargetIsekaiASC->MakeEffectContext();
		Ctx.AddSourceObject(this);
		 	
		// create gamplay effect spec from the state effect class which is set to be infinite duration
		const FGameplayEffectSpecHandle SpecHandle =
			TargetIsekaiASC->MakeOutgoingSpec(UGameplayEffect_State_Exhausted::StaticClass(), 1.0f, Ctx);
		 	
		if (SpecHandle.IsValid())
		{
			ExhaustedStateEffectHandle = 
				TargetIsekaiASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
	// Recovered stamina -> remove exhausted state
	else if (NewStamina >= GetMaxStamina() && bHasExhausted && ExhaustedStateEffectHandle.IsValid())
	{
		UE_LOG(LogIsekaiAbilitySystem, Log,
			TEXT("Removing Exhausted state: ASC=%s Stamina=%.2f"),
			*TargetIsekaiASC->GetName(),
			NewStamina);

		TargetIsekaiASC->RemoveActiveGameplayEffect(ExhaustedStateEffectHandle);
		ExhaustedStateEffectHandle.Invalidate();
	}
}

void UIsekaiAttributeSet::HandleOutOfHealth(UIsekaiAbilitySystemComponent* TargetIsekaiASC)
{
	if (!TargetIsekaiASC || !TargetIsekaiASC->GetOwner()->HasAuthority())
	{
		return;
	}
	
	const bool bIsAlreadyDead = TargetIsekaiASC->HasMatchingGameplayTag(Tags::State::Dead) || DeadStateEffectHandle.IsValid();
	
	if (bIsAlreadyDead)
	{
		UE_LOG(LogIsekaiAbilitySystem, Verbose,
			TEXT("HandleOutOfHealth called but ASC %s is already dead."),
			*TargetIsekaiASC->GetName());
		return;
	}
	
	UE_LOG(LogIsekaiAbilitySystem, Log,
		TEXT("Applying Dead state GE for ASC %s"),
		*TargetIsekaiASC->GetName());

	FGameplayEffectContextHandle Ctx = TargetIsekaiASC->MakeEffectContext();
	Ctx.AddSourceObject(this);
	
	const FGameplayEffectSpecHandle SpecHandle =
		TargetIsekaiASC->MakeOutgoingSpec(UGameplayEffect_State_Dead::StaticClass(), 1.0f, Ctx);
	
	if (SpecHandle.IsValid())
	{
		DeadStateEffectHandle = 
			TargetIsekaiASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void UIsekaiAttributeSet::InvalidateTransientEffectHandles()
{
	DeadStateEffectHandle.Invalidate();
	ExhaustedStateEffectHandle.Invalidate();
}

// --- Replication Notification ---
void UIsekaiAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIsekaiAttributeSet, Health, OldValue);
}

void UIsekaiAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIsekaiAttributeSet, MaxHealth, OldValue);
}

void UIsekaiAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIsekaiAttributeSet, Stamina, OldValue);
}

void UIsekaiAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIsekaiAttributeSet, MaxStamina, OldValue);
}

void UIsekaiAttributeSet::OnRep_HealthRegenRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIsekaiAttributeSet, HealthRegenRate, OldValue);
}

void UIsekaiAttributeSet::OnRep_HealthRegenDelay(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIsekaiAttributeSet, HealthRegenDelay, OldValue);
}

void UIsekaiAttributeSet::OnRep_StaminaRegenRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIsekaiAttributeSet, StaminaRegenRate, OldValue);
}

void UIsekaiAttributeSet::OnRep_StaminaRegenDelay(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIsekaiAttributeSet, StaminaRegenDelay, OldValue);
}
