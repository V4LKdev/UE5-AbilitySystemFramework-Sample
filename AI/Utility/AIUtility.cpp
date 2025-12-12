// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "AIUtility.h"

#include "AIAssessment/AI/IsekaiAITypes.h"

ETeamAttitude::Type FAIUtility::GetTeamAttitude(const AActor* Self, const AActor* Other)
{
	// Safety Checks
	if (!IsValid(Self) || !IsValid(Other))
	{
		return ETeamAttitude::Neutral;
	}
    
	// Self-Detection
	if (Self == Other)
	{
		return ETeamAttitude::Friendly;
	}
    
	// Interface Checks
	const IGenericTeamAgentInterface* SelfTeamAgent = Cast<const IGenericTeamAgentInterface>(Self);
	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(Other);

	if (!SelfTeamAgent || !OtherTeamAgent)
	{
		// If one doesn't implement the interface, default to Neutral
		return ETeamAttitude::Neutral;
	}

	// Team ID Comparison
	const uint8 SelfID = SelfTeamAgent->GetGenericTeamId().GetId();
	const uint8 TheirID = OtherTeamAgent->GetGenericTeamId().GetId();

	if (SelfID == TheirID)
	{
		return ETeamAttitude::Friendly;
	}

	// Explicit Hostilities
	if ((SelfID == EIsekaiTeamID::Player && TheirID == EIsekaiTeamID::Enemy) ||
		(SelfID == EIsekaiTeamID::Enemy && TheirID == EIsekaiTeamID::Player))
	{
		return ETeamAttitude::Hostile;
	}

	return ETeamAttitude::Neutral;
}