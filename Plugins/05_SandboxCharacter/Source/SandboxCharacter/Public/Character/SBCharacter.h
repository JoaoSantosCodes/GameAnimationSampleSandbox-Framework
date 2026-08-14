#pragma once

#include "CoreMinimal.h"
#include "ModularCharacter.h"
#include "Interfaces/SBCharacterInterface.h"
#include "SBCharacter.generated.h"

class USBPawnDataAsset;

UCLASS()
class SANDBOXCHARACTER_API ASBCharacter : public AModularCharacter, public ISBCharacterInterface
{
	GENERATED_BODY()

public:
	ASBCharacter();

	// ISBCharacterInterface implementation
	virtual UObject* GetPawnData_Implementation() const override;
	virtual UActorComponent* GetAttributeComponent_Implementation() const override;
	virtual UActorComponent* GetStateComponent_Implementation() const override;
	virtual UActorComponent* GetAbilityComponent_Implementation() const override;

	virtual bool IsLocallyControlled() const override;

private:
	friend class FSBNetworkTestsSpec;
	friend class FSBInteractionTestsSpec;
	void Test_Possess(AController* NewController);

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox|Character")
	TObjectPtr<USBPawnDataAsset> PawnData;

	void InitializeFromPawnData();
};
