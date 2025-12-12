// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "IsekaiStaminaCostAbility.h"
#include "GA_MeleeAttack.generated.h"

/**
 * 
 */
UCLASS()
class AIASSESSMENT_API UGA_MeleeAttack : public UIsekaiStaminaCostAbility
{
	GENERATED_BODY()
public:
	UGA_MeleeAttack();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** The Montage to play. */
	UPROPERTY(EditDefaultsOnly, Category="Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

	/** The Gameplay Effect to apply to the enemy. */
	UPROPERTY(EditDefaultsOnly, Category="Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** Radius of the hit sphere trace. */
	UPROPERTY(EditDefaultsOnly, Category="Combat")
	float AttackRadius = 80.f;

	/** Forward offset for the hit sphere trace. */
	UPROPERTY(EditDefaultsOnly, Category="Combat")
	float AttackRange = 150.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Combat")
	float AutoFaceRadius = 250.f;

	// --- Internal ---
	
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnEventReceived(FGameplayEventData EventData);

	void PerformAttackTrace();
	void RotateTowardsBestTarget();
};