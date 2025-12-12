// Copyright V4LKdev and Vlad. All rights reserved.

#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "IsekaiAITypes.generated.h"

UENUM(BlueprintType)
enum class EStealthState : uint8
{
	None 		UMETA(Hidden),
	Idle 		UMETA(DisplayName="Idle"),
	Suspicious	UMETA(DisplayName="Suspicious"),
	Searching	UMETA(DisplayName="Searching"),
	Alerted		UMETA(DisplayName="Alerted")
};

UENUM(BlueprintType)
enum EIsekaiTeamID : uint8
{
	None = 255 UMETA(Hidden),
	Player = 0 UMETA(DisplayName="Player"),
	Enemy = 1 UMETA(DisplayName="Enemy"),
	Neutral = 2 UMETA(DisplayName="Neutral")
	// Factions could be added here later
};

/** 
 * Modifier to adjust alert gain rate based on target tags. e.g. Crouching or Invisible targets.
 */
USTRUCT(BlueprintType)
struct AIASSESSMENT_API FAIAlertTargetTagModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Isekai|AI|Alert", meta=(Categories="State"))
	FGameplayTag TargetTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Isekai|AI|Alert")
	float GainMultiplier = 1.f;
};

/** 
 * Tuning parameters for AI Alert behavior.
 */
USTRUCT(BlueprintType)
struct AIASSESSMENT_API FAlertTuning
{
	GENERATED_BODY()
	
	// --- Sight Params ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sight", meta=(Unit="/s", ToolTip="Alert gain per second when Player is within Start Distance."))
	float BaseSightGainRate = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sight", meta=(Unit="/s", ToolTip="Alert gain per second when Player is at max End Distance."))
	float SightMinGainRate = 5.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sight", meta=(Unit="cm", ToolTip="Within this radius, Gain Rate is at Max (BaseSightGainRate)."))
	float SightDistanceFalloffStart = 500.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sight", meta=(Unit="cm", ToolTip="Past this radius, Gain Rate is at Min (SightMinGainRate)."))
	float SightDistanceFalloffEnd = 2000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sight", meta=(Unit="cm", ToolTip="If player is within this radius, they are instantly spotted (Magic proximity)."))
	float InstantDiscoveryRadius = 150.f;
	
	// --- Distance Thresholds ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Threshold", meta=(Unit="cm", ToolTip="Maximum distance at which the AI will search for the player instead of being alerted"))
	float SearchDistanceThreshold = 1600.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Threshold", meta=(Unit="cm", ToolTip="Distance within which the AI will chase the player once alerted before transitioning into searching"))
	float ChaseDistanceThreshold = 2000.f;
	
	// --- Modifiers ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alert")
	TArray<FAIAlertTargetTagModifier> TargetTagModifiers;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alert", meta=(ToolTip="Amount the alert values has to rise to trigger the suspicious state"))
	float SuspiciousThreshold = 20.f;
	
	// --- Hearing Params ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hearing", meta=(Unit="Flat", ToolTip="Flat amount of Alert to add when hearing a noise at max loudness."))
	float HearingAlertAdd = 25.f;
	
	// --- Squad Propagation Params ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Squad", meta=(ToolTip="Flat amount of Alert to add to squad members when one spots the player."))
	float SquadAlertAdd = 50.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Squad", meta=(Unit="cm", ToolTip="Radius within which squad members are fully alerted when one spots the player."))
	float SquadInstantAlertRadius = 500.f;
	
	// --- Decay & Time ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Time", meta=(Unit="s", ToolTip="Time to wait after losing sight before Alert starts decaying."))
	float GraceTime = 2.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Time", meta=(Unit="/s", ToolTip="How fast Alert drops when not sensing player."))
	float DecreaseRate = 5.f;
};

UENUM(BlueprintType)
enum EAlertUrgency
{
	Info		UMETA(DisplayName="Info"),
	Warning		UMETA(DisplayName="Warning"),
	Critical	UMETA(DisplayName="Critical")
};

USTRUCT(BlueprintType)
struct AIASSESSMENT_API FSquadMessage
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Sender = nullptr;
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> TargetActor = nullptr;
	UPROPERTY(BlueprintReadWrite)
	FVector TargetLocation = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite)
	TEnumAsByte<EAlertUrgency> Urgency = EAlertUrgency::Info;
	UPROPERTY(BlueprintReadWrite, meta=(Categories="SquadMessage"))
	FGameplayTag MessageTag;
};

