// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "AIStealthComponent.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "AIAssessment/IsekaiLoggingChannels.h"
#include "AIAssessment/AI/IsekaiBlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemGlobals.h"
#include "AISquadComponent.h"
#include "AIAssessment/NativeGameplayTags.h"
#include "AIAssessment/Character/IsekaiCharacterBase.h"

namespace StealthDebugCVars
{
	static TAutoConsoleVariable<bool> CVarDebugStealth(
		TEXT("Isekai.Stealth.Debug"),
		false,
		TEXT("Global toggle for AI Stealth debugging."),
		ECVF_Cheat);

	static TAutoConsoleVariable<bool> CVarShowRanges(
		TEXT("Isekai.Stealth.ShowRanges"),
		true,
		TEXT("Draws detection radii (Instant, Falloff, Chase, etc)."),
		ECVF_Cheat);
	
	static TAutoConsoleVariable<bool> CVarShowStats(
		TEXT("Isekai.Stealth.ShowStats"),
		true,
		TEXT("Draws text block with exact tuning values and gain rates."),
		ECVF_Cheat);

	static TAutoConsoleVariable<bool> CVarShowStimuli(
		TEXT("Isekai.Stealth.ShowStimuli"),
		true,
		TEXT("Draws lines to current target and last known position."),
		ECVF_Cheat);
	
	static TAutoConsoleVariable<bool> CVarShowSound(
		TEXT("Isekai.Stealth.ShowSound"),
		true,
		TEXT("Draws sound stimulus spheres."),
		ECVF_Cheat);
}

#pragma region Init & Reset

UAIStealthComponent::UAIStealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAIStealthComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UAIStealthComponent, CurrentStealthState);
	DOREPLIFETIME(UAIStealthComponent, CurrentAlertValue);
}

void UAIStealthComponent::Init(AAIController* AICon, UBlackboardComponent* InBlackboard, const FAlertTuning& InTuning)
{
	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogIsekaiAI, Warning, TEXT("UAIStealthComponent::Init called on non-authority!"));
		return;
	}
	
	OwnerController = AICon;
	BlackboardComp = InBlackboard;
	Tuning = InTuning;
	
	CurrentAlertValue = 0.f;
	CurrentStealthState = EStealthState::Idle;
	
	// Initialize blackboard values
	if (BlackboardComp)
	{
		InBlackboard->SetValueAsFloat(BBKeys::GAlertLevel, 0.f);
		InBlackboard->SetValueAsEnum(BBKeys::GStealthState, static_cast<uint8>(CurrentStealthState));
	}
}

void UAIStealthComponent::Reset()
{
	if (!GetOwner()->HasAuthority()) return;
	
	if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(AlertTimerHandle);
	
	CurrentAlertValue = 0.f;
	CurrentStealthState = EStealthState::Idle;
	TimeSinceLastStimulus = 0.f;
	bIsCoolingDown = false;
	
	OwnerController = nullptr;
	BlackboardComp = nullptr;
	Tuning = FAlertTuning();
}

void UAIStealthComponent::CompleteSearch()
{
	if (!GetOwner()->HasAuthority()) return;
	
	if (CurrentAlertValue >= MAX_ALERT_VALUE)
	{
		bIsCoolingDown = true;
	}
	
	if (BlackboardComp)
	{
		BlackboardComp->ClearValue(BBKeys::GLastKnownPosition);
		BlackboardComp->ClearValue(BBKeys::GTargetActor);
	}
}

void UAIStealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())	GetWorld()->GetTimerManager().ClearTimer(AlertTimerHandle);
	Super::EndPlay(EndPlayReason);
}

#pragma endregion

#pragma region Stimuli Processing

