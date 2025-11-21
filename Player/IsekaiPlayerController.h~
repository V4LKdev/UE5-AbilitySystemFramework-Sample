// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "GameFramework/PlayerController.h"
#include "IsekaiPlayerController.generated.h"

class UIsekaiUIBridge;
class UIsekaiPlayerHUD;
class AIsekaiPlayer;
class UIsekaiAbilitySystemComponent;
class AIsekaiPlayerCharacter;
struct FInputActionValue;
class UInputConfig;

/**
 * PlayerController for Isekai.
 *
 * Responsibilities:
 * - Own Enhanced Input mappings for the local player.
 * - Bind input actions to native handlers and ability input tag callbacks.
 * - Forward framed input to the Isekai ASC via PostProcessInput.
 */
UCLASS()
class AIASSESSMENT_API AIsekaiPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AIsekaiPlayerController();
	
	/** Creates and initializes the HUD for the given local player pawn. */
	void InitializeHUD(AIsekaiPlayer* PlayerPawn);
	
protected:
	virtual void SetupInputComponent() override;

	/** Called once per frame after input is processed; forwards input state to the ASC. */
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	
	virtual void OnUnPossess() override;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<const UInputConfig> InputConfig;
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UIsekaiPlayerHUD> HUDClass;
	
private:
	/** Returns the currently possessed pawn cast to AIsekaiPlayer, if any. */
	AIsekaiPlayer* GetIsekaiPlayerPawn() const;
	/** Returns (and caches) the Isekai ASC from the currently possessed pawn, if any. */
	UIsekaiAbilitySystemComponent* GetPawnAbilitySystemComponent();
	
	/** Weak reference to the pawn's ASC, refreshed on possession changes. */
	TWeakObjectPtr<UIsekaiAbilitySystemComponent> PawnASC;
	
	UPROPERTY()
	TObjectPtr<UIsekaiPlayerHUD> PlayerHUD;
	
	// Ability input handlers bound via InputConfig
	void Input_AbilityTagPressed(FGameplayTag InputTag);
	void Input_AbilityTagReleased(FGameplayTag InputTag);
	
	// Native movement/look handlers bound via InputConfig
	void Input_Look(const FInputActionValue& Value);
	void Input_Move(const FInputActionValue& Value);
};
