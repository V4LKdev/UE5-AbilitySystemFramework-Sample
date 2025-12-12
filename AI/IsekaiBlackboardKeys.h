// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"

namespace BBKeys
{
	// --- Actors & Objects ---
	inline const FName GSelfActor = TEXT("SelfActor");
	inline const FName GTargetActor = TEXT("TargetActor");
	
	// --- Spatial Data ---
	inline const FName GLastKnownPosition = TEXT("LastKnownPosition");
	inline const FName GStimulusLocation = TEXT("StimulusLocation");
	inline const FName GMoveToLocation = TEXT("MoveToLocation");
	
	// --- State & Logic ---
	inline const FName GHasLOS = TEXT("bHasLOS");
	inline const FName GAlertLevel = TEXT("AlertLevel");
	inline const FName GStealthState = TEXT("StealthState");
	inline const FName GPatrolIndex = TEXT("PatrolIndex");
	inline const FName GPatrolDirection = TEXT("PatrolDirection");
	// inline const FName GCombatState = TEXT("CombatState");

	
	// Squad
	// inline const FName GSquadState = TEXT("SquadState"); // Passive, Coordinated Search, Combat etc...
	// inline const FName GSquadLeader = TEXT("SquadLeader");
	
}