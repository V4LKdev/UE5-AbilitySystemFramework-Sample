// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "IsekaiGameplayEffect_State.h"
#include "GameplayEffect_State_Dead.generated.h"

/**
 * 
 */
UCLASS()
class AIASSESSMENT_API UGameplayEffect_State_Dead : public UIsekaiGameplayEffect_State
{
	GENERATED_BODY()
public:
	UGameplayEffect_State_Dead();
	
	virtual void PostInitProperties() override;
};
