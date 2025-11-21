// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "IsekaiEffectActor.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AIAssessment/IsekaiLoggingChannels.h"


AIsekaiEffectActor::AIsekaiEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AIsekaiEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TargetActor);
	if (!ASI) return;

	UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent();
	if (!TargetASC) return;
	
	if (!GameplayEffectClass)
	{
		UE_LOG(LogIsekaiAbilitySystem, Warning, TEXT("AIsekaiEffectActor::ApplyEffectToTarget: GameplayEffectClass is null"));
		return;
	}
	
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle SpecHandle =
		TargetASC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle);
	
	const FActiveGameplayEffectHandle ActiveEffectHandle =
		TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	
	const bool bIsInfinite = SpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
	if (bIsInfinite && InfiniteEffectRemovalPolicy == RemoveOnEndOverlap)
	{
		ActiveEffectHandles.Add(ActiveEffectHandle, TargetASC);
	}
	
	if (bDestroyOnEffectApplication)
	{
		Destroy();
	}
}

void AIsekaiEffectActor::OnOverlap(AActor* TargetActor)
{
	if (!HasAuthority()) return;
	
	if (InstantEffectApplicationPolicy == ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}
	if (DurationEffectApplicationPolicy == ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}
	if (InfiniteEffectApplicationPolicy == ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}
}

void AIsekaiEffectActor::OnEndOverlap(AActor* TargetActor)
{
	if (!HasAuthority()) return;
	
	if (InstantEffectApplicationPolicy == ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}
	if (DurationEffectApplicationPolicy == ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}
	if (InfiniteEffectApplicationPolicy == ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}
	if (InfiniteEffectRemovalPolicy == RemoveOnEndOverlap)
	{
		RemoveInfiniteEffectsFrom(TargetActor);
	}
}

void AIsekaiEffectActor::RemoveInfiniteEffectsFrom(AActor* TargetActor)
{
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TargetActor);
	UAbilitySystemComponent* TargetASC = ASI ? ASI->GetAbilitySystemComponent() : nullptr;
	if (!TargetASC) return;

	TArray<FActiveGameplayEffectHandle> HandlesToRemove;
	for (const auto& Pair : ActiveEffectHandles)
	{
		if (TargetASC == Pair.Value)
		{
			HandlesToRemove.Add(Pair.Key);
			TargetASC->RemoveActiveGameplayEffect(Pair.Key, 1);
		}
	}

	for (const FActiveGameplayEffectHandle& Handle : HandlesToRemove)
	{
		ActiveEffectHandles.Remove(Handle);
	}
}

