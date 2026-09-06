#pragma once

#include "CoreMinimal.h"
#include "SBParkourTypes.generated.h"

UENUM(BlueprintType)
enum class ESBParkourActionType : uint8
{
	None UMETA(DisplayName = "None"),
	Vault UMETA(DisplayName = "Vault"),
	Mantle UMETA(DisplayName = "Mantle"),
	Climb UMETA(DisplayName = "Climb")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBParkourObstacleData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	FVector WallLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	FVector WallNormal = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	FVector LedgeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	float ObstacleHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	float ObstacleDepth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	ESBParkourActionType RecommendedAction = ESBParkourActionType::None;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBParkourSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	float ForwardTraceDistance = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	float MinVaultHeight = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	float MaxVaultHeight = 110.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	float MaxMantleHeight = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	float MaxVaultDepth = 100.0f;
};