void UAIStealthComponent::HandleSightStimulus(AActor* SightActor, const FAIStimulus& Stimulus)
{
	if (!GetOwner()->HasAuthority() || !IsValid(BlackboardComp))	return;
	
	const bool bIsSensed = Stimulus.WasSuccessfullySensed();
	
	// Update blackboard awareness
	BlackboardComp->SetValueAsBool(BBKeys::GHasLOS, bIsSensed);
	BlackboardComp->SetValueAsVector(BBKeys::GStimulusLocation, Stimulus.StimulusLocation);
	
	if (bIsSensed)
	{
		BlackboardComp->SetValueAsObject(BBKeys::GTargetActor, SightActor);
		// Reset cooldown on new sighting
		bIsCoolingDown = false;
	}
	
	// Manage Update loop
	const bool bShouldRunUpdates = bIsSensed || CurrentAlertValue > 0.f;
	
	if (bShouldRunUpdates)
	{
		GetWorld()->GetTimerManager().SetTimer(
			AlertTimerHandle,
			this,
			&UAIStealthComponent::UpdateAlertLogic,
			TIMER_RATE,
			true);
		return;
	}
	
	// Only stop updates if we are in idle and lost sight of the player physically
	const bool bShouldStopUpdates = !bIsSensed && CurrentStealthState != EStealthState::Idle;
	
	if (bShouldStopUpdates)
	{
		GetWorld()->GetTimerManager().ClearTimer(AlertTimerHandle);
	}
}

void UAIStealthComponent::HandleHearingStimulus(AActor* HearingActor, FAIStimulus Stimulus)
{
	if (!GetOwner()->HasAuthority() || !Stimulus.WasSuccessfullySensed() || !IsValid(BlackboardComp))
	{
		return;
	}
	
	// Update blackboard awareness
	BlackboardComp->SetValueAsVector(BBKeys::GStimulusLocation, Stimulus.StimulusLocation);
	BlackboardComp->SetValueAsObject(BBKeys::GTargetActor, HearingActor);
	
	// Calculate Impact
	const float AddedAlert = Tuning.HearingAlertAdd * Stimulus.Strength;
	
	float NewVal = FMath::Clamp(CurrentAlertValue + AddedAlert, 0.f, MAX_ALERT_VALUE);
	
	TimeSinceLastStimulus = 0.f;
	
	EvaluateStateTransition(NewVal);
	
	GetWorld()->GetTimerManager().SetTimer(
		AlertTimerHandle,
		this,
		&UAIStealthComponent::UpdateAlertLogic,
		TIMER_RATE,
		true);
	
	// Debug
	if (StealthDebugCVars::CVarDebugStealth.GetValueOnGameThread() && StealthDebugCVars::CVarShowSound.GetValueOnGameThread())
	{
		DrawDebugSoundStimulus(Stimulus.StimulusLocation, Stimulus.Strength, Stimulus.Tag);
	}
}

void UAIStealthComponent::HandleSquadStimulus(AActor* TargetActor, FVector TargetLocation, float AlertAmount)
{
	if (!GetOwner()->HasAuthority() || !IsValid(BlackboardComp)) return;
	
	// Update blackboard awareness
	BlackboardComp->SetValueAsVector(BBKeys::GStimulusLocation, TargetLocation);
	if (IsValid(TargetActor))
	{
		BlackboardComp->SetValueAsObject(BBKeys::GTargetActor, TargetActor);
	}
	
	// Apply Alert
	float NewVal = FMath::Clamp(CurrentAlertValue + AlertAmount, 0.f, MAX_ALERT_VALUE);
	
	TimeSinceLastStimulus = 0.f;
	
	EvaluateStateTransition(NewVal);
	
	// Ensure Update Loop is running
	GetWorld()->GetTimerManager().SetTimer(
		AlertTimerHandle,
		this,
		&UAIStealthComponent::UpdateAlertLogic,
		TIMER_RATE,
		true);
}

#pragma endregion

#pragma region Logic Loop

