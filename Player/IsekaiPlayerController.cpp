// Fill out your copyright notice in the Description page of Project Settings.


#include "IsekaiPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputConfig.h"
#include "AbilitySystemInterface.h"
#include "AIAssessment/NativeGameplayTags.h"
#include "AIAssessment/AbilitySystem/IsekaiAbilitySystemComponent.h"
#include "AIAssessment/Character/IsekaiCharacterBase.h"
#include "AIAssessment/Character/IsekaiPlayer.h"
#include "AIAssessment/Input/IsekaiInputComponent.h"
#include "AIAssessment/Widget/IsekaiPlayerHUD.h"
#include "AIAssessment/IsekaiLoggingChannels.h"

AIsekaiPlayerController::AIsekaiPlayerController()
{
}

void AIsekaiPlayerController::InitializeHUD(AIsekaiPlayer* PlayerPawn)
{
	if (!IsLocalController() || !PlayerPawn)
	{
		return;
	}
	
	if (!PlayerHUD)
	{
		PlayerHUD = CreateWidget<UIsekaiPlayerHUD>(this, HUDClass);
		PlayerHUD->AddToViewport();
	}
	
	PlayerHUD->InitFromBridge(PlayerPawn->GetUIBridge());
}

void AIsekaiPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Only bind input for the local controller
	if (!IsLocalController() || !InputConfig)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	ensureMsgf(Subsystem, TEXT("Enhanced Input Subsystem not found on Local Player!"));

	Subsystem->AddMappingContext( InputConfig->DefaultMappingContext, 0 );
	
	TArray<uint32> BoundHandles;

	UIsekaiInputComponent* Input = Cast<UIsekaiInputComponent>(InputComponent);

	ensureMsgf(Input, TEXT("IsekaiPlayerController expected an IsekaiInputComponent!"));
	
	Input->BindAbilityActions(
		InputConfig,
		this,
		&ThisClass::Input_AbilityTagPressed,
		&ThisClass::Input_AbilityTagReleased,
		BoundHandles
	);
	
	Input->BindNativeAction(
		InputConfig,
		Tags::Input::Look,
		ETriggerEvent::Triggered,
		this,
		&ThisClass::Input_Look
	);
	
	Input->BindNativeAction(
		InputConfig,
		Tags::Input::Move,
		ETriggerEvent::Triggered,
		this,
		&ThisClass::Input_Move
	);
}

void AIsekaiPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (IsLocalController() && GetPawn() && GetPawn()->GetPlayerState()) // Only process for locally controlled pawns when the playerstate is replicated (aka the ASC is setup)
	{
		if (UIsekaiAbilitySystemComponent* IsekaiASC = GetPawnAbilitySystemComponent())
		{
			IsekaiASC->ProcessAbilityInput(DeltaTime, bGamePaused);
		}
	}
	
	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void AIsekaiPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
	
	PawnASC.Reset();
}


AIsekaiPlayer* AIsekaiPlayerController::GetIsekaiPlayerPawn() const
{
	APawn* Pawn = GetPawn();
	return Cast<AIsekaiPlayer>(Pawn);
}

UIsekaiAbilitySystemComponent* AIsekaiPlayerController::GetPawnAbilitySystemComponent()
{
	if (!PawnASC.IsValid())
	{
		APawn* ControlledPawn = GetPawn();
		if (!ControlledPawn) 
		{
			UE_LOG(LogIsekaiPlayer, Verbose,
				TEXT("GetPawnAbilitySystemComponent: No pawn for controller %s"),
				*GetName());
			return nullptr;
		}

		const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(ControlledPawn);
		if (!ASI)
		{
			UE_LOG(LogIsekaiPlayer, Warning,
				TEXT("GetPawnAbilitySystemComponent: Pawn %s does not implement IAbilitySystemInterface"),
				*ControlledPawn->GetName());
			return nullptr;
		}
		
		UIsekaiAbilitySystemComponent* IsekaiASC = Cast<UIsekaiAbilitySystemComponent>(ASI->GetAbilitySystemComponent());
		if (!IsekaiASC)
		{
			// This can trigger during initialisation when the pawn is possessed but the ASC is not yet set up
			UE_LOG(LogIsekaiPlayer, Warning,
				TEXT("GetPawnAbilitySystemComponent: Pawn %s returned a null or non-Isekai ASC"),
				*ControlledPawn->GetName());
			return nullptr;
		}
		
		PawnASC = IsekaiASC;
	}
	return PawnASC.Get();
}


// --- Input Handlers ---
void AIsekaiPlayerController::Input_AbilityTagPressed(FGameplayTag InputTag)
{
	if (UIsekaiAbilitySystemComponent* IsekaiASC = GetPawnAbilitySystemComponent())
	{
		IsekaiASC->AbilityInputTagPressed(InputTag);
	}
}

void AIsekaiPlayerController::Input_AbilityTagReleased(FGameplayTag InputTag)
{
	if (UIsekaiAbilitySystemComponent* IsekaiASC = GetPawnAbilitySystemComponent())
	{
		IsekaiASC->AbilityInputTagReleased(InputTag);
	}
}

void AIsekaiPlayerController::Input_Look(const FInputActionValue& Value)
{
	if (AIsekaiPlayer* IsekaiPlayer = GetIsekaiPlayerPawn())
	{
		IsekaiPlayer->HandleLookInput(Value);
	}
}

void AIsekaiPlayerController::Input_Move(const FInputActionValue& Value)
{
	if (AIsekaiPlayer* IsekaiPlayer = GetIsekaiPlayerPawn())
	{
		IsekaiPlayer->HandleMoveInput(Value);
	}
}
