// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "IsekaiPatrolPath.generated.h"


UCLASS()
class AIASSESSMENT_API AIsekaiPatrolPath : public AActor
{
	GENERATED_BODY()

public:
	AIsekaiPatrolPath();

	/** Returns world location of a spline point */
	FVector GetSplinePointLocation(int32 Index) const;

	/** Returns total number of points */
	int32 GetNumberOfSplinePoints() const;

	/** Snaps all spline points to the nearest navigation mesh or ground */
	UFUNCTION(CallInEditor, Category="Editor")
	void SnapPointsToGround();
	
	bool IsLooping() const { return PatrolSpline->IsClosedLoop(); }
	
	USplineComponent* GetSpline() const { return PatrolSpline; }
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Path")
	TObjectPtr<USplineComponent> PatrolSpline;

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif

};
