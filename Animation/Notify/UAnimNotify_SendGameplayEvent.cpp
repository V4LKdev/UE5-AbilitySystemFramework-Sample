// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "UAnimNotify_SendGameplayEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"

void UUAnimNotify_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                            const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (!MeshComp) return;
	
	AActor* Actor = MeshComp->GetOwner();
	if (!Actor) return;
	
	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
	{
		return;
	}
	
	FGameplayEventData PayloadData;
	PayloadData.EventTag = EventTag;
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Actor, EventTag, PayloadData);
}

FString UUAnimNotify_SendGameplayEvent::GetNotifyName_Implementation() const
{
	return EventTag.IsValid() ? EventTag.ToString() : TEXT("SendGameplayEvent (None)");
}
