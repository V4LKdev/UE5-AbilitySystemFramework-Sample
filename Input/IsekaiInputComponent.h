// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "AIAssessment/Player/InputConfig.h"
#include "IsekaiInputComponent.generated.h"


class UInputConfig;

/**
 * Enhanced Input component wrapper for Isekai.
 *
 * Provides helpers to bind ability input actions (tagged in InputConfig) to
 * arbitrary UObject methods, and to bind native actions such as Move/Look.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AIASSESSMENT_API UIsekaiInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	/**
	 * Binds all ability input actions defined in the given InputConfig.
	 *
	 * - For each FTaggedInputAction with a valid tag, binds Started to PressedFunc
	 *   and Completed/Canceled to ReleasedFunc (if provided).
	 * - OutBoundHandles collects the underlying input binding handles for later unbinding.
	 */
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(
		const UInputConfig* InputConfig,
		UserClass* Object,
		PressedFuncType PressedFunc,
		ReleasedFuncType ReleasedFunc,
		TArray<uint32>& OutBoundHandles);
	
	/**
	 * Binds a single native action (e.g., Move, Look) identified by an input tag.
	 * Looks up the UInputAction in InputConfig and binds it to the given function.
	 */
	template<class UserClass, typename FuncType>
	void BindNativeAction(
		const UInputConfig* InputConfig,
		FGameplayTag InputTag,
		ETriggerEvent TriggerEvent,
		UserClass* Object,
		FuncType Func);
};

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UIsekaiInputComponent::BindAbilityActions(const UInputConfig* InputConfig, UserClass* Object,
	PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& OutBoundHandles)
{
	check(InputConfig);
	
	for (const FTaggedInputAction& Action : InputConfig->AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				FEnhancedInputActionEventBinding& Binding =
					BindAction(Action.InputAction, ETriggerEvent::Started, Object, PressedFunc, Action.InputTag);
				
				OutBoundHandles.Add(Binding.GetHandle());
			}
			
			if (ReleasedFunc)
			{
				FEnhancedInputActionEventBinding& Binding =
					BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag);
				
				FEnhancedInputActionEventBinding& CancelBinding =
					BindAction(Action.InputAction, ETriggerEvent::Canceled, Object, ReleasedFunc, Action.InputTag);
				
				OutBoundHandles.Add(Binding.GetHandle());
				OutBoundHandles.Add(CancelBinding.GetHandle());
			}
		}
	}
}

template <class UserClass, typename FuncType>
void UIsekaiInputComponent::BindNativeAction(const UInputConfig* InputConfig, FGameplayTag InputTag,
	ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func)
{
	check(InputConfig);
	
	if (const UInputAction* Action = InputConfig->FindNativeActionByTag(InputTag))
	{
		BindAction(Action, TriggerEvent, Object, Func);
	}
}
