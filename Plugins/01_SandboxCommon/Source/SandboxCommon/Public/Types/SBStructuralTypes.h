#pragma once

#include "CoreMinimal.h"
#include "SBStructuralTypes.generated.h"

UENUM(BlueprintType)
enum class ESBStructuralMaterialTier : uint8
{
	Wood                 UMETA(DisplayName = "Wood"),
	Stone                UMETA(DisplayName = "Stone"),
	Metal                UMETA(DisplayName = "Metal"),
	ReinforcedTitanium   UMETA(DisplayName = "ReinforcedTitanium")
};

UENUM(BlueprintType)
enum class ESBStructuralStabilityState : uint8
{
	Stable               UMETA(DisplayName = "Stable"),
	Stressed             UMETA(DisplayName = "Stressed"),
	Critical             UMETA(DisplayName = "Critical"),
	Collapsing           UMETA(DisplayName = "Collapsing")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBStructuralNodeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
	float StructuralStability = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
	float CurrentLoadWeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
	float MaxLoadCapacity = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
	int32 DistanceFromAnchor = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
	int32 MaxSupportDistance = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
	bool bIsGroundAnchor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
	ESBStructuralStabilityState StabilityState = ESBStructuralStabilityState::Stable;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBStructuralSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
	ESBStructuralMaterialTier MaterialTier = ESBStructuralMaterialTier::Stone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
	float BaseLoadCapacity = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
	int32 MaxHorizontalSpan = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
	float StressThreshold = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
	float CriticalThreshold = 0.90f;
};
