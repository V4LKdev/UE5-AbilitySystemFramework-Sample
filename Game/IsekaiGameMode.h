// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "IsekaiGameMode.generated.h"

/**
 * 
 */
UCLASS()
class AIASSESSMENT_API AIsekaiGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AIsekaiGameMode();
	
	void HandlePlayerDeath(APlayerController* PlayerController);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn")
	float RespawnDelay = 5.f;
	
	UFUNCTION()
	void RespawnPlayer(AController* Controller);
};