void UAIStealthComponent::UpdateAlertLogic()
{
	if (!GetOwner()->HasAuthority() || !IsValid(BlackboardComp)) return;

	AActor* Target = GetTargetActor();
	const bool bHasLOS = BlackboardComp->GetValueAsBool(BBKeys::GHasLOS);
	
	// Calculate Gain
	float DeltaChange = 0.f;
	bool bIsGainingAlert = false;
	
	if (bHasLOS && Target)
	{
		const float Gain = CalculateSightGain(Target);
		
		// Add Alertness
		if (Gain > KINDA_SMALL_NUMBER)
		{
			DeltaChange = Gain;
			TimeSinceLastStimulus = 0.f;
			bIsGainingAlert = true;
		}
	}
	
	// Decay Logic
	if (!bIsGainingAlert)
	{
		TimeSinceLastStimulus += TIMER_RATE;
		
		bool bCanDecay = true;
		
		// If MAX Alert + No Cooldown -> No Decay
		if (CurrentAlertValue >= MAX_ALERT_VALUE && !bIsCoolingDown)
		{
			bCanDecay = false;
		}
		// Within Grace Time -> No Decay
		else if (TimeSinceLastStimulus < Tuning.GraceTime)
		{
			bCanDecay = false;
		}
	
		if (bCanDecay)
		{
			DeltaChange -= Tuning.DecreaseRate;
		}
	}
	
	// Apply Change
	float NewVal = CurrentAlertValue + (DeltaChange * TIMER_RATE);
    
	// Instant Discovery Override
	if (DeltaChange > 0.f && Target) 
	{
		const float DistSq = FVector::DistSquared(GetOwner()->GetActorLocation(), Target->GetActorLocation());
		if (DistSq < FMath::Square(Tuning.InstantDiscoveryRadius))
		{
			NewVal = MAX_ALERT_VALUE;
		}
	}
	
	// Dead Target Check
	if (Target)
	{
		const AIsekaiCharacterBase* TargetChar = Cast<AIsekaiCharacterBase>(Target);
		if (TargetChar && TargetChar->IsDead())
		{
			BlackboardComp->ClearValue(BBKeys::GTargetActor);
			BlackboardComp->ClearValue(BBKeys::GLastKnownPosition);
			BlackboardComp->SetValueAsBool(BBKeys::GHasLOS, false);
			NewVal = 0.f;
		}
	}
	
	NewVal = FMath::Clamp(NewVal, 0.f, MAX_ALERT_VALUE);
	
	if (NewVal < KINDA_SMALL_NUMBER && bIsCoolingDown)
	{
		bIsCoolingDown = false;
	}
	
	// Evaluate State
	EvaluateStateTransition(NewVal);
}

void UAIStealthComponent::EvaluateStateTransition(float NewVal)
{
	// Commit new alert value
	const float OldVal = CurrentAlertValue;
	CurrentAlertValue = NewVal;
	
	EStealthState NewState = CurrentStealthState;
	
	// Alerted or Searching
	if (NewVal >= MAX_ALERT_VALUE)
	{
		const AActor* TargetActor = GetTargetActor();
		const bool bHasLOS = BlackboardComp ? BlackboardComp->GetValueAsBool(BBKeys::GHasLOS) : false;
		bool bIsCrouching;
		const float VisMod = GetVisibilityModifier(TargetActor, /*out*/ bIsCrouching);

		// Default to Searching
		NewState = EStealthState::Searching;
		
		// Upgrade to Alerted if conditions met:
		// 1. Valid Target
		// 2. Has LOS
		// 3. Within Chase Radius
		// 4. TODO: Within Spottable radius (based on visibility modifier)
		if (IsValid(TargetActor) && bHasLOS && VisMod > KINDA_SMALL_NUMBER)
		{
			const float DistSq = FVector::DistSquared(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation());
			
			if (DistSq < FMath::Square(Tuning.ChaseDistanceThreshold))
			{
				NewState = EStealthState::Alerted;
			}
		}
	}
	else if (NewVal > Tuning.SuspiciousThreshold)
	{
		NewState = EStealthState::Suspicious;
	}
	else
	{
		NewState = EStealthState::Idle;
	}
	
	
	// Squad logic
	/** If entering Alerted from a lower state, notify squad */
	if (NewState == EStealthState::Alerted && CurrentStealthState != EStealthState::Alerted)
	{
		if (UAISquadComponent* SquadComp = Cast<UAISquadComponent>(GetOwner()->GetComponentByClass(UAISquadComponent::StaticClass())))
		{
			AActor* TargetActor = GetTargetActor();
			SquadComp->BroadcastEnemySpotted(TargetActor);
		}
	}
	
	const bool bStateChanged = NewState != CurrentStealthState;
	const bool bAlertChanged = !FMath::IsNearlyEqual(OldVal, CurrentAlertValue);
	
	// Set LKP if transitioning from Alerted to lower state
	if (bStateChanged && CurrentStealthState == EStealthState::Alerted && BlackboardComp)
	{
		const AActor* TargetActor = GetTargetActor();
		if (IsValid(TargetActor))
		{
			BlackboardComp->SetValueAsVector(BBKeys::GLastKnownPosition, TargetActor->GetActorLocation());
		}
	}
	
	if (bStateChanged || bAlertChanged)
	{
		CurrentStealthState = NewState;
	
		SyncToBlackboard();
		
		BroadcastStateChange();
	}
}

