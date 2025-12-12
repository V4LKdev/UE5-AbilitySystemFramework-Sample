// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "AISquadSubsystem.h"

#include "AIAssessment/IsekaiLoggingChannels.h"
#include "AIAssessment/Component/AISquadComponent.h"

static TAutoConsoleVariable<bool> CVarDebugSquads(
	TEXT("Isekai.Squad.Debug"),
	false,
	TEXT("Enable debug drawing for AI Squad Communication"),
	ECVF_Cheat);

void UAISquadSubsystem::RegisterMember(const int32 SquadID, UAISquadComponent* Member)
{
	if (!Member) return;
	
	auto& Members = Squads.FindOrAdd(SquadID);
	
	if (!Members.Contains(Member))
	{
		Members.Add(Member);
	}
	else
	{
		UE_LOG(LogIsekaiAI, Log, TEXT("UAISquadSubsystem::RegisterMember: Member %s is already registered in Squad %d"),
			*Member->GetName(), SquadID);
	}
}

void UAISquadSubsystem::UnregisterMember(const int32 SquadID, UAISquadComponent* Member)
{
	if (!DoesSquadExist(SquadID))
	{
		UE_LOG(LogIsekaiAI, Log, TEXT("UAISquadSubsystem::UnregisterMember: Squad %d does not exist"), SquadID);
		return;
	}
	
	auto& Members = Squads[SquadID];
	Members.Remove(Member);
	
	if (Members.Num() == 0)
	{
		Squads.Remove(SquadID);
	}
}

void UAISquadSubsystem::BroadcastMessage(int32 SquadID, const FSquadMessage& Message) const
{
	if (!DoesSquadExist(SquadID))
	{
		UE_LOG(LogIsekaiAI, Log, TEXT("UAISquadSubsystem::BroadcastMessage: Squad %d does not exist"), SquadID);
		return;
	}
	
	const auto& Members = Squads[SquadID];
	
	if (CVarDebugSquads.GetValueOnGameThread())
	{
		DrawDebugMessage(Message, Members);
	}
	
	for (const auto& MemberPtr : Members)
	{
		if (MemberPtr.IsValid() && Message.Sender != MemberPtr->GetOwner())
		{
			MemberPtr->ReceiveMessage(Message);
		}
	}
}

TArray<TWeakObjectPtr<UAISquadComponent>> UAISquadSubsystem::GetSquadMembers(int32 SquadID) const
{
	if (!DoesSquadExist(SquadID))
	{
		UE_LOG(LogIsekaiAI, Log, TEXT("UAISquadSubsystem::GetSquadMembers: Squad %d does not exist"), SquadID);
		return {};
	}
	return Squads[SquadID];
}

int32 UAISquadSubsystem::GetSquadMemberCount(int32 SquadID) const
{
	if (!DoesSquadExist(SquadID))
	{
		UE_LOG(LogIsekaiAI, Log, TEXT("UAISquadSubsystem::GetSquadMemberCount: Squad %d does not exist"), SquadID);
		return 0;
	}
	return Squads[SquadID].Num();
}

bool UAISquadSubsystem::DoesSquadExist(const int32 SquadID) const
{
	return Squads.Contains(SquadID) && Squads[SquadID].Num() > 0;
}

void UAISquadSubsystem::DrawDebugMessage(const FSquadMessage& Msg, const TArray<TWeakObjectPtr<UAISquadComponent>>& Members) const
{
	if (!Msg.Sender || !GetWorld()) return;
	
	const FVector Start = Msg.Sender->GetActorLocation() + FVector(0,0,50.f);

	for (const auto& MemberPtr : Members)
	{
		if (MemberPtr.IsValid() && Msg.Sender != MemberPtr->GetOwner())
		{
			const FVector End = MemberPtr->GetOwner()->GetActorLocation() + FVector(0,0,50.f);
			
			DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 2.f, 0, 2.f);
			// label the line with the message tag in the middle
			DrawDebugString(GetWorld(), (Start + End) / 2, Msg.MessageTag.ToString(), nullptr, FColor::White, 2.f);
		}
	}
	
	DrawDebugSphere(GetWorld(), Start, 50.f, 12, FColor::Cyan, false, 2.f);
}
