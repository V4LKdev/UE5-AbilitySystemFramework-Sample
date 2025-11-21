// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffectComponent.h"
#include "IsekaiCancelAbilityTagsGameplayEffectComponent.generated.h"


/**
 * 
 */
UCLASS(DisplayName="Cancel Abilities with Tags")
class AIASSESSMENT_API UIsekaiCancelAbilityTagsGameplayEffectComponent : public UGameplayEffectComponent
{
	GENERATED_BODY()
public:
	virtual bool OnActiveGameplayEffectAdded(FActiveGameplayEffectsContainer& ActiveGEContainer, FActiveGameplayEffect& ActiveGE) const override;
	
	void SetAndApplyCanceledAbilityTagChanges(const FInheritedTagContainer& InCanceledAbilityTags);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CancelAbilities")
	FInheritedTagContainer CanceledAbilityTags;
};
