// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "InputConfig.generated.h"


class UInputMappingContext;
class UInputAction;

/**
 *	Single input action tagged with a gameplay tag
 */
USTRUCT(BlueprintType)
struct FTaggedInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, meta=(Categories="Input"))
	FGameplayTag InputTag;
};

/**
 * Data asset that holds input mapping contexts and tagged input actions
 */
UCLASS()
class AIASSESSMENT_API UInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly , Category="Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TArray<FTaggedInputAction> NativeInputActions;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TArray<FTaggedInputAction> AbilityInputActions;
	
	
	const UInputAction* FindNativeActionByTag(const FGameplayTag& Tag) const
	{
		for (const auto& [InputAction, InputTag] : NativeInputActions)
		{
			if (InputAction && InputTag.MatchesTagExact(Tag))
			{
				return InputAction;
			}
		}
		return nullptr;
	}
	
	const UInputAction* FindAbilityActionByTag(const FGameplayTag& Tag) const
	{
		for (const auto& [InputAction, InputTag] : AbilityInputActions)
		{
			if (InputAction && InputTag.MatchesTagExact(Tag))
			{
				return InputAction;
			}
		}
		return nullptr;
	}
	
	FGameplayTag FindAbilityTagForAction(const UInputAction* Action) const
	{
		for (const auto& [InputAction, InputTag] : AbilityInputActions)
		{
			if (InputAction == Action)
			{
				return InputTag;
			}
		}
		return FGameplayTag::EmptyTag;
	}
};
