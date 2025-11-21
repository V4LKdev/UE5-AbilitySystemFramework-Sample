// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "GA_Sprint.h"
#include "AIAssessment/NativeGameplayTags.h"
#include "AIAssessment/Character/IsekaiCharacterBase.h"
#include "AIAssessment/Character/IsekaiCharacterMovementComponent.h"
#include "AIAssessment/IsekaiLoggingChannels.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"

// --- Helper ---
bool UGA_Sprint::GetSprintContext(AIsekaiCharacterBase*& OutChar, UIsekaiCharacterMovementComponent*& OutMoveComp) const
{
	OutChar = GetIsekaiCharacter();
	OutMoveComp = OutChar ? OutChar->GetIsekaiCharacterMovement() : nullptr;
	return OutChar && OutMoveComp;
}

// --- Ability ---
UGA_Sprint::UGA_Sprint()
{
	// --- Tags ---
	FGameplayTagContainer CombinedTags = GetAssetTags();
	CombinedTags.AddTag(Tags::Ability::Movement_Sprint);
	CombinedTags.AddTag(Tags::Ability::Stamina_Drain);
	SetAssetTags(CombinedTags);
	
	// --- While active Tags ---
	ActivationOwnedTags.AddTag(Tags::State::Movement_Sprinting);

	// --- Networking ---
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	// --- Activation Policy ---
	ActivationPolicy = WhileInputActive;

	// --- Stamina settings ---
	StaminaDrainPerSecond = 10.0f;
	bAllowCostOverdraw    = false;
}

bool UGA_Sprint::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	
	// --- Sprint-specific checks ---
	AIsekaiCharacterBase* IsekaiCharacter;
	UIsekaiCharacterMovementComponent* IsekaiMoveComp;
	if (!GetSprintContext(IsekaiCharacter, IsekaiMoveComp))
	{
		return false;
	}
	
	const EMovementMode Mode = IsekaiMoveComp->MovementMode;
	if (Mode != MOVE_Walking && Mode != MOVE_Falling)
	{
		return false;
	}
	
	return true;
}

void UGA_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		return;
	}
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogIsekaiAbilitySystem, Verbose,
			TEXT("UGA_Sprint: CommitAbility failed for %s"),
			*GetName());
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	
	// --- Sprint logic ---
	AIsekaiCharacterBase* IsekaiCharacter;
	UIsekaiCharacterMovementComponent* IsekaiMoveComp;
	
	if (!GetSprintContext(IsekaiCharacter, IsekaiMoveComp))
	{
		UE_LOG(LogIsekaiAbilitySystem, Warning,
			TEXT("UGA_Sprint::ActivateAbility failed to get sprint context for %s"),
			*GetName());
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	
	UE_LOG(LogIsekaiAbilitySystem, Verbose,
		TEXT("UGA_Sprint activated for %s (InitialDash=%s)"),
		*IsekaiCharacter->GetName(),
		bUseInitialDash ? TEXT("true") : TEXT("false"));

	IsekaiMoveComp->StartSprinting();
	
	IsekaiCharacter->MovementModeChangedDelegate.AddDynamic(this, &ThisClass::HandleMovementModeChanged);
			
	MovementStateChangedHandle =
		IsekaiMoveComp->OnIsMovingChanged.AddUObject(this, &ThisClass::HandleMovementStateChanged);


	// Start stamina drain if on ground and moving
	if (IsekaiMoveComp->IsMovingOnGround() && IsekaiMoveComp->IsMoving())
	{
		StartStaminaDrain();
	}
	
	if (bUseInitialDash)
	{
		ApplyInitialDash();
	}
}

void UGA_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UE_LOG(LogIsekaiAbilitySystem, Verbose,
		TEXT("UGA_Sprint::EndAbility on %s (Cancelled=%s, DuringInitialDash=%s)"),
		*GetName(),
		bWasCancelled ? TEXT("true") : TEXT("false"),
		bDuringInitialDash ? TEXT("true") : TEXT("false"));

	if (bDuringInitialDash)
	{
		// Queue ending the ability until after the dash finishes
		bEndAbilityQueued = true;
		bReplicateQueuedEndAbility = bReplicateEndAbility;
		bWasEndAbilityCancelled = bWasCancelled;
		
		return;
	}
	
	bDuringInitialDash = false;
	bEndAbilityQueued = false;
	bReplicateQueuedEndAbility = false;
	bWasEndAbilityCancelled = false;
	
	if (AIsekaiCharacterBase* IsekaiCharacter = GetIsekaiCharacter())
	{
		if (UIsekaiCharacterMovementComponent* IsekaiMoveComp = IsekaiCharacter->GetIsekaiCharacterMovement())
		{
			IsekaiMoveComp->StopSprinting();
			
			IsekaiCharacter->MovementModeChangedDelegate.RemoveDynamic(this, &ThisClass::HandleMovementModeChanged);
			
			if (MovementStateChangedHandle.IsValid())
			{
				IsekaiMoveComp->OnIsMovingChanged.Remove(MovementStateChangedHandle);
				MovementStateChangedHandle.Reset();
			}
		}
	}

	// Base Stamina ability cleanup
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// --- Dash ---

