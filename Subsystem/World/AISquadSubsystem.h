// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIAssessment/AI/IsekaiAITypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "AISquadSubsystem.generated.h"

class UAISquadComponent;

/**
 * Central "Dispatch Server" for AI Squads.
 * Manages registration and message routing.
 */
UCLASS()
class AIASSESSMENT_API UAISquadSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	// Squad Management
	void RegisterMember(int32 SquadID, UAISquadComponent* Member);
	void UnregisterMember(int32 SquadID, UAISquadComponent* Member);
	
	// Messaging
	void BroadcastMessage(int32 SquadID, const FSquadMessage& Message) const;
	
	// Utility
	TArray<TWeakObjectPtr<UAISquadComponent>> GetSquadMembers(int32 SquadID) const;
	int32 GetSquadMemberCount(int32 SquadID) const;
	bool DoesSquadExist(int32 SquadID) const;
	
private:
	// Squad Store
	TMap<int32, TArray<TWeakObjectPtr<UAISquadComponent>>> Squads;
	
	void DrawDebugMessage(const FSquadMessage& Msg, const TArray<TWeakObjectPtr<UAISquadComponent>>& Members) const;
};
