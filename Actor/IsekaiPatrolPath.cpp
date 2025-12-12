// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "IsekaiPatrolPath.h"

#include "NavigationSystem.h"
#include "Components/SplineComponent.h"


AIsekaiPatrolPath::AIsekaiPatrolPath()
{
	PrimaryActorTick.bCanEverTick = false;
	
	PatrolSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PatrolSpline"));
	PatrolSpline->SetClosedLoop(true);
	RootComponent = PatrolSpline;
}

FVector AIsekaiPatrolPath::GetSplinePointLocation(int32 Index) const
{
	const int32 SafeIndex = Index % FMath::Max(1, GetNumberOfSplinePoints());
	return PatrolSpline->GetLocationAtSplinePoint(SafeIndex, ESplineCoordinateSpace::World);
}

int32 AIsekaiPatrolPath::GetNumberOfSplinePoints() const
{
	return PatrolSpline->GetNumberOfSplinePoints();
}

#if WITH_EDITOR
void AIsekaiPatrolPath::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}
#endif


void AIsekaiPatrolPath::SnapPointsToGround()
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if(!NavSys)
	{
		UE_LOG(LogNavigation, Warning, TEXT("SnapPointsToGround: No Navigation System found in the world."));
		return;
	}
	
	const int32 NumPoints = GetNumberOfSplinePoints();
	for(int32 i = 0; i < NumPoints; ++i)
	{
		const FVector PointLocation = PatrolSpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		
		FNavLocation ProjectedLocation;
		
		if (NavSys->ProjectPointToNavigation(PointLocation, ProjectedLocation, FVector(500.f, 500.f, 1000.f)))
		{
			// Raise the point a bit above ground to avoid clipping
			ProjectedLocation.Location.Z += 25.f;
			PatrolSpline->SetLocationAtSplinePoint(i, ProjectedLocation.Location, ESplineCoordinateSpace::World);
		}
		else
		{
			UE_LOG(LogNavigation, Log, TEXT("SnapPointsToGround: Could not project point %d to navigation."), i);
		}
	}
	
	UE_LOG(LogNavigation, Log, TEXT("SnapPointsToGround: Snapped %d points to navigation."), NumPoints);
}