void UGA_Sprint::ApplyInitialDash()
{
	AIsekaiCharacterBase* IsekaiCharacter;
	UIsekaiCharacterMovementComponent* IsekaiMoveComp;
	if (!GetSprintContext(IsekaiCharacter, IsekaiMoveComp))
	{
		UE_LOG(LogIsekaiAbilitySystem, Warning,
			TEXT("UGA_Sprint::ApplyInitialDash failed to get sprint context for %s"),
			*GetName());
		return;
	}
	
	FVector DashDirection = IsekaiCharacter->GetActorForwardVector();
	DashDirection.Z = 0.f;
	DashDirection.Normalize();
	
	if (!DashDirection.IsNearlyZero())
	{
		const bool bStartOnGround = IsekaiMoveComp->IsMovingOnGround();

		const FVector CurrentVelocity = IsekaiMoveComp->Velocity;
		const FVector CurrentHorizontalVelocity = FVector::VectorPlaneProject(CurrentVelocity, FVector::UpVector);
		const float SprintSpeed = IsekaiMoveComp->GetSprintSpeed();
		const float TargetSpeed = bStartOnGround
			? SprintSpeed
			: SprintSpeed * InitialDashEndAirVelocityMultiplier;

		const FVector TargetHorizontalVelocity =
			CurrentHorizontalVelocity.IsNearlyZero()
				? DashDirection * TargetSpeed
				: DashDirection * FMath::Max(CurrentHorizontalVelocity.Size(), TargetSpeed);
		
		const FVector FinishVelocity(
			TargetHorizontalVelocity.X,
			TargetHorizontalVelocity.Y,
			CurrentVelocity.Z * .2f);


		UAbilityTask_ApplyRootMotionConstantForce* DashTask =
			UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
				this,
				FName(TEXT("SprintInitialDash")),
				DashDirection,
				InitialDashStrength,
				InitialDashDuration,
				false,
				nullptr,
				ERootMotionFinishVelocityMode::SetVelocity,
				FinishVelocity,
				0.f,
				false
			);
		
		if (DashTask)
		{
			UE_LOG(LogIsekaiAbilitySystem, Verbose,
				TEXT("UGA_Sprint initial dash started for %s"),
				*IsekaiCharacter->GetName());

			DashTask->OnFinish.AddDynamic(this, &UGA_Sprint::OnInitialDashFinished);
			
			DashTask->ReadyForActivation();
			bDuringInitialDash = true;
		}
	}
}

void UGA_Sprint::OnInitialDashFinished()
{
	bDuringInitialDash = false;
	
	UE_LOG(LogIsekaiAbilitySystem, Verbose,
		TEXT("UGA_Sprint initial dash finished for %s"),
		*GetName());

	if (bEndAbilityQueued)
	{
		// value defines if it was cancelled or ended normally
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), bReplicateQueuedEndAbility, bWasEndAbilityCancelled);
		
		bEndAbilityQueued = false;
		bReplicateQueuedEndAbility = false;
		bWasEndAbilityCancelled = false;
	}
}


// --- Handlers ---

void UGA_Sprint::HandleMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	if (GetAvatarActorFromActorInfo() != Character)
	{
		return;
	}
	
	const UIsekaiCharacterMovementComponent* IsekaiMoveComp = Cast<UIsekaiCharacterMovementComponent>(Character->GetCharacterMovement());
	if (!IsekaiMoveComp)
	{
		return;
	}
	
	if (IsekaiMoveComp->IsFalling())
	{
		StopAllStaminaCosts();
	}
	else if (IsekaiMoveComp->IsMovingOnGround() && IsekaiMoveComp->IsMoving())
	{
		StartStaminaDrain();
	}
}

void UGA_Sprint::HandleMovementStateChanged(bool bIsMoving)
{
	if (!IsActive())
	{
		return;
	}
	
	AIsekaiCharacterBase* IsekaiCharacter = GetIsekaiCharacter();
	UIsekaiCharacterMovementComponent* IsekaiMoveComp = IsekaiCharacter ? IsekaiCharacter->GetIsekaiCharacterMovement() : nullptr;
	
	if (!IsekaiCharacter || !IsekaiMoveComp)
	{
		return;
	}
	
	if (bIsMoving)
	{
		// Start draining stamina when starting to move
		if (IsekaiMoveComp->IsMovingOnGround())
		{
			StartStaminaDrain();
		}
	}
	else
	{
		// Stop draining stamina when stopping movement
		StopAllStaminaCosts();
	}
}
