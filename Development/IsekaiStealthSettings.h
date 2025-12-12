// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "IsekaiStealthSettings.generated.h"

USTRUCT(BlueprintType)
struct FNoiseConfig
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Noise", meta=(ClampMin="0.0"))
	float Loudness = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Noise", meta=(ClampMin="0.0"))
	float MaxRange = 1000.0f;
};

/**
 * 
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="Isekai Stealth"))
class AIASSESSMENT_API UIsekaiStealthSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category="Noise", meta=(categories="Noise"))
	TMap<FGameplayTag, FNoiseConfig> NoiseMap;
	
	UFUNCTION(BlueprintPure, Category="Isekai|Stealth|Settings")
	static const FNoiseConfig& GetNoiseConfig(const FGameplayTag& NoiseTag);
};
