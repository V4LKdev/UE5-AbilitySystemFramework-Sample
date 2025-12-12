// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "AISquadComponent.h"

#include "AIStealthComponent.h"
#include "AIAssessment/NativeGameplayTags.h"
#include "AIAssessment/Subsystem/World/AISquadSubsystem.h"

UAISquadComponent::UAISquadComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// Server only logic
	SetIsReplicatedByDefault(false);
}

void UAISquadComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetSquadSubsystem())
	{
		GetSquadSubsystem()->RegisterMember(SquadID, this);
	}
}

void UAISquadComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetSquadSubsystem())
	{
		GetSquadSubsystem()->UnregisterMember(SquadID, this);
	}
	
	Super::EndPlay(EndPlayReason);
}

void UAISquadComponent::SetSquadID(const int32 NewSquadID)
{
	if (GetSquadSubsystem())
	{
		GetSquadSubsystem()->UnregisterMember(SquadID, this);
		SquadID = NewSquadID;
		GetSquadSubsystem()->RegisterMember(SquadID, this);
	}
	else
	{
		SquadID = NewSquadID;
	}
}


void UAISquadComponent::BroadcastEnemySpotted(AActor* EnemyActor)
{
	FSquadMessage Msg;
	Msg.Sender = GetOwner();
	Msg.TargetActor = EnemyActor;
	Msg.TargetLocation = EnemyActor ? EnemyActor->GetActorLocation() : FVector::ZeroVector;
	Msg.Urgency = EAlertUrgency::Critical;
	Msg.MessageTag = Tags::SquadMessage::EnemySpotted;
	
	BroadcastMessage(Msg);
}

void UAISquadComponent::BroadcastMessage(const FSquadMessage& Msg)
{
	if (GetSquadSubsystem())
	{
		GetSquadSubsystem()->BroadcastMessage(SquadID, Msg);
	}
}

void UAISquadComponent::ReceiveMessage(const FSquadMessage& Msg) const
{
	if (Msg.Sender == GetOwner()) return;
	
	UAIStealthComponent* StealthComp = Cast<UAIStealthComponent>(GetOwner()->GetComponentByClass(UAIStealthComponent::StaticClass()));
	if (!StealthComp) return;
	
	// Ignore if already at max alert
	if (StealthComp->GetAlertValue() >= MAX_ALERT_VALUE) return;
	
	const FAlertTuning& AlertTuning = StealthComp->GetTuning();
	
	const float DistSq = FVector::DistSquared(GetOwner()->GetActorLocation(), Msg.Sender->GetActorLocation());
	const float InstantRadiusSq = FMath::Square(AlertTuning.SquadInstantAlertRadius);
	
	float AlertAmount = 0.f;
	
	if (DistSq <= InstantRadiusSq)
	{
		// Full alert within instant radius
		AlertAmount = MAX_ALERT_VALUE;
	}
	else
	{
		// Apply Base Alertness
		AlertAmount = AlertTuning.SquadAlertAdd;
	}
	
	StealthComp->HandleSquadStimulus(Msg.TargetActor, Msg.TargetLocation, AlertAmount);
}

UAISquadSubsystem* UAISquadComponent::GetSquadSubsystem()
{
	if (!CachedSquadSubsystem.IsValid())
	{
		if (const UWorld* World = GetWorld())
		{
			UAISquadSubsystem* Subsystem = World->GetSubsystem<UAISquadSubsystem>();
			CachedSquadSubsystem = Subsystem;
		}
	}
	return CachedSquadSubsystem.Get();
}
