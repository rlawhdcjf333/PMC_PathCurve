// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PMC_PathCurve.generated.h"

class UCurveVector;

UCLASS(ClassGroup=Custom, meta=(BlueprintSpawnableComponent))
class NEWPROJECT_API UPMC_PathCurve : public UProjectileMovementComponent
{
	GENERATED_BODY()

public:
	
	UPMC_PathCurve();

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ProjectilePathCurve)
	TObjectPtr<UCurveVector> PathOffsetCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ProjectilePathCurve)
	float PathCurveMoveDuration;

	UPROPERTY()
	FVector StartLocation;

	UPROPERTY()
	FVector TargetLocation;

	UPROPERTY()
	float CurrentPathCurveMoveTime;

	UPROPERTY()
	FVector InitialVelocity;

	FVector GetPathOffsetInWorldSpace(const float MoveFraction) const;

	FVector ComputePathOffsetForce(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void SetTargetLocation(FVector NewTargetLoc);
};
