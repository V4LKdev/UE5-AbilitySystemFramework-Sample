// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "IsekaiCharacterBase.h"
#include "AIAssessment/AI/IsekaiAITypes.h"
#include "AIAssessment/Component/AISquadComponent.h"
#include "IsekaiAICharacter.generated.h"

class AIsekaiPatrolPath;
class UAIOverheadWidget;
class UWidgetComponent;
class UBehaviorTree;
class UIsekaiAbilitySet;

/**
 * Isekai AI Character base class.
 * Owns its ASC + AttributeSet on the pawn.
 * GAS wiring is still done through AIsekaiCharacterBase::InitAbilitySystem
 * to keep consistent init flow across players and AI.
 */
UCLASS()
class AIASSESSMENT_API AIsekaiAICharacter : public AIsekaiCharacterBase
{
	GENERATED_BODY()

public:
	AIsekaiAICharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void Tick(float DeltaTime) override;
	
	// --- Getters ---
	UBehaviorTree* GetAIBehaviorTree() const { return BehaviorTree; }
	FAlertTuning GetAlertTuning() const { return AlertTuning; }
	UAIStealthComponent* GetStealthComponent() const { return StealthComponent; }
	TSoftObjectPtr<AIsekaiPatrolPath> GetPatrolPath() const { return PatrolPath; }
	
	// --- Overrides ---
	virtual void HandleOutOfHealth() override;
	
	UFUNCTION(BlueprintPure, Category="Isekai|AI|Squad")
	int32 GetSquadID() const { return SquadComponent ? SquadComponent->GetSquadID() : INDEX_NONE; }

protected:
	virtual void BeginPlay() override;
	
	virtual void HandleVisualsOnDeath() override;
	
	/** Safe entry point to initialize the Overhead UI Widget */
	void InitOverheadWidget();
	
	/** Grants default attributes and ability sets to the AI */
	void ApplyStartupData();
	
	// --- Components ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Isekai|AIConfig")
	TObjectPtr<UWidgetComponent> WidgetComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Isekai|AIConfig")
	TObjectPtr<UAIStealthComponent> StealthComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Isekai|AIConfig")
	TObjectPtr<UAISquadComponent> SquadComponent;
	
	// --- AI Config ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Isekai|AIConfig")
	TObjectPtr<UBehaviorTree> BehaviorTree;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Isekai|AIConfig")
	float DestructionDelayAfterDeath = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Isekai")
	FAlertTuning AlertTuning;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Isekai")
	TSoftObjectPtr<AIsekaiPatrolPath> PatrolPath;
	
	// --- GAS Config ---
	UPROPERTY(EditDefaultsOnly, Category="Isekai|GAS")
	TArray<TObjectPtr<UIsekaiAbilitySet>> StartupAbilitySets;
	
	uint8 bStartupDataApplied : 1;
};
