// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "IsekaiGameMode.h"

#include "AIAssessment/Character/IsekaiPlayer.h"
#include "AIAssessment/Player/IsekaiPlayerController.h"
#include "AIAssessment/Player/IsekaiPlayerState.h"

AIsekaiGameMode::AIsekaiGameMode()
{
	PlayerControllerClass = AIsekaiPlayerController::StaticClass();
	DefaultPawnClass = AIsekaiPlayer::StaticClass();
	PlayerStateClass = AIsekaiPlayerState::StaticClass();
}

void AIsekaiGameMode::HandlePlayerDeath(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}
	
	FTimerDelegate RespawnDelegate;
	RespawnDelegate.BindUFunction(this, FName("RespawnPlayer"), PlayerController);
	
	FTimerHandle RespawnDelegateHandle;
	GetWorldTimerManager().SetTimer(RespawnDelegateHandle, RespawnDelegate, RespawnDelay, false);
}

void AIsekaiGameMode::RespawnPlayer(AController* Controller)
{
	if (!Controller)
	{
		return;
	}
	
	if (AIsekaiPlayerState* IsekaiPS = Controller->GetPlayerState<AIsekaiPlayerState>())
	{
		IsekaiPS->ResetForRespawn();
	}
	
	if (APawn* OldPawn = Controller->GetPawn())
	{
		Controller->UnPossess();
		OldPawn->Destroy();
	}
	
	RestartPlayer(Controller);
}
