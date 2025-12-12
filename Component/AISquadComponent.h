// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIAssessment/AI/IsekaiAITypes.h"
#include "Components/ActorComponent.h"
#include "AISquadComponent.generated.h"


class UAISquadSubsystem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AIASSESSMENT_API UAISquadComponent : public UActorComponent
{
	GENERATED_BODY()

	friend UAISquadSubsystem;
public:
	UAISquadComponent();

	// --- Messaging Interface ---
	void BroadcastEnemySpotted(AActor* EnemyActor);
	/** Generic message broadcast */
	void BroadcastMessage(const FSquadMessage& Msg);
	
	int32 GetSquadID() const { return SquadID; }
	void SetSquadID(const int32 NewSquadID);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// --- Messaging Handlers ---
	/** Relay message to other squad members, Only called by the Squad Subsystem */
	void ReceiveMessage(const FSquadMessage& Msg) const;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Isekai", meta=(ClampMin="0"))
	int32 SquadID = -1;
	
private:
	UAISquadSubsystem* GetSquadSubsystem();
	TWeakObjectPtr<UAISquadSubsystem> CachedSquadSubsystem;
};
