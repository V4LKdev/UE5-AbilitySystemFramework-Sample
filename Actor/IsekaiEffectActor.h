// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameFramework/Actor.h"
#include "IsekaiEffectActor.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

/** When and how this actor should apply its configured gameplay effects. */
UENUM(BlueprintType)
enum EEffectApplicationPolicy
{
	ApplyOnOverlap,
	ApplyOnEndOverlap,
	DoNotApply
};

/** When and how infinite effects applied by this actor should be removed. */
UENUM(BlueprintType)
enum EEffectRemovalPolicy
{
	RemoveOnEndOverlap,
	DoNotRemove
};

/**
 * Simple actor that applies GameplayEffects to overlapping targets.
 *
 * Can be configured to apply instant, duration, and infinite effects when actors
 * begin or end overlap, and optionally destroy itself after application.
 */
UCLASS()
class AIASSESSMENT_API AIsekaiEffectActor : public AActor
{
	GENERATED_BODY()

public:
	AIsekaiEffectActor();

	/**
	 * Applies the specified GameplayEffectClass to TargetActor via its ASC.
	 * Used internally by overlap handlers and callable from Blueprints.
	 */
	UFUNCTION(BlueprintCallable)
	void ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass);

	/** Called when a target begins overlapping this actor (see application policies). */
	UFUNCTION(BlueprintCallable)
	void OnOverlap(AActor* TargetActor);

	/** Called when a target ends overlapping this actor (see removal/application policies). */
	UFUNCTION(BlueprintCallable)
	void OnEndOverlap(AActor* TargetActor);

	/** Destroy this actor after successfully applying its effects (if true). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	bool bDestroyOnEffectApplication = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TArray<TSubclassOf<UGameplayEffect>> InstantGameplayEffectClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TEnumAsByte<EEffectApplicationPolicy> InstantEffectApplicationPolicy = DoNotApply;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TArray<TSubclassOf<UGameplayEffect>> DurationGameplayEffectClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TEnumAsByte<EEffectApplicationPolicy> DurationEffectApplicationPolicy = DoNotApply;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TArray<TSubclassOf<UGameplayEffect>> InfiniteGameplayEffectClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TEnumAsByte<EEffectApplicationPolicy> InfiniteEffectApplicationPolicy = DoNotApply;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TEnumAsByte<EEffectRemovalPolicy> InfiniteEffectRemovalPolicy = DoNotRemove;

	UPROPERTY()
	TMap<FActiveGameplayEffectHandle, UAbilitySystemComponent*> ActiveEffectHandles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Applied Effects")
	float ActorLevel = 1.f;
	
private:
	void RemoveInfiniteEffectsFrom(AActor* TargetActor);
};
