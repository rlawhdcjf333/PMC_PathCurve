// Fill out your copyright notice in the Description page of Project Settings.


#include "NewController.h"

#include "AbilitySystemComponent.h"
#include "NewCharacter.h"
#include "GameFramework/Character.h"


void ANewController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);
	
	if (ANewCharacter* CharacterBase = Cast<ANewCharacter>(P))
	{
		CharacterBase->GetAbilitySystem()->InitAbilityActorInfo(CharacterBase, CharacterBase);
	}
}
