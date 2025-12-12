// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "BTTask_GetNextPatrolPoint.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "AIAssessment/IsekaiLoggingChannels.h"
#include "AIAssessment/Actor/IsekaiPatrolPath.h"
#include "AIAssessment/AI/IsekaiAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_GetNextPatrolPoint::UBTTask_GetNextPatrolPoint()
{
	NodeName = TEXT("Get Next Patrol Point");
	
	PatrolIndexKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_GetNextPatrolPoint, PatrolIndexKey));
	MoveToLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_GetNextPatrolPoint, MoveToLocationKey));
	PatrolDirectionKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_GetNextPatrolPoint, PatrolDirectionKey));
}

EBTNodeResult::Type UBTTask_GetNextPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;
	
	
	// Get Patrol Path
	AIsekaiAIController* AICon = Cast<AIsekaiAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AICon)) return EBTNodeResult::Failed;
	
	const AIsekaiPatrolPath* PatrolPath = AICon->GetPatrolPath();
	if (!IsValid(PatrolPath)) return EBTNodeResult::Failed;
	
	
	// Calculate Index
	const int32 NumPoints = PatrolPath->GetNumberOfSplinePoints();
	if (NumPoints == 0) return EBTNodeResult::Failed;
	
	const int32 CurrentIndex = BB->GetValueAsInt(PatrolIndexKey.SelectedKeyName);
	
	int32 Direction = BB->GetValueAsInt(PatrolDirectionKey.SelectedKeyName);
	if (Direction == 0) Direction = 1; // Default to forward
	
	int32 NextIndex = -1;
	
	if (PatrolPath->IsLooping())
	{
		// Standard Loop: 0 -> 1 -> 2 -> ... -> N -> 0
		NextIndex = (CurrentIndex + Direction) % NumPoints;
		if (NextIndex < 0) NextIndex += NumPoints; // Wrap negative
	}
	else
	{
		// Ping-Pong logic 0 -> 1 -> ... -> N -> N-1 -> ... -> 0
		NextIndex = CurrentIndex + Direction;
		
		if (NextIndex >= NumPoints)
		{
			NextIndex = NumPoints - 2; // Go back to Previous
			Direction = -1;
		}
		else if (NextIndex < 0)
		{
			NextIndex = 1; // Go forward again
			Direction = 1;
		}
		
		// Update direction in Blackboard
		BB->SetValueAsInt(PatrolDirectionKey.SelectedKeyName, Direction);
	}
	
	NextIndex = FMath::Clamp(NextIndex, 0, FMath::Max(0, NumPoints - 1));
	
	
	// Resolve Location
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	
	FVector TargetLocation = FVector::ZeroVector;
	bool bFoundValidPoint = false;
	
	
	// Retry loop: If point N is blocked, try N+1 (Max 2 attempts to avoid infinite loops/freeze)
	for (int32 Attempt = 0; Attempt < 2; ++Attempt)
	{
		FVector SplineWorldLoc = PatrolPath->GetSplinePointLocation(NextIndex);
		FNavLocation ProjectedLoc;
		
		// Project to NavMesh
		bool bOnNavmesh = false;
		if (NavSys)
		{
			bOnNavmesh = NavSys->ProjectPointToNavigation(SplineWorldLoc, ProjectedLoc, FVector(200.f, 200.f, 500.f));
		}
		
		TargetLocation = bOnNavmesh ? ProjectedLoc.Location : SplineWorldLoc;
		
		// Check Reachability
		bool bReachable = true;
		if (bCheckReachability && AICon && bOnNavmesh)
		{
			FPathFindingQuery Query;
			Query.StartLocation = AICon->GetPawn()->GetActorLocation();
			Query.EndLocation = TargetLocation;
			Query.NavData = NavSys->GetDefaultNavDataInstance();
			Query.SetNavAgentProperties(AICon->GetNavAgentPropertiesRef());
			
			bReachable = NavSys->TestPathSync(Query, EPathFindingMode::Hierarchical);
		}
		
		if (bOnNavmesh && bReachable)
		{
			bFoundValidPoint = true;
			break;
		}
		else
		{
			UE_LOG(LogIsekaiAI, Warning, TEXT("Patrol Point %d is unreachable or off-mesh. Skipping."), NextIndex);
			NextIndex = (NextIndex + 1) % NumPoints; // Try next
		}
	}
	
	if (!bFoundValidPoint)
	{
		UE_LOG(LogIsekaiAI, Error, TEXT("No valid patrol points found on path %s."), *PatrolPath->GetName());
		return EBTNodeResult::Failed;
	}
	
	// Commit to Blackboard
	BB->SetValueAsVector(MoveToLocationKey.SelectedKeyName, TargetLocation);
	BB->SetValueAsInt(PatrolIndexKey.SelectedKeyName, NextIndex);
	
	return EBTNodeResult::Succeeded;
}
