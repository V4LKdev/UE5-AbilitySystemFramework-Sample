// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "UAnimNotify_ReportNoise.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, meta=(DisplayName="Report Noise"))
class AIASSESSMENT_API UUAnimNotify_ReportNoise : public UAnimNotify
{
	GENERATED_BODY()
public:
	UUAnimNotify_ReportNoise();
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Notify", meta=(categories="Noise"))
	FGameplayTag NoiseType;
};
