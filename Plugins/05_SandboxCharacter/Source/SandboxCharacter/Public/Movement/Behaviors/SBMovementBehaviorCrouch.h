#pragma once

#include "CoreMinimal.h"
#include "Movement/Behaviors/SBMovementBehavior.h"
#include "Movement/DataAssets/SBMovementBehaviorDefinition.h"
#include "SBMovementBehaviorCrouch.generated.h"

UCLASS(BlueprintType)
class SANDBOXCHARACTER_API USBMovementBehaviorCrouchDefinition : public USBMovementBehaviorDefinition
{
	GENERATED_BODY()

public:
	// Altura reduzida da cápsula de colisão ao agachar (configurável pelo designer)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crouch")
	float CrouchedHalfHeight = 40.0f;
};

USTRUCT(BlueprintType)
struct FSBMovementCrouchRuntimeData
{
	GENERATED_BODY()

	// Controla se a ativação de toggle está ativa
	UPROPERTY(BlueprintReadWrite, Category = "Crouch")
	bool bToggleActive = false;
};

UCLASS(Blueprintable, BlueprintType)
class SANDBOXCHARACTER_API USBMovementBehaviorCrouch : public USBMovementBehavior
{
	GENERATED_BODY()

public:
	USBMovementBehaviorCrouch();

	virtual bool CanEnter_Implementation(const FSBBehaviorContext& Context) const override;
	virtual void Enter_Implementation(const FSBBehaviorContext& Context) override;
	virtual void Update_Implementation(float DeltaTime, const FSBBehaviorContext& Context) override;
	virtual void Exit_Implementation(const FSBBehaviorContext& Context) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Crouch|Runtime")
	FSBMovementCrouchRuntimeData CrouchRuntimeData;
};
