#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "SBCombatFeedbackTypes.generated.h"

UENUM(BlueprintType)
enum class ESBCombatFeedbackIntensity : uint8
{
	Light UMETA(DisplayName = "Light"),
	Medium UMETA(DisplayName = "Medium"),
	Heavy UMETA(DisplayName = "Heavy"),
	Critical UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBHitStopConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFeedback")
	float Duration = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFeedback")
	float TimeDilation = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFeedback")
	bool bAffectAttacker = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFeedback")
	bool bAffectTarget = true;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBCameraShakeConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFeedback")
	TSubclassOf<UCameraShakeBase> CameraShakeClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFeedback")
	float ShakeScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFeedback")
	FVector DirectionalImpulse = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBTemporalDilationConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFeedback")
	float TargetDilation = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFeedback")
	float Duration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFeedback")
	bool bGlobal = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBCombatFeedbackProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFeedback")
	FName ProfileName = FName("Default");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFeedback")
	FSBHitStopConfig HitStop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFeedback")
	FSBCameraShakeConfig CameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatFeedback")
	FSBTemporalDilationConfig Slomo;
};
