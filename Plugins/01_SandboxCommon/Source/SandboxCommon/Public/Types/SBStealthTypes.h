#pragma once

#include "CoreMinimal.h"
#include "SBStealthTypes.generated.h"

UENUM(BlueprintType)
enum class ESBStealthState : uint8
{
	Hidden UMETA(DisplayName = "Hidden"),
	Suspicious UMETA(DisplayName = "Suspicious"),
	Detected UMETA(DisplayName = "Detected")
};

UENUM(BlueprintType)
enum class ESBNoiseLoudness : uint8
{
	Silent UMETA(DisplayName = "Silent"),
	Footstep UMETA(DisplayName = "Footstep"),
	Sprint UMETA(DisplayName = "Sprint"),
	Combat UMETA(DisplayName = "Combat"),
	Explosion UMETA(DisplayName = "Explosion")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBNoiseEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float Radius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float Loudness = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	TWeakObjectPtr<AActor> Instigator = nullptr;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBStealthSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float BaseVisibility = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float CrouchVisibilityMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float ShadowVisibilityMultiplier = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float AlertBuildRate = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float AlertDecayRate = 1.0f;
};
