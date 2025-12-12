// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "BTTask_FindClosestPatrolPoint.h"

#include "NavigationSystem.h"
#include "AIAssessment/Actor/IsekaiPatrolPath.h"
#include "AIAssessment/AI/IsekaiAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_FindClosestPatrolPoint::UBTTask_FindClosestPatrolPoint()
{
	NodeName = "Find Closest Patrol Point";
	PatrolIndexKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindClosestPatrolPoint, PatrolIndexKey));
	MoveToLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindClosestPatrolPoint, MoveToLocationKey));
	PatrolDirectionKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindClosestPatrolPoint, PatrolDirectionKey));
}

EBTNodeResult::Type UBTTask_FindClosestPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AIsekaiAIController* AICon = Cast<AIsekaiAIController>(OwnerComp.GetAIOwner());
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AICon || !BB) return EBTNodeResult::Failed;

	const AIsekaiPatrolPath* PathActor = AICon->GetPatrolPath();
	if (!PathActor) return EBTNodeResult::Failed;

	USplineComponent* Spline = PathActor->GetSpline();
	if (!Spline) return EBTNodeResult::Failed;

	FVector MyLoc = AICon->GetPawn()->GetActorLocation();
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	// 1. Find Closest Input Key (Float representation of location on spline, e.g., 2.5)
	float ClosestInputKey = Spline->FindInputKeyClosestToWorldLocation(MyLoc);
	FVector SplineLoc = Spline->GetLocationAtSplineInputKey(ClosestInputKey, ESplineCoordinateSpace::World);

	// 2. Project to NavMesh
	FNavLocation ProjectedLoc;
	bool bOnNavMesh = false;
	if (NavSys)
	{
		bOnNavMesh = NavSys->ProjectPointToNavigation(SplineLoc, ProjectedLoc, FVector(200.f, 200.f, 500.f));
	}
	
	FVector MoveToLoc = bOnNavMesh ? ProjectedLoc.Location : SplineLoc;

	// 3. Determine the Patrol Index
	// If we are at 2.5, we are "past" 2. So we set Index to 2.
	// The NEXT task (GetNextPatrolPoint) will read 2, add 1, and send us to 3.
	// This ensures we merge to the line (2.5), then continue to the next logical point (3).
	int32 NewIndex = FMath::FloorToInt(ClosestInputKey);
	
	// Edge Case: If we are exactly at the end of a non-looping path
	if (!PathActor->IsLooping() && NewIndex >= Spline->GetNumberOfSplinePoints() - 1)
	{
		NewIndex = Spline->GetNumberOfSplinePoints() - 2; // Set to penultimate so we turn around
	}

	// 4. Update Blackboard
	BB->SetValueAsVector(MoveToLocationKey.SelectedKeyName, MoveToLoc);
	BB->SetValueAsInt(PatrolIndexKey.SelectedKeyName, NewIndex);
	
	// Reset Ping-Pong Direction
	BB->SetValueAsInt(PatrolDirectionKey.SelectedKeyName, bRandomizeDirection ? (FMath::RandBool() ? 1 : -1) : PatrolDirection);

	return EBTNodeResult::Succeeded;
}
