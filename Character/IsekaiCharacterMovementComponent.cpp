// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "IsekaiCharacterMovementComponent.h"

#include "IsekaiCharacterBase.h"
#include "AIAssessment/IsekaiLoggingChannels.h"
#include "AIAssessment/NativeGameplayTags.h"
#include "AIAssessment/AbilitySystem/IsekaiAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"


UIsekaiCharacterMovementComponent::UIsekaiCharacterMovementComponent()
{
	bWantsToSprint = false;
	bIsExhausted = false;
	
	// Enable crouching
	NavAgentProps.bCanCrouch = true;
	
	bCanWalkOffLedgesWhenCrouching = true;
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
	
	UnCrouch();
}

// --- Core movement overrides ---
float UIsekaiCharacterMovementComponent::GetMaxSpeed() const
{
	float Speed = Super::GetMaxSpeed();
	
	if (Speed <= 0.f || IsCrouching())
	{
		// Crouch speed is not affected by sprinting or exhaustion and is already set in Super::GetMaxSpeed()
		return Speed;
	}
	
	const float NormalizedFactor = MaxWalkSpeed > 0.f ? Speed / MaxWalkSpeed : 1.f;
	
	float EffectiveSpeed = MaxWalkSpeed;
	
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

bool UIsekaiCharacterMovementComponent::CanUncrouch() const
{
	// code is copied from UCharacterMovementComponent::CanUncrouch
	
	// 1. Basic Validity Checks
	if (!HasValidData() || !CharacterOwner || !CharacterOwner->GetCapsuleComponent())
	{
		return false;
	}

	// If we aren't crouched, we can "uncrouch" (operation is a no-op)
	if (!IsCrouching())
	{
		return true;
	}

	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	if (!DefaultCharacter) return false;

	// 2. Geometry Setup
	const float CurrentCrouchedHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float HalfHeightAdjust = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - OldUnscaledHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	// 3. Collision Query Setup
	const UWorld* MyWorld = GetWorld();
	constexpr float SweepInflation = KINDA_SMALL_NUMBER * 10.f;
	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, CharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(CapsuleParams, ResponseParam);

	// The shape of the capsule if we were standing
	const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust);
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	
	bool bEncroached = true;

	// 4. Perform the Sweep
	if (!bCrouchMaintainsBaseLocation)
	{
		// Standard Mode: Expand Upwards/Outwards
		bEncroached = MyWorld->OverlapBlockingTestByChannel(PawnLocation, GetWorldToGravityTransform(), CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

		if (bEncroached)
		{
			// Try adjusting capsule position
			if (ScaledHalfHeightAdjust > 0.f)
			{
				float PawnRadius, PawnHalfHeight;
				CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
				const float ShrinkHalfHeight = PawnHalfHeight - PawnRadius;
				const float TraceDist = PawnHalfHeight - ShrinkHalfHeight;
				const FVector Down = TraceDist * GetGravityDirection();

				FHitResult Hit(1.f);
				const FCollisionShape ShortCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, ShrinkHalfHeight);
				
				// Sweep down to find ground
				const bool bBlockingHit = MyWorld->SweepSingleByChannel(Hit, PawnLocation, PawnLocation + Down, GetWorldToGravityTransform(), CollisionChannel, ShortCapsuleShape, CapsuleParams);
				
				if (!Hit.bStartPenetrating)
				{
					// Check if standing at the hit location works
					const float DistanceToBase = (Hit.Time * TraceDist) + ShortCapsuleShape.Capsule.HalfHeight;
					const FVector Adjustment = (-DistanceToBase + StandingCapsuleShape.Capsule.HalfHeight + SweepInflation + MIN_FLOOR_DIST / 2.f) * -GetGravityDirection();
					const FVector NewLoc = PawnLocation + Adjustment;
					
					bEncroached = MyWorld->OverlapBlockingTestByChannel(NewLoc, GetWorldToGravityTransform(), CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
				}
			}
		}
	}
	else
	{
		// Maintain Base Location Mode
		FVector StandingLocation = PawnLocation + (StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentCrouchedHalfHeight) * -GetGravityDirection();
		bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, GetWorldToGravityTransform(), CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
		
		if (bEncroached && IsMovingOnGround())
		{
			constexpr float MinFloorDist = KINDA_SMALL_NUMBER * 10.f;
			if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
			{
				StandingLocation -= (CurrentFloor.FloorDist - MinFloorDist) * -GetGravityDirection();
				bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, GetWorldToGravityTransform(), CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
			}
		}
	}
	
	return !bEncroached;
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