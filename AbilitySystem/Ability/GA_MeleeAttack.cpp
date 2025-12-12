// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "GA_MeleeAttack.h"

#include "AIAssessment/IsekaiLoggingChannels.h"
#include "AIAssessment/NativeGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GenericTeamAgentInterface.h"

UGA_MeleeAttack::UGA_MeleeAttack()
{
	FGameplayTagContainer CombinedTags = GetAssetTags();
	CombinedTags.AddTag(Tags::Ability::Combat_MeleeAttack);
	SetAssetTags(CombinedTags);
	
	ActivationPolicy = OnInputTriggered;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	InitialStaminaCost = 15.f;
	bAllowCostOverdraw = false;
}

void UGA_MeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!AttackMontage)
	{
		UE_LOG(LogIsekaiAbilitySystem, Error, TEXT("No Montage assigned to GA_MeleeAttack!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	// Auto-Face Target
	RotateTowardsBestTarget();

	// Play Montage
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, 
		FName("MeleeAttack"), 
		AttackMontage
	);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->ReadyForActivation();
	
	// Wait for Hit Event
	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		Tags::Event::Montage_Hit,
		nullptr,
		false,
		false
	);

	EventTask->EventReceived.AddDynamic(this, &ThisClass::OnEventReceived);
	EventTask->ReadyForActivation();
}

void UGA_MeleeAttack::OnEventReceived(FGameplayEventData EventData)
{
	PerformAttackTrace();
}

void UGA_MeleeAttack::PerformAttackTrace()
{
	if (!GetOwningActorFromActorInfo()->HasAuthority()) return;

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return;

	const FVector Start = Avatar->GetActorLocation();
	const FVector Forward = Avatar->GetActorForwardVector();
	const FVector End = Start + (Forward * AttackRange);

	TArray<FHitResult> OutHits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Avatar);

	const bool bHit = GetWorld()->SweepMultiByChannel(
		OutHits, Start, End, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(AttackRadius), Params
	);
	
	// DrawDebugSphere(GetWorld(), End, AttackRadius, 12, bHit ? FColor::Green : FColor::Red, false, 1.0f);

	if (bHit)
	{
		for (const FHitResult& Hit : OutHits)
		{
			if (AActor* Target = Hit.GetActor())
			{
				if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target))
				{
					// Team Check: Don't hit friends
					const IGenericTeamAgentInterface* AvatarTeamAgent = Cast<IGenericTeamAgentInterface>(Avatar);
					
					if (AvatarTeamAgent)
					{
						if (AvatarTeamAgent->GetTeamAttitudeTowards(*Target) == ETeamAttitude::Friendly)
						{
							continue; // Friendly Fire OFF
						}
					}

					// Apply Damage
					FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass);
					if (SpecHandle.IsValid())
					{
						FGameplayAbilityTargetDataHandle TargetDataHandle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Target);
						auto Handle =
						ApplyGameplayEffectSpecToTarget(
							GetCurrentAbilitySpecHandle(),
							GetCurrentActorInfo(),
							GetCurrentActivationInfo(),
							SpecHandle,
							TargetDataHandle);
					}
				}
			}
		}
	}
}

void UGA_MeleeAttack::RotateTowardsBestTarget()
{
	// Only for Player-Controlled Pawns
	APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (!AvatarPawn || !AvatarPawn->IsPlayerControlled()) return;

	const FVector Start = AvatarPawn->GetActorLocation();
	const FVector Forward = AvatarPawn->GetActorForwardVector();
	const FVector End = Start + (Forward * AutoFaceRadius);

	TArray<FHitResult> OutHits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(AvatarPawn);

	bool bHit = GetWorld()->SweepMultiByChannel(
		OutHits, Start, End, FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(AutoFaceRadius),
		Params
	);

	AActor* BestTarget = nullptr;
	float BestDot = -1.0f; 

	if (bHit)
	{
		const IGenericTeamAgentInterface* AvatarTeamAgent = Cast<IGenericTeamAgentInterface>(AvatarPawn);

		for (const FHitResult& Hit : OutHits)
		{
			AActor* Target = Hit.GetActor();
			if (!Target || Target == AvatarPawn) continue;

			// Team Check: Ignore friends for auto-aim
			if (AvatarTeamAgent)
			{
				const IGenericTeamAgentInterface* TargetTeamAgent = Cast<IGenericTeamAgentInterface>(Target);
				if (TargetTeamAgent && TargetTeamAgent->GetGenericTeamId() == AvatarTeamAgent->GetGenericTeamId())
				{
					continue;
				}
			}
            
			FVector DirToTarget = (Target->GetActorLocation() - Start).GetSafeNormal();
			float Dot = FVector::DotProduct(Forward, DirToTarget);

			// Must be roughly in front (> 45 deg)
			if (Dot > BestDot && Dot > 0.5f) 
			{
				BestDot = Dot;
				BestTarget = Target;
			}
		}
	}

	if (BestTarget)
	{
		FVector LookDir = BestTarget->GetActorLocation() - Start;
		LookDir.Z = 0.f; 
        
		// Instant Snap
		if (!LookDir.IsNearlyZero())
		{
			AvatarPawn->SetActorRotation(LookDir.Rotation());
		}
	}
}

void UGA_MeleeAttack::OnMontageCompleted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGA_MeleeAttack::OnMontageCancelled()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}