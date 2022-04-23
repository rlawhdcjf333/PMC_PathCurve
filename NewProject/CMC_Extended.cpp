// Fill out your copyright notice in the Description page of Project Settings.


#include "CMC_Extended.h"

#include "GameFramework/Character.h"
#include "ProfilingDebugging/CsvProfiler.h"


void FCharacterNetworkMoveData_Extended::ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove,
	ENetworkMoveType MoveType)
{
	FCharacterNetworkMoveData::ClientFillNetworkMoveData(ClientMove, MoveType);

	const FSavedMove_Character_Extended& ExtendedClientMove = static_cast<const FSavedMove_Character_Extended&>(ClientMove);
	MaxWalkSpeed = ExtendedClientMove.MaxWalkSpeed;
}

bool FCharacterNetworkMoveData_Extended::Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar,
	UPackageMap* PackageMap, ENetworkMoveType MoveType)
{
	NetworkMoveType = MoveType;

	bool bLocalSuccess = true;
	const bool bIsSaving = Ar.IsSaving();

	Ar << TimeStamp;

	// TODO: better packing with single bit per component indicating zero/non-zero
	Acceleration.NetSerialize(Ar, PackageMap, bLocalSuccess);

	Location.NetSerialize(Ar, PackageMap, bLocalSuccess);

	// ControlRotation : FRotator handles each component zero/non-zero test; it uses a single signal bit for zero/non-zero, and uses 16 bits per component if non-zero.
	ControlRotation.NetSerialize(Ar, PackageMap, bLocalSuccess);

	SerializeOptionalValue<uint8>(bIsSaving, Ar, CompressedMoveFlags, 0);

	if (MoveType == ENetworkMoveType::NewMove)
	{
		// Location, relative movement base, and ending movement mode is only used for error checking, so only save for the final move.
		SerializeOptionalValue<UPrimitiveComponent*>(bIsSaving, Ar, MovementBase, nullptr);
		SerializeOptionalValue<FName>(bIsSaving, Ar, MovementBaseBoneName, NAME_None);
		SerializeOptionalValue<uint8>(bIsSaving, Ar, MovementMode, MOVE_Walking);
		SerializeOptionalValue<float>(bIsSaving, Ar, MaxWalkSpeed, 0.f);
	}

	return !Ar.IsError();
}

UCMC_Extended::UCMC_Extended()
	:Super()
{
	SetNetworkMoveDataContainer(ExtendedNetworkMoveDataContainer);
}

void UCMC_Extended::SetMaxWalkSpeed(float InValue)
{
	if (GetOwnerRole() != ROLE_AutonomousProxy)
	{
		return;
	}

	MaxWalkSpeed = InValue;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2, 10, FColor::Emerald,
			FString::Printf(TEXT("Client MaxWalkSpeed Shift Timestamp : %.5f"), GetPredictionData_Client_Character()->CurrentTimeStamp));
	}
}

static float NetServerMoveTimestampExpiredWarningThreshold_Extended = 1.0f;
FAutoConsoleVariableRef CVarNetServerMoveTimestampExpiredWarningThreshold(
	TEXT("net.NetServerMoveTimestampExpiredWarningThreshold"),
	NetServerMoveTimestampExpiredWarningThreshold_Extended,
	TEXT("Tolerance for ServerMove() to warn when client moves are expired more than this time threshold behind the server."),
	ECVF_Default);

DECLARE_CYCLE_STAT(TEXT("Char Extended ServerMove"), STAT_ExtendedCharacterMovementServerMove, STATGROUP_Character);

