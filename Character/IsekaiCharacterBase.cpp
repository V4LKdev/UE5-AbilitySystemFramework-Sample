// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "IsekaiCharacterBase.h"

#include "AIAssessment/AbilitySystem/IsekaiAbilitySystemComponent.h"
#include "AIAssessment/AbilitySystem/IsekaiAttributeSet.h"
#include "AIAssessment/Component/IsekaiUIBridge.h"
#include "AIAssessment/IsekaiLoggingChannels.h"
#include "IsekaiCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

AIsekaiCharacterBase::AIsekaiCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UIsekaiCharacterMovementComponent>(CharacterMovementComponentName))
{
	UIBridge = CreateDefaultSubobject<UIsekaiUIBridge>(TEXT("UIBridge"));
}

void AIsekaiCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AIsekaiCharacterBase, bIsDead);
}

UAbilitySystemComponent* AIsekaiCharacterBase::GetAbilitySystemComponent() const
{
	return IsekaiAbilitySystemComponent;
}

UIsekaiCharacterMovementComponent* AIsekaiCharacterBase::GetIsekaiCharacterMovement() const
{
	return Cast<UIsekaiCharacterMovementComponent>(GetCharacterMovement());
}

float AIsekaiCharacterBase::GetHealth() const
{
	return IsekaiAttributeSet ? IsekaiAttributeSet->GetHealth() : 0.0f;
}

float AIsekaiCharacterBase::GetMaxHealth() const
{
	return IsekaiAttributeSet ? IsekaiAttributeSet->GetMaxHealth() : 0.0f;
}

float AIsekaiCharacterBase::GetStamina() const
{
	return IsekaiAttributeSet ? IsekaiAttributeSet->GetStamina() : 0.0f;
}

float AIsekaiCharacterBase::GetMaxStamina() const
{
	return IsekaiAttributeSet ? IsekaiAttributeSet->GetMaxStamina() : 0.0f;
}



void AIsekaiCharacterBase::InitAbilitySystem(AActor* Owner, UIsekaiAbilitySystemComponent* InIsekaiASC, UIsekaiAttributeSet* InIsekaiAS)
{
	IsekaiAbilitySystemComponent = InIsekaiASC;
	IsekaiAttributeSet = InIsekaiAS;
	
	ensureMsgf(IsekaiAbilitySystemComponent,
		TEXT("AIsekaiCharacterBase::InitAbilitySystem: IsekaiAbilitySystemComponent is null"));
	ensureMsgf(IsekaiAttributeSet,
		TEXT("AIsekaiCharacterBase::InitAbilitySystem: IsekaiAttributeSet is null"));
	
	IsekaiAbilitySystemComponent->InitAbilityActorInfo(Owner, this);
	
	UIBridge->InitFromAbilitySystem(IsekaiAbilitySystemComponent, IsekaiAttributeSet);
	
	GetIsekaiCharacterMovement()->InitializeIsekaiReferences(this, IsekaiAbilitySystemComponent);
}


// --- Death handling ---
void AIsekaiCharacterBase::HandleOutOfHealth()
{
	if (bIsDead)
	{
		return;
	}
	
	if (!HasAuthority())
	{
		return;
	}
	
	bIsDead = true; // Triggers OnRep_IsDead on clients
	
	if (IsekaiAbilitySystemComponent)
	{
		// shutoff active abilities
		IsekaiAbilitySystemComponent->HandleOutOfHealth();
	}
	
	HandleVisualsOnDeath();
	
	// Player and AI handling (e.g., respawn, destroy) should be done in subclasses
}

void AIsekaiCharacterBase::OnRep_IsDead()
{
	if (!bIsDead)
	{
		return;
	}
	
	HandleVisualsOnDeath();
}

void AIsekaiCharacterBase::HandleVisualsOnDeath()
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}
	
	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	UE_LOG(LogIsekaiPlayer, Log,
		TEXT("Character %s died on net role %d"),
		*GetName(),
		static_cast<int32>(GetLocalRole()));

	BP_OnDeath();
}