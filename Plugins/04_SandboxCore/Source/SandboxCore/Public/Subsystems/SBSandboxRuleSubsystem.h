// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/SBCommonTypes.h"
#include "SBSandboxRuleSubsystem.generated.h"

UCLASS(BlueprintType)
class SANDBOXCORE_API USBSandboxRuleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USBSandboxRuleSubsystem();

	// Avalia se uma regra é atendida por um determinado ator
	UFUNCTION(BlueprintPure, Category = "Sandbox|RuleEngine")
	bool EvaluateRule(const FSBRule& Rule, AActor* TargetActor) const;

	// Avalia uma única condição contra um ator
	UFUNCTION(BlueprintPure, Category = "Sandbox|RuleEngine")
	bool EvaluateCondition(const FSBRuleCondition& Condition, AActor* TargetActor) const;
};