void UCMC_Extended::ServerMove_PerformMovement(const FCharacterNetworkMoveData& MoveData)
{
	SCOPE_CYCLE_COUNTER(STAT_ExtendedCharacterMovementServerMove);

	if (!HasValidData() || !IsActive())
	{
		return;
	}	

	const float ClientTimeStamp = MoveData.TimeStamp;
	FVector_NetQuantize10 ClientAccel = MoveData.Acceleration;
	const uint8 ClientMoveFlags = MoveData.CompressedMoveFlags;
	const FRotator ClientControlRotation = MoveData.ControlRotation;

	const FCharacterNetworkMoveData_Extended& ExtendedMoveData = static_cast<const FCharacterNetworkMoveData_Extended&>(MoveData);
	const float ClientMaxWalkSpeed = ExtendedMoveData.MaxWalkSpeed;

	FNetworkPredictionData_Server_Character* ServerData = GetPredictionData_Server_Character();
	check(ServerData);

	if( !VerifyClientTimeStamp(ClientTimeStamp, *ServerData) )
	{
		const float ServerTimeStamp = ServerData->CurrentClientTimeStamp;
		// This is more severe if the timestamp has a large discrepancy and hasn't been recently reset.
		if (ServerTimeStamp > 1.0f && FMath::Abs(ServerTimeStamp - ClientTimeStamp) > NetServerMoveTimestampExpiredWarningThreshold_Extended)
		{
			UE_LOG(LogNetPlayerMovement, Warning, TEXT("ServerMove: TimeStamp expired: %f, CurrentTimeStamp: %f, Character: %s"), ClientTimeStamp, ServerTimeStamp, *GetNameSafe(CharacterOwner));
		}
		else
		{
			UE_LOG(LogNetPlayerMovement, Log, TEXT("ServerMove: TimeStamp expired: %f, CurrentTimeStamp: %f, Character: %s"), ClientTimeStamp, ServerTimeStamp, *GetNameSafe(CharacterOwner));
		}		
		return;
	}

	bool bServerReadyForClient = true;
	APlayerController* PC = Cast<APlayerController>(CharacterOwner->GetController());
	if (PC)
	{
		bServerReadyForClient = PC->NotifyServerReceivedClientData(CharacterOwner, ClientTimeStamp);
		if (!bServerReadyForClient)
		{
			ClientAccel = FVector::ZeroVector;
		}
	}

	const UWorld* MyWorld = GetWorld();
	const float DeltaTime = ServerData->GetServerMoveDeltaTime(ClientTimeStamp, CharacterOwner->GetActorTimeDilation(*MyWorld));

	if (DeltaTime > 0.f)
	{
		ServerData->CurrentClientTimeStamp = ClientTimeStamp;
		ServerData->ServerAccumulatedClientTimeStamp += DeltaTime;
		ServerData->ServerTimeStamp = MyWorld->GetTimeSeconds();
		ServerData->ServerTimeStampLastServerMove = ServerData->ServerTimeStamp;

		if (PC)
		{
			PC->SetControlRotation(ClientControlRotation);
		}

		if (!bServerReadyForClient)
		{
			return;
		}

		// Perform actual movement
		if ((MyWorld->GetWorldSettings()->GetPauserPlayerState() == NULL))
		{
			if (PC)
			{
				PC->UpdateRotation(DeltaTime);
			}

			if (FMath::IsNearlyEqual(MaxWalkSpeed, ClientMaxWalkSpeed) == false)
			{
				MaxWalkSpeed = ClientMaxWalkSpeed;
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(1, 10, FColor::Emerald,
						FString::Printf(TEXT("Server MaxWalkSpeed Shif Timestamp : %.5f"), ClientTimeStamp));
				}
			}
			
			MoveAutonomous(ClientTimeStamp, DeltaTime, ClientMoveFlags, ClientAccel);
		}

		UE_CLOG(CharacterOwner && UpdatedComponent, LogNetPlayerMovement, VeryVerbose, TEXT("ServerMove Time %f Acceleration %s Velocity %s Position %s Rotation %s DeltaTime %f Mode %s MovementBase %s.%s (Dynamic:%d)"),
			ClientTimeStamp, *ClientAccel.ToString(), *Velocity.ToString(), *UpdatedComponent->GetComponentLocation().ToString(), *UpdatedComponent->GetComponentRotation().ToCompactString(), DeltaTime, *GetMovementName(),
			*GetNameSafe(GetMovementBase()), *CharacterOwner->GetBasedMovement().BoneName.ToString(), MovementBaseUtility::IsDynamicBase(GetMovementBase()) ? 1 : 0);
	}

	// Validate move only after old and first dual portion, after all moves are completed.
	if (MoveData.NetworkMoveType == FCharacterNetworkMoveData::ENetworkMoveType::NewMove)
	{
		ServerMoveHandleClientError(ClientTimeStamp, DeltaTime, ClientAccel, MoveData.Location, MoveData.MovementBase, MoveData.MovementBaseBoneName, MoveData.MovementMode);
	}
}

FNetworkPredictionData_Client* UCMC_Extended::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UCMC_Extended* MutableThis = const_cast<UCMC_Extended*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Character_Extended(*this);
	}

	return ClientPredictionData;
}

void FSavedMove_Character_Extended::Clear()
{
	FSavedMove_Character::Clear();
	MaxWalkSpeed = 0.f;
}

void FSavedMove_Character_Extended::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,
                                               FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	if (IsValid(C) && IsValid(C->GetCharacterMovement()))
	{
		if (MaxWalkSpeed != C->GetCharacterMovement()->MaxWalkSpeed)
		{
			bForceNoCombine = true;
		}
		
		MaxWalkSpeed = C->GetCharacterMovement()->MaxWalkSpeed;
	}
}

FSavedMovePtr FNetworkPredictionData_Client_Character_Extended::AllocateNewMove()
{
	return  FSavedMovePtr(new FSavedMove_Character_Extended());
}
