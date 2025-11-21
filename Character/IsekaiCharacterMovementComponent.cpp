// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "IsekaiCharacterMovementComponent.h"

#include "IsekaiCharacterBase.h"
#include "AIAssessment/IsekaiLoggingChannels.h"
#include "AIAssessment/NativeGameplayTags.h"
#include "AIAssessment/AbilitySystem/IsekaiAbilitySystemComponent.h"


UIsekaiCharacterMovementComponent::UIsekaiCharacterMovementComponent()
{
	MaxWalkSpeed = BaseWalkSpeed;
	
	bWantsToSprint = false;
	bIsExhausted = false;
}

void UIsekaiCharacterMovementComponent::InitializeIsekaiReferences(AIsekaiCharacterBase* InCharacter,
	UIsekaiAbilitySystemComponent* InASC)
{
	if (!InCharacter || !InASC)
	{
		UE_LOG(LogIsekaiPlayer, Warning, TEXT("UIsekaiCharacterMovementComponent::InitializeIsekaiReferences: Invalid character or ASC provided."));
		return;
	}
	
	CachedIsekaiCharacter = InCharacter;
	CachedASC = InASC;
	
	ExhaustedTagChangedHandle =
		InASC->RegisterGameplayTagEvent(Tags::State::Exhausted, EGameplayTagEventType::NewOrRemoved).AddUObject(
			this, &UIsekaiCharacterMovementComponent::HandleExhaustedTagChanged);
}

void UIsekaiCharacterMovementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedASC && ExhaustedTagChangedHandle.IsValid())
	{
		CachedASC->UnregisterGameplayTagEvent(ExhaustedTagChangedHandle, Tags::State::Exhausted);
	}
	
	Super::EndPlay(EndPlayReason);
}

// --- Core movement overrides ---
float UIsekaiCharacterMovementComponent::GetMaxSpeed() const
{
	float Speed = Super::GetMaxSpeed();
	
	if (Speed <= 0.f)
	{
		return Speed;
	}
	
	const float NormalizedFactor = BaseWalkSpeed > 0.f ? Speed / BaseWalkSpeed : 1.f;
	
	float EffectiveSpeed = BaseWalkSpeed;
	
	const bool bCanApplySprint = bWantsToSprint && !bIsExhausted;
	if (bCanApplySprint)
	{
		EffectiveSpeed *= SprintSpeedMultiplier;
	}
	
	if (bIsExhausted)
	{
		EffectiveSpeed *= ExhaustedSpeedMultiplier;
	}
	
	EffectiveSpeed *= NormalizedFactor;
	
	return EffectiveSpeed;
}

void UIsekaiCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation,
	const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	
	const float CurrentSpeedSqr = Velocity.SizeSquared();
	const float Threshold = FMath::Square(MovingThreshold);
	
	const bool bCurrentlyMoving = CurrentSpeedSqr > Threshold;
	if (bIsMoving != bCurrentlyMoving)
	{
		bIsMoving = bCurrentlyMoving;
		OnIsMovingChanged.Broadcast(bIsMoving);
	}
}

void UIsekaiCharacterMovementComponent::AddInputVector(FVector WorldVector, bool bForce)
{
	if (IsRootMotionActive())
	{
		return;
	}
	
	Super::AddInputVector(WorldVector, bForce);
}

bool UIsekaiCharacterMovementComponent::IsRootMotionActive() const
{
	return CurrentRootMotion.HasRootMotionToApply() || HasAnimRootMotion();
}


// --- Gas integration ---
void UIsekaiCharacterMovementComponent::HandleExhaustedTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	const bool bNewExhausted = NewCount > 0;
	SetIsExhausted(bNewExhausted);
}

void UIsekaiCharacterMovementComponent::StartSprinting()
{
	// We could check against tags etc here
	bWantsToSprint = true;
}

void UIsekaiCharacterMovementComponent::StopSprinting()
{
	bWantsToSprint = false;
}

void UIsekaiCharacterMovementComponent::SetIsExhausted(bool bNewExhausted)
{
	bIsExhausted = bNewExhausted;
}