#include "NewProject/NewCharacter.h"

#include "CMC_Extended.h"
#include "AbilitySystemComponent.h"

ANewCharacter::ANewCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UCMC_Extended>(CharacterMovementComponentName))
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	AbilitySystemComponent->SetIsReplicated(true);
}

UCMC_Extended* ANewCharacter::GetExtendedCMC() const
{
	return Cast<UCMC_Extended>(GetCharacterMovement());
}

void ANewCharacter::BeginPlay()
{
	Super::BeginPlay();

	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Emerald,
			FString::Printf(TEXT("Current CMC Class : %s"), *GetCharacterMovement()->GetClass()->GetDisplayNameText().ToString()));
	}
}

void ANewCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	SetOwner(NewController);
}
