// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "IsekaiPlayer.h"

#include "InputActionValue.h"
#include "AIAssessment/IsekaiLoggingChannels.h"
#include "AIAssessment/NativeGameplayTags.h"
#include "AIAssessment/AbilitySystem/IsekaiAbilitySystemComponent.h"
#include "AIAssessment/Game/IsekaiGameMode.h"
#include "AIAssessment/Player/IsekaiPlayerController.h"
#include "AIAssessment/Player/IsekaiPlayerState.h"

AIsekaiPlayer::AIsekaiPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TeamID = EIsekaiTeamID::Player;
	TeamIdStruct = FGenericTeamId(TeamID);
}

void AIsekaiPlayer::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	InitializeAbilitySystemFromPlayerState();
}

void AIsekaiPlayer::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	InitializeAbilitySystemFromPlayerState();
}

void AIsekaiPlayer::InitializeAbilitySystemFromPlayerState()
{
	AIsekaiPlayerState* PS = GetPlayerState<AIsekaiPlayerState>();
	if (!PS)
	{
		UE_LOG(LogIsekaiPlayer, Error, TEXT("AIAssessmentPlayerState: PS is null"));
		return;
	}
	
	UIsekaiAbilitySystemComponent* ASC = PS->GetIsekaiAbilitySystemComponent();
	UIsekaiAttributeSet* AttributeSet = PS->GetIsekaiAttributeSet();
	
	InitAbilitySystem(PS, ASC,  AttributeSet);
	
	if (HasAuthority())
	{
		PS->ApplyStartupAbilitySets();
	}
	
	AIsekaiPlayerController* PlayerController = Cast<AIsekaiPlayerController>(GetController());
	if (PlayerController)
	{
		PlayerController->InitializeHUD(this);
	}
}

// --- Input Handlers ---
void AIsekaiPlayer::HandleMoveInput(const FInputActionValue& Value)
{
	FVector2D MoveAxis = Value.Get<FVector2D>();
	
	if (IsekaiAbilitySystemComponent && IsekaiAbilitySystemComponent->HasMatchingGameplayTag(Tags::State::Movement_InputBlocked))
	{
		return;
	}
	
	const bool bHasInput = !MoveAxis.IsNearlyZero(MoveInputDeadZone);
	
	if (!bHasInput || !Controller)
	{
		return;
	}
	
	MoveAxis = MoveAxis.GetClampedToMaxSize(1.0f);

	const FRotator ControlRotation = Controller->GetControlRotation();
	FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
	FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	FVector Right   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Right, MoveAxis.X);
	AddMovementInput(Forward, MoveAxis.Y);
}

void AIsekaiPlayer::HandleLookInput(const FInputActionValue& Value)
{
	const FVector2d LookAxis = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxis.X * LookSensitivity);
	AddControllerPitchInput(LookAxis.Y * LookSensitivity);
}

// --- Death Handling ---
void AIsekaiPlayer::HandleOutOfHealth()
{
	Super::HandleOutOfHealth();
	
	// Respawn logic
	if (AIsekaiGameMode* GM = GetWorld()->GetAuthGameMode<AIsekaiGameMode>())
	{
		if (AIsekaiPlayerController* PC = Cast<AIsekaiPlayerController>(GetController()))
		{
			GM->HandlePlayerDeath(PC);
		}
	}
}