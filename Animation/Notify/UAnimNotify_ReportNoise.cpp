// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "UAnimNotify_ReportNoise.h"

#include "AIAssessment/Development/IsekaiStealthSettings.h"
#include "Perception/AISense_Hearing.h"

UUAnimNotify_ReportNoise::UUAnimNotify_ReportNoise()
{
	bIsNativeBranchingPoint = false;
}

void UUAnimNotify_ReportNoise::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}
	
	APawn* OwningPawn = Cast<APawn>(MeshComp->GetOwner());
	if (!OwningPawn)
	{
		return;
	}
	
	if (OwningPawn->HasAuthority())
	{
		// Lookup Config
		const FNoiseConfig& Config = UIsekaiStealthSettings::GetNoiseConfig(NoiseType);
		
		UAISense_Hearing::ReportNoiseEvent(
			OwningPawn,
			OwningPawn->GetActorLocation(),
			Config.Loudness,
			OwningPawn,
			Config.MaxRange,
			NoiseType.GetTagName());
	}
}
