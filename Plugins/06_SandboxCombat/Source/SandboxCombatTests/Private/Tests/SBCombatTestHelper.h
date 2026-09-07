// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SmartObjectDefinition.h"
#include "SBCombatTestHelper.generated.h"

UCLASS()
class USBCombatTestHelper : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<APawn> LastAgroTarget = nullptr;

	UPROPERTY()
	int32 LastBossPhase = -1;

	UFUNCTION()
	void HandleAgroTargetChanged(APawn* NewTarget)
	{
		LastAgroTarget = NewTarget;
	}

	UFUNCTION()
	void HandleBossPhaseChanged(int32 NewPhase)
	{
		LastBossPhase = NewPhase;
	}
};

UCLASS()
class USBTestSmartObjectBehaviorDefinition : public USmartObjectBehaviorDefinition
{
	GENERATED_BODY()
};

