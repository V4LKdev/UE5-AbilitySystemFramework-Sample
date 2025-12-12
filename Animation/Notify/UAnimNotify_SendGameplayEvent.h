// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "UAnimNotify_SendGameplayEvent.generated.h"

/**
 * Anim Notify that sends a Gameplay Event to the owner's Ability System Component.
 * Use this in Montages to trigger Ability logic (like dealing damage).
 */
UCLASS()
class AIASSESSMENT_API UUAnimNotify_SendGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Event", meta=(categories="Event"))
	FGameplayTag EventTag;
};