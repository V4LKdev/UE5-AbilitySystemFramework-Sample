// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "IsekaiAbilitySet.generated.h"

class UIsekaiAbilitySystemComponent;
class UAttributeSet;
struct FActiveGameplayEffectHandle;
struct FGameplayAbilitySpecHandle;

/**
 * A Single Ability to grant with level and optional input binding
 */
USTRUCT(BlueprintType)
struct FIsekaiGrantedAbility
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TSubclassOf<class UIsekaiGameplayAbility> AbilityClass = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	int32 AbilityLevel = 1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability", meta=(categories ="Input"))
	FGameplayTag InputTag;
};


USTRUCT(BlueprintType)
struct FIsekaiGrantedEffect
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<class UGameplayEffect> EffectClass = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	int32 EffectLevel = 1;
};

USTRUCT(BlueprintType)
struct FIsekaiGrantedAttributeSet
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute Set")
	TSubclassOf<UAttributeSet> AttributeSetClass = nullptr;
};

/**
 * Handles for abilities and effects that were granted
 */
struct FIsekaiAbilitySetGrantedHandles
{
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;
	TArray<FActiveGameplayEffectHandle> EffectHandles;
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
	
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	void AddAttributeSet(UAttributeSet* AttributeSet);
	
	void TakeFromAbilitySystem(UIsekaiAbilitySystemComponent* AbilitySystemComponent);
	void Reset();
};	


/**
 * 
 */
UCLASS()
class AIASSESSMENT_API UIsekaiAbilitySet : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySet")
	TArray<FIsekaiGrantedAbility> Abilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySet")
	TArray<FIsekaiGrantedEffect> Effects;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySet")
	TArray<FIsekaiGrantedAttributeSet> AttributeSets;
	
	/**
	 * Grant abilities, effects, and attribute sets from this set to the ASC.
	 * Only valid to call on the authority owning the ASC.
	 *
	 * @param ASC                Target ability system component.
	 * @param OutGrantedHandles  Optional handle container to allow removal later.
	 * @param SourceObject       Optional source object (e.g. PlayerState, equipment).
	 */
	void GiveToAbilitySystem(UIsekaiAbilitySystemComponent* ASC, 
		FIsekaiAbilitySetGrantedHandles* OutGrantedHandles = nullptr, 
		UObject* SourceObject = nullptr) const;
};