#pragma endregion

#pragma region Helpers

float UAIStealthComponent::CalculateSightGain(const AActor* Target) const
{
	if (!Target) return 0.f;

	bool bIsCrouching;
	const float VisMod = GetVisibilityModifier(Target, bIsCrouching);
	if (VisMod <= KINDA_SMALL_NUMBER) return 0.f; // Completely Hidden
	
	const float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());

	// Temp implementation: SightModification affects being Spotted distance
	float ModifiedMaxSightDistance =
		FMath::Lerp(Tuning.SightDistanceFalloffStart, Tuning.SightDistanceFalloffEnd, VisMod);
	
	if (Dist > ModifiedMaxSightDistance)
	{
		return 0.f; // Too far to see crouching target
	}
	
	const float DistFactor = 1.f - FMath::GetMappedRangeValueClamped(
		FVector2D(Tuning.SightDistanceFalloffStart, Tuning.SightDistanceFalloffEnd),
		FVector2D(0.f, 1.f),
		Dist);

	const float BaseGain = FMath::Lerp(Tuning.SightMinGainRate, Tuning.BaseSightGainRate, DistFactor);
	
	return BaseGain * VisMod;
}

float UAIStealthComponent::GetVisibilityModifier(const AActor* Target, bool& bOutIsCrouching) const
{
	const UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
	if (!TargetASC) return 1.f;
	
	float Multiplier = 1.f;
	bOutIsCrouching = false;
	
	if (TargetASC->HasMatchingGameplayTag(Tags::State::Movement_Crouching))
	{
		bOutIsCrouching = true;
	}
	
	for (const auto& Elem : Tuning.TargetTagModifiers)
	{
		if (TargetASC->HasMatchingGameplayTag(Elem.TargetTag))
		{
			if (Elem.GainMultiplier == 0.f) return 0.f;
			
			Multiplier *= Elem.GainMultiplier;
		}
	}
	
	return Multiplier;
}

void UAIStealthComponent::BroadcastStateChange() const
{
	FStealthStateData Data;
	Data.AlertValue = CurrentAlertValue;
	Data.MaxAlertValue = MAX_ALERT_VALUE;
	Data.SuspicionThreshold = Tuning.SuspiciousThreshold;
	Data.CurrentState = CurrentStealthState;
	
	OnStealthUpdate.Broadcast(Data);
}

void UAIStealthComponent::OnRep_StealthState()
{
	BroadcastStateChange();
}

void UAIStealthComponent::SyncToBlackboard() const
{
	if (!BlackboardComp) return;
	
	BlackboardComp->SetValueAsFloat(BBKeys::GAlertLevel, CurrentAlertValue);
	BlackboardComp->SetValueAsEnum(BBKeys::GStealthState, static_cast<uint8>(CurrentStealthState));
}

AActor* UAIStealthComponent::GetTargetActor() const
{
	if (!BlackboardComp) return nullptr;
	return Cast<AActor>(BlackboardComp->GetValueAsObject(BBKeys::GTargetActor));
}

FStealthStateData UAIStealthComponent::GetCurrentStealthStateData() const
{
	FStealthStateData CurrentData;
	CurrentData.AlertValue = CurrentAlertValue;
	CurrentData.MaxAlertValue = MAX_ALERT_VALUE;
	CurrentData.SuspicionThreshold = Tuning.SuspiciousThreshold;
	CurrentData.CurrentState = CurrentStealthState;
	
	return CurrentData;
}

#pragma endregion

#pragma region Debug Suite

