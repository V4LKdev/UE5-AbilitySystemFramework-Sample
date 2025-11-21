// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "IsekaiCharacterBase.h"
#include "IsekaiPlayer.generated.h"

struct FInputActionValue;

/** 
 * Player pawn that pulls ASC/Attributes from PlayerState.
 * 
 * Initialization is called:
 *  - on server in PossessedBy()
 *  - on client in OnRep_PlayerState()
 */
UCLASS()
class AIASSESSMENT_API AIsekaiPlayer : public AIsekaiCharacterBase
{
	GENERATED_BODY()
public:
	AIsekaiPlayer(const FObjectInitializer& ObjectInitializer);
	
	// Server: Called when controller possesses this pawn
	virtual void PossessedBy(AController* NewController) override;
	
	// Client: Called when PlayerState is replicated
	virtual void OnRep_PlayerState() override;
	
	/** Native input handlers */
	void HandleMoveInput(const FInputActionValue& Value);
	void HandleLookInput(const FInputActionValue& Value);
	
	virtual void HandleOutOfHealth() override;

protected:
	void InitializeAbilitySystemFromPlayerState();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	float LookSensitivity = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	float MoveInputDeadZone = 0.05f;
};
