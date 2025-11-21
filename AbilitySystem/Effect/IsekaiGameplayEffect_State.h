// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "IsekaiGameplayEffect_State.generated.h"

/**
 * Tiny subclass of UGameplayEffect to represent state effects (like Dead, Exhausted, etc.) granting tags while active
 */
UCLASS()
class AIASSESSMENT_API UIsekaiGameplayEffect_State : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UIsekaiGameplayEffect_State()
	{
		DurationPolicy = EGameplayEffectDurationType::Infinite;
	}
};