void UAIStealthComponent::DrawDebugStealth()
{
	if (!StealthDebugCVars::CVarDebugStealth.GetValueOnGameThread())
	{
		return;
	}

	if (!GetOwner()) return;
	
	const FVector Origin = GetOwner()->GetActorLocation();
	FVector EyeLocation = Origin;
	
	if (const APawn* P = Cast<APawn>(GetOwner()))
	{
		EyeLocation = P->GetPawnViewLocation();
	}

	if (StealthDebugCVars::CVarShowRanges.GetValueOnGameThread())
	{
		DrawDebugRanges(Origin, EyeLocation);
	}

	if (StealthDebugCVars::CVarShowStats.GetValueOnGameThread())
	{
		DrawDebugLiveStats(Origin, EyeLocation);
	}

	if (StealthDebugCVars::CVarShowStimuli.GetValueOnGameThread())
	{
		DrawDebugStimuliHistory(Origin);
	}
}

void UAIStealthComponent::DrawDebugRanges(const FVector& Center, const FVector& EyeLocation) const
{
	const UWorld* World = GetWorld();
	if (!World) return;

	// Only draw relevant rings based on state
	const bool bIsSearching = CurrentStealthState == EStealthState::Searching;
	const bool bIsAlerted = CurrentStealthState == EStealthState::Alerted;
	const bool bIsCombat = bIsAlerted || bIsSearching;

	// Helper lambda for labeling rings
	auto DrawLabel = [&](const FString& Text, float Radius, FColor Color)
	{
		DrawDebugString(World, Center + FVector(Radius, 0, 0), Text, nullptr, Color, 0.f, false);
	};

	// 1. Instant Discovery
	if (!bIsCombat)
	{
		DrawDebugCircle(World, Center, Tuning.InstantDiscoveryRadius, 36, FColor::Red, false, -1.f, 0, 2.f, FVector(1,0,0), FVector(0,1,0));
		DrawLabel(TEXT("Instant"), Tuning.InstantDiscoveryRadius, FColor::Red);
	}

	// 2. Vision Falloff
	if (!bIsCombat)
	{
		// Start of Falloff (Max Gain)
		DrawDebugCircle(World, Center, Tuning.SightDistanceFalloffStart, 36, FColor::Orange, false, -1.f, 0, 1.f, FVector(1,0,0), FVector(0,1,0));
		DrawLabel(TEXT("Max Gain End"), Tuning.SightDistanceFalloffStart, FColor::Orange);
		
		// End of Falloff (Min Gain)
		DrawDebugCircle(World, Center, Tuning.SightDistanceFalloffEnd, 36, FColor::Yellow, false, -1.f, 0, 1.f, FVector(1,0,0), FVector(0,1,0));
		DrawLabel(TEXT("Min Gain End"), Tuning.SightDistanceFalloffEnd, FColor::Yellow);
	}

	// 3. Chase / Search Thresholds
	if (bIsAlerted)
	{
		// Chase Radius
		DrawDebugCircle(World, Center, Tuning.ChaseDistanceThreshold, 36, FColor::Red, false, -1.f, 0, 3.f, FVector(1,0,0), FVector(0,1,0));
		DrawLabel(TEXT("Chase Limit"), Tuning.ChaseDistanceThreshold, FColor::Red);
	}
	else if (bIsSearching)
	{
		// Search Radius
		DrawDebugCircle(World, Center, Tuning.SearchDistanceThreshold, 36, FColor::Purple, false, -1.f, 0, 2.f, FVector(1,0,0), FVector(0,1,0));
		DrawLabel(TEXT("Search Area"), Tuning.SearchDistanceThreshold, FColor::Purple);
	}
}

void UAIStealthComponent::DrawDebugLiveStats(const FVector& Center, const FVector& EyeLocation) const
{
	// Color now dynamically matches state
	const FColor StateColor = GetCurrentStateColor();
	
	FString DebugText = FString::Printf(TEXT("[%s]"), *UEnum::GetValueAsString(CurrentStealthState));
	
	// Add Alert Value
	DebugText += FString::Printf(TEXT("\nAlert: %.1f / %.1f"), CurrentAlertValue, MAX_ALERT_VALUE);
	
	// Add Logic info
	if (bIsCoolingDown) DebugText += TEXT("\n[COOLDOWN]");
	if (TimeSinceLastStimulus < Tuning.GraceTime) 
	{
		DebugText += FString::Printf(TEXT("\nGrace: %.1fs"), Tuning.GraceTime - TimeSinceLastStimulus);
	}

	// Calculate current gain/loss for display
	float CurrentRate = -Tuning.DecreaseRate;
	AActor* Target = GetTargetActor();
	if (BlackboardComp && BlackboardComp->GetValueAsBool(BBKeys::GHasLOS) && Target)
	{
		const float Gain = CalculateSightGain(Target);
		if (Gain > 0.f)
		{
			CurrentRate = Gain;
			// Add Visibility Mod info
			bool bIsCrouching;
			const float VisMod = GetVisibilityModifier(Target, bIsCrouching);
			DebugText += FString::Printf(TEXT("\nVis Mod: x%.2f"), VisMod);
		}
	}

	// Only show rate if it's changing or if we are gaining.
	// Hides negative rate if we are already at 0.
	const bool bIsZeroAndDecaying = (CurrentAlertValue <= 0.001f && CurrentRate < 0.f);

	if (!bIsZeroAndDecaying)
	{
		DebugText += FString::Printf(TEXT("\nRate: %+.1f/s"), CurrentRate);
	}

	// Draw overhead
	DrawDebugString(GetWorld(), Center + FVector(0,0,180.f), DebugText, nullptr, StateColor, 0.f, true, 1.2f);
}

void UAIStealthComponent::DrawDebugStimuliHistory(const FVector& Center) const
{
	if (!BlackboardComp) return;

	// Draw line to Current Target
	if (AActor* Target = GetTargetActor())
	{
		const bool bHasLOS = BlackboardComp->GetValueAsBool(BBKeys::GHasLOS);
		const FColor LineColor = bHasLOS ? FColor::Red : FColor::Silver;
		const float Thickness = bHasLOS ? 3.f : 1.f;
		
		DrawDebugLine(GetWorld(), Center, Target->GetActorLocation(), LineColor, false, -1.f, 0, Thickness);
		
		if (bHasLOS)
		{
			DrawDebugSphere(GetWorld(), Target->GetActorLocation(), 15.f, 8, FColor::Red, false, -1.f, 0, 1.f);
		}
	}

	// Draw Last Known Position (LKP)
	const FVector LKP = BlackboardComp->GetValueAsVector(BBKeys::GLastKnownPosition);
	if (!LKP.IsZero())
	{
		// Draw a "Ghost" capsule to represent where the AI thinks the player is
		DrawDebugCapsule(GetWorld(), LKP, 88.f, 34.f, FQuat::Identity, FColor::Yellow, false, -1.f, 0, 1.f);
		DrawDebugString(GetWorld(), LKP, TEXT("Last Known Position"), nullptr, FColor::Yellow, 0.f, true);
		
		// line from actor to LKP to show path intent
		DrawDebugLine(GetWorld(), Center, LKP, FColor::Yellow, false, -1.f, 0, 1.f);
	}
}

void UAIStealthComponent::DrawDebugSoundStimulus(const FVector& Location, float Loudness, FName NoiseTag) const
{
	const UWorld* World = GetWorld();
	if (!World) return;

	// Map loudness to a visible radius.
	const float ClampedLoudness = FMath::Max(0.f, Loudness);
	const float Radius = ClampedLoudness * 100.f;

	// Draw sound ripple and annotate loudness + tag
	DrawDebugCircle(World, Location, Radius, 32, FColor::Cyan, false, 3.f, 0, 2.f, FVector(1,0,0), FVector(0,1,0));

	FString TagText = NoiseTag.IsNone() ? TEXT("None") : NoiseTag.ToString();
	if (!TagText.Equals(TEXT("None")))
	{
		int32 DotIdx = INDEX_NONE;
		if (TagText.FindChar('.', DotIdx) && DotIdx != INDEX_NONE)
		{
			TagText = TagText.Mid(DotIdx + 1);
		}
	}
	DrawDebugString(World, Location + FVector(0,0,20.f), FString::Printf(TEXT("Loudness: %.2f\nTag: %s"), ClampedLoudness, *TagText), nullptr, FColor::Cyan, 3.f, true);
}

FColor UAIStealthComponent::GetCurrentStateColor() const
{
	switch (CurrentStealthState)
	{
	case EStealthState::Idle: return FColor::Green;
	case EStealthState::Suspicious: return FColor::Yellow;
	case EStealthState::Searching: return FColor::Orange;
	case EStealthState::Alerted: return FColor::Red;
	default: return FColor::White;
	}
}

#pragma endregion
