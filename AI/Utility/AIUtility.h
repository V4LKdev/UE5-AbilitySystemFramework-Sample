// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"


/**
 * 
 */
class AIASSESSMENT_API FAIUtility
{
public:
	static ETeamAttitude::Type GetTeamAttitude(const AActor* Self, const AActor* Other);
};
