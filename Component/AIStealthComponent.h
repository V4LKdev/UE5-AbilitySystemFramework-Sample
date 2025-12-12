// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIAssessment/AI/IsekaiAITypes.h"
#include "Components/ActorComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "AIStealthComponent.generated.h"

class UAbilitySystemComponent;
class AAIController;
class UBlackboardComponent;

/** 
 * Data payload for UI updates.
 * Contains everything the UI needs to render the stealth widget correctly.
 */
USTRUCT(BlueprintType)
struct FStealthStateData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float AlertValue = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float MaxAlertValue = 100.f;

	UPROPERTY(BlueprintReadOnly)
	float SuspicionThreshold = 20.f;

	UPROPERTY(BlueprintReadOnly)
	EStealthState CurrentState = EStealthState::Idle;
	
	bool IsPreSuspicion() const { return AlertValue > 0.f && AlertValue < SuspicionThreshold; }
	
	float GetAlertPercentageTotal() const
	{
		return FMath::Clamp(AlertValue / MaxAlertValue, 0.f, 1.f);
	}
	
	float GetAlertPercentageSuspicion() const
	{
		if (AlertValue < SuspicionThreshold)
		{
			return 0.f;
		}
		return FMath::Clamp((AlertValue - SuspicionThreshold) / (MaxAlertValue - SuspicionThreshold), 0.f, 1.f);
	}
};

constexpr float MAX_ALERT_VALUE = 100.f;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnStealthUpdateSignature, const FStealthStateData& /*NewData*/);

/** 
 * Manages AI stealth logic, alert levels, and state transitions.
 * 
 * ARCHITECTURE NOTE:
 * This component lives on the PAWN (AICharacter) to support replication to clients.
 * However, its Logic (UpdateAlert, Stimuli) is driven exclusively by the Server-Only AIController.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AIASSESSMENT_API UAIStealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAIStealthComponent();
	
	// --- Lifecycle ---
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	// --- Server Only Logic ---
	void Init(AAIController* AICon, UBlackboardComponent* Blackboard, const FAlertTuning& Tuning);
	void Reset();
	
	UFUNCTION(BlueprintCallable, Category="Isekai|AI|Stealth")
	void CompleteSearch();
	
	// --- Stimuli Entry Points ---
	void HandleSightStimulus(AActor* SightActor, const FAIStimulus& Stimulus);
	void HandleHearingStimulus(AActor* HearingActor, FAIStimulus Stimulus);
	void HandleSquadStimulus(AActor* TargetActor, FVector TargetLocation, float AlertAmount);
	
	// --- Public Getters ---
	UFUNCTION(BlueprintPure, Category="Isekai|AI|Stealth")
	float GetAlertValue() const { return CurrentAlertValue; }
	
	FAlertTuning GetTuning() const { return Tuning; }
	
	UFUNCTION(BlueprintPure, Category="Isekai|AI|Stealth")
	EStealthState GetCurrentStealthState() const { return CurrentStealthState; }
	
	UFUNCTION(BlueprintPure, Category="Isekai|AI|Stealth")
	FStealthStateData GetCurrentStealthStateData() const;
	
	// --- Delegates ---
	FOnStealthUpdateSignature OnStealthUpdate;
	
	// --- Debug ---
	UFUNCTION(BlueprintCallable, Category="Isekai|Debug")
	void DrawDebugStealth();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// --- Internal logic ---
	void UpdateAlertLogic();
	void BroadcastStateChange() const;
	
	// --- Calc Helpers ---
	float CalculateSightGain(const AActor* Target) const;
	float GetVisibilityModifier(const AActor* Target, bool& bOutIsCrouching) const;
	
	// --- State Management ---
	void EvaluateStateTransition(float NewAlertValue);
	
	// --- Blackboard Helpers ---
	void SyncToBlackboard() const;
	AActor* GetTargetActor() const;
	
	// --- Replicated Properties ---
	UPROPERTY(ReplicatedUsing=OnRep_StealthState, VisibleAnywhere, BlueprintReadOnly)
	float CurrentAlertValue = 0.f;
	UPROPERTY(ReplicatedUsing=OnRep_StealthState, VisibleAnywhere, BlueprintReadOnly)
	EStealthState CurrentStealthState;
	
	UFUNCTION()
	void OnRep_StealthState();
	
	// --- Internal State ---
	UPROPERTY()
	TObjectPtr<AAIController> OwnerController;
    
	UPROPERTY()
	TObjectPtr<UBlackboardComponent> BlackboardComp;

	FAlertTuning Tuning;
	FTimerHandle AlertTimerHandle;
	
	float TimeSinceLastStimulus = 0.f; 
	
	UPROPERTY(VisibleAnywhere, Category="Isekai|Debug")
	bool bIsCoolingDown = false;
	
	const float TIMER_RATE = 0.1f; // 10 updates per second
	
	// --- Debug Internals ---
	void DrawDebugRanges(const FVector& Center, const FVector& EyeLocation) const;
	void DrawDebugLiveStats(const FVector& Center, const FVector& EyeLocation) const;
	void DrawDebugStimuliHistory(const FVector& Center) const;
	void DrawDebugSoundStimulus(const FVector& Location, float Loudness, FName NoiseTag) const;
	
	// Helper to get color based on current state
	FColor GetCurrentStateColor() const;
};
