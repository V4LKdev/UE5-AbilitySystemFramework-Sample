// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "IsekaiAICharacter.h"

#include "AIAssessment/IsekaiLoggingChannels.h"
#include "AIAssessment/AbilitySystem/IsekaiAbilitySet.h"
#include "AIAssessment/AbilitySystem/IsekaiAbilitySystemComponent.h"
#include "AIAssessment/AbilitySystem/IsekaiAttributeSet.h"
#include "AIAssessment/AI/IsekaiAIController.h"
#include "AIAssessment/Component/AIStealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIAssessment/Widget/AIOverheadWidget.h"


AIsekaiAICharacter::AIsekaiAICharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AIsekaiAIController::StaticClass();
	
	// GAS Setup
	IsekaiAbilitySystemComponent = CreateDefaultSubobject<UIsekaiAbilitySystemComponent>(TEXT("IsekaiAbilitySystemComponent"));
	IsekaiAbilitySystemComponent->SetIsReplicated(true);
	IsekaiAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	IsekaiAttributeSet = CreateDefaultSubobject<UIsekaiAttributeSet>(TEXT("IsekaiAttributeSet"));
	
	// AI Logic Components
	StealthComponent = CreateDefaultSubobject<UAIStealthComponent>(FName("StealthComponent"));
	StealthComponent->SetIsReplicated(true);
	
	// Character Movement Setup
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bRequestedMoveUseAcceleration = true;
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	
	// UI Widget Setup
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(RootComponent);
	WidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	WidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	WidgetComponent->SetTickMode(ETickMode::Automatic);
	
	TeamID = EIsekaiTeamID::Enemy;
	TeamIdStruct = FGenericTeamId(TeamID);
	
	SquadComponent = CreateDefaultSubobject<UAISquadComponent>(TEXT("SquadComponent"));
	SquadComponent->SetIsReplicated(false);
}

void AIsekaiAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (HasAuthority() && StealthComponent)
	{
		StealthComponent->DrawDebugStealth();
	}
}

void AIsekaiAICharacter::BeginPlay()
{
	Super::BeginPlay();
	
	InitAbilitySystem(this, IsekaiAbilitySystemComponent, IsekaiAttributeSet);
	
	InitOverheadWidget();
	
	if (HasAuthority())
	{
		if (!BehaviorTree)
		{
			UE_LOG(LogIsekaiAI, Error, TEXT("Missing behavior tree asset!"));
			return;
		}
		
		ApplyStartupData();
	}
}

void AIsekaiAICharacter::InitOverheadWidget()
{
	if (!WidgetComponent || !WidgetComponent->GetWidgetClass() || !StealthComponent || !UIBridge)
	{
		return;
	}
	
	if (!WidgetComponent->HasBeenInitialized())
	{
		WidgetComponent->InitWidget();
	}
	

	UAIOverheadWidget* OverheadWidget = Cast<UAIOverheadWidget>(WidgetComponent->GetUserWidgetObject());
	if (!OverheadWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("InitUIWidget: OverheadWidget is invalid!"));
		return;
	}
	
	OverheadWidget->InitFromBridge(UIBridge);
	UIBridge->InitStealthComponent(StealthComponent);
}


void AIsekaiAICharacter::ApplyStartupData()
{
	if (bStartupDataApplied || !IsekaiAbilitySystemComponent)
	{
		return;
	}
	
	for (const UIsekaiAbilitySet* AbilitySet : StartupAbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->GiveToAbilitySystem(IsekaiAbilitySystemComponent, nullptr, this);
		}
	}
	
	bStartupDataApplied = true;
}


void AIsekaiAICharacter::HandleOutOfHealth()
{
	Super::HandleOutOfHealth();
	
	if (!HasAuthority()) return;
	
	if (StealthComponent)
	{
		StealthComponent->Reset();
	}
	
	if (AIsekaiAIController* AIController = Cast<AIsekaiAIController>(GetController()))
	{
		AIController->HandlePawnDeath();
	}
	
	SetLifeSpan(DestructionDelayAfterDeath);
}

void AIsekaiAICharacter::HandleVisualsOnDeath()
{
	Super::HandleVisualsOnDeath();
	
	if (WidgetComponent)
	{
		WidgetComponent->SetVisibility(false);
	}
}
