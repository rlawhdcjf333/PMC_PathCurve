// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NewController.generated.h"

UCLASS()
class NEWPROJECT_API ANewController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void AcknowledgePossession(APawn* P) override;
};
