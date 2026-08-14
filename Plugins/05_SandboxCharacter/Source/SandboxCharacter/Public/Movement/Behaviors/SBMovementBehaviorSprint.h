#pragma once

#include "CoreMinimal.h"
#include "Movement/Behaviors/SBMovementBehavior.h"
#include "SBMovementBehaviorSprint.generated.h"

USTRUCT(BlueprintType)
struct FSBMovementSprintRuntimeData
{
	GENERATED_BODY()

	// Controla se a ativação por toggle está ativa
	UPROPERTY(BlueprintReadWrite, Category = "Sprint")
	bool bToggleActive = false;

	// Acumulador de duração da corrida
	UPROPERTY(BlueprintReadWrite, Category = "Sprint")
	float SprintDuration = 0.0f;
};

UCLASS(Blueprintable, BlueprintType)
class SANDBOXCHARACTER_API USBMovementBehaviorSprint : public USBMovementBehavior
{
	GENERATED_BODY()

public:
	USBMovementBehaviorSprint();

	virtual bool CanEnter_Implementation(const FSBBehaviorContext& Context) const override;
	virtual void Enter_Implementation(const FSBBehaviorContext& Context) override;
	virtual void Update_Implementation(float DeltaTime, const FSBBehaviorContext& Context) override;
	virtual void Exit_Implementation(const FSBBehaviorContext& Context) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Sprint|Runtime")
	FSBMovementSprintRuntimeData SprintRuntimeData;
};
