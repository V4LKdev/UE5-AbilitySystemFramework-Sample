// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "IsekaiAIController.generated.h"

class AIsekaiPatrolPath;
class UAIStealthComponent;
struct FAIStimulus;
class AIsekaiAICharacter;

/**
 * Server-authoritative controller for AI Agents.
 * Manages perception (Sight/Hearing) and initializes the Blackboard/Behavior Tree.
 * Feeds sensory data into the Pawn's StealthComponent.
 */
UCLASS()
class AIASSESSMENT_API AIsekaiAIController : public AAIController
{
	GENERATED_BODY()

public:
	AIsekaiAIController();

	/** Stops behavior tree, perception, and other controller logic when the pawn dies. */
	void HandlePawnDeath();
	
	AIsekaiPatrolPath* GetPatrolPath() const { return CachedPatrolPath.Get(); }

protected:
	// --- Actor Interface ---
	virtual void BeginPlay() override;
	
	// --- Controller Interface ---
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	// Handle AI Perception updates
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* InTargetActor, FAIStimulus InStimulus);
	
private:
	void SetupPerceptionSystem();
	void InitAIBehavior();
	void ResetBlackboard();
	
	TWeakObjectPtr<AIsekaiAICharacter> ControlledAICharacter;
	TWeakObjectPtr<AIsekaiPatrolPath> CachedPatrolPath;
};
