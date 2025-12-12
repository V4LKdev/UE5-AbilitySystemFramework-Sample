// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "IsekaiStealthSettings.h"

const FNoiseConfig& UIsekaiStealthSettings::GetNoiseConfig(const FGameplayTag& NoiseTag)
{
	const UIsekaiStealthSettings* Settings = GetDefault<UIsekaiStealthSettings>();
	if (const FNoiseConfig* FoundConfig = Settings->NoiseMap.Find(NoiseTag))
	{
		return *FoundConfig;
	}
	
	static FNoiseConfig DefaultConfig;
	return DefaultConfig;
}
