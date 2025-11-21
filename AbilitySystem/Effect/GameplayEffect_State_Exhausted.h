// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "IsekaiGameplayEffect_State.h"
#include "GameplayEffect_State_Exhausted.generated.h"

/**
 * 
 */
UCLASS()
class AIASSESSMENT_API UGameplayEffect_State_Exhausted : public UIsekaiGameplayEffect_State
{
	GENERATED_BODY()
public:
	UGameplayEffect_State_Exhausted();
	
	virtual void PostInitProperties() override;
};
