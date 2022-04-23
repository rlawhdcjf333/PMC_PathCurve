#pragma once

#include "GameFramework/Character.h"
#include "NewCharacter.generated.h"

class UAbilitySystemComponent;

UCLASS()
class NEWPROJECT_API ANewCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	ANewCharacter(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	class UCMC_Extended* GetExtendedCMC() const;
	
	UPROPERTY()
	UAbilitySystemComponent* AbilitySystemComponent;

	UFUNCTION(BlueprintCallable)
	UAbilitySystemComponent* GetAbilitySystem() const {return AbilitySystemComponent;}

	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
};