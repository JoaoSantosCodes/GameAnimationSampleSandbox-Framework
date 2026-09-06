#pragma once

#include "CoreMinimal.h"
#include "SBCoverTypes.generated.h"

UENUM(BlueprintType)
enum class ESBCoverType : uint8
{
	None UMETA(DisplayName = "None"),
	LowCover UMETA(DisplayName = "LowCover"),
	HighCover UMETA(DisplayName = "HighCover")
};

UENUM(BlueprintType)
enum class ESBCoverEdge : uint8
{
	None UMETA(DisplayName = "None"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right"),
	Top UMETA(DisplayName = "Top")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBCoverPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
	FVector Normal = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
	ESBCoverType CoverType = ESBCoverType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
	bool bHasLeftEdge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
	bool bHasRightEdge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
	bool bHasTopEdge = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBCoverSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
	float TraceDistance = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
	float LowCoverHeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
	float HighCoverHeight = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover")
	float EdgeCheckDistance = 60.0f;
};
