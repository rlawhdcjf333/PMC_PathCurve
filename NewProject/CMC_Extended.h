// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CMC_Extended.generated.h"


struct NEWPROJECT_API FCharacterNetworkMoveData_Extended : public FCharacterNetworkMoveData
{
	FCharacterNetworkMoveData_Extended()
		:FCharacterNetworkMoveData()
		,MaxWalkSpeed(0.f)
	{
	}

	float MaxWalkSpeed;

	virtual void ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType) override;
	virtual bool Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType) override;
};

struct NEWPROJECT_API FCharacterNetworkMoveDataContainer_Extended : public FCharacterNetworkMoveDataContainer
{
	FCharacterNetworkMoveDataContainer_Extended()
		: FCharacterNetworkMoveDataContainer()
	{
		NewMoveData		= &ExtendedBaseDefaultMoveData[0];
		PendingMoveData	= &ExtendedBaseDefaultMoveData[1];
		OldMoveData		= &ExtendedBaseDefaultMoveData[2];
	}

private:
	FCharacterNetworkMoveData_Extended ExtendedBaseDefaultMoveData[3];
};

UCLASS()
class NEWPROJECT_API UCMC_Extended : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UCMC_Extended();

	UFUNCTION(BlueprintCallable)
	void SetMaxWalkSpeed(float InValue);

	virtual void ServerMove_PerformMovement(const FCharacterNetworkMoveData& MoveData) override;

	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

private:
	FCharacterNetworkMoveDataContainer_Extended ExtendedNetworkMoveDataContainer;
	
};

class NEWPROJECT_API FSavedMove_Character_Extended : public FSavedMove_Character
{
public:
	FSavedMove_Character_Extended()
		:FSavedMove_Character()
		,MaxWalkSpeed(0.f)
	{
	}

	float MaxWalkSpeed;

	virtual void Clear() override;
	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
	
};

class NEWPROJECT_API FNetworkPredictionData_Client_Character_Extended : public FNetworkPredictionData_Client_Character
{
public:
	FNetworkPredictionData_Client_Character_Extended(const UCharacterMovementComponent& ClientMovement)
		:FNetworkPredictionData_Client_Character(ClientMovement)
	{
	}

	virtual FSavedMovePtr AllocateNewMove() override;
};
