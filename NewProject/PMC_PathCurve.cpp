// Fill out your copyright notice in the Description page of Project Settings.


#include "PMC_PathCurve.h"
#include "Curves/CurveVector.h"

UPMC_PathCurve::UPMC_PathCurve()
{
	PathOffsetCurve = nullptr;
	StartLocation = FVector::ZeroVector;
	TargetLocation = FVector::ZeroVector;
	PathCurveMoveDuration = 0.f;
	CurrentPathCurveMoveTime = 0.f;
	InitialVelocity = FVector::ZeroVector;
}

void UPMC_PathCurve::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = UpdatedComponent->GetComponentLocation();
	if (PathOffsetCurve)
	{
		InitialVelocity = InitialSpeed > 0 ? Velocity.GetSafeNormal() * InitialSpeed : Velocity;
		TargetLocation = StartLocation + InitialVelocity * PathCurveMoveDuration;
	}
}

void UPMC_PathCurve::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (PathOffsetCurve && PathCurveMoveDuration > CurrentPathCurveMoveTime)
	{
		const FVector PathOffsetForce = ComputePathOffsetForce(DeltaTime);
		AddForce(PathOffsetForce);
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FVector UPMC_PathCurve::ComputePathOffsetForce(float DeltaTime)
{
	if (PathCurveMoveDuration > SMALL_NUMBER && DeltaTime > SMALL_NUMBER)
	{
		const float MoveFraction = (CurrentPathCurveMoveTime + DeltaTime) / PathCurveMoveDuration;

		FVector CurrentTargetLocation = FMath::Lerp<FVector, float>(StartLocation, TargetLocation, MoveFraction);
		CurrentTargetLocation += GetPathOffsetInWorldSpace(MoveFraction);

		const FVector CurrentLocation = UpdatedComponent->GetComponentLocation();

		CurrentPathCurveMoveTime += DeltaTime;

		return (CurrentTargetLocation - CurrentLocation) / DeltaTime;
	}

	return FVector::ZeroVector;
}

FVector UPMC_PathCurve::GetPathOffsetInWorldSpace(const float MoveFraction) const
{
	if (PathOffsetCurve)
	{
		float MinCurveTime(0.f);
		float MaxCurveTime(1.f);

		const UCurveVector& Curve = *PathOffsetCurve;
		Curve.GetTimeRange(MinCurveTime, MaxCurveTime);
		Curve.GetVectorValue(FMath::GetRangeValue(FVector2f(MinCurveTime, MaxCurveTime), MoveFraction));
		
		// Calculate path offset
		const FVector PathOffsetInFacingSpace =	Curve.GetVectorValue(FMath::GetRangeValue(FVector2f(MinCurveTime, MaxCurveTime), MoveFraction));
		FRotator FacingRotation((TargetLocation-StartLocation).Rotation());
		FacingRotation.Pitch = 0.f; // By default we don't include pitch in the offset, but an option could be added if necessary
		return FacingRotation.RotateVector(PathOffsetInFacingSpace);
	}

	return FVector::ZeroVector;
}

void UPMC_PathCurve::SetTargetLocation(FVector NewTargetLoc)
{
	TargetLocation = NewTargetLoc;
}