// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimMontage.h"
#include "SBExecutionTypes.generated.h"

UENUM(BlueprintType)
enum class ESBExecutionRole : uint8
{
	Attacker UMETA(DisplayName = "Attacker"),
	Victim UMETA(DisplayName = "Victim")
};

UENUM(BlueprintType)
enum class ESBExecutionState : uint8
{
	Inactive UMETA(DisplayName = "Inactive"),
	Aligning UMETA(DisplayName = "Aligning"),
	Executing UMETA(DisplayName = "Executing"),
	Finished UMETA(DisplayName = "Finished"),
	Aborted UMETA(DisplayName = "Aborted")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBExecutionPairDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Execution")
	FName ExecutionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Execution")
	TObjectPtr<UAnimMontage> AttackerMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Execution")
	TObjectPtr<UAnimMontage> VictimMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Execution")
	FVector RelativeVictimLocation = FVector(100.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Execution")
	FRotator RelativeVictimRotation = FRotator(0.0f, 180.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Execution")
	float ExecutionDuration = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Execution")
	float DamageOnFinish = 9999.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Execution")
	bool bGrantInvulnerabilityToAttacker = true;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBActiveExecution
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Execution")
	TWeakObjectPtr<AActor> AttackerActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Execution")
	TWeakObjectPtr<AActor> VictimActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Execution")
	FSBExecutionPairDefinition Definition;

	UPROPERTY(BlueprintReadOnly, Category = "Execution")
	ESBExecutionState State = ESBExecutionState::Inactive;

	UPROPERTY(BlueprintReadOnly, Category = "Execution")
	float ElapsedTime = 0.0f;
};
