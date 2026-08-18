#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SBInteractionConfigDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FSBInteractionToleranceConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float RangeTolerance = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float ServerValidationTolerance = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float ServerExecutionTolerance = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float CompletionTolerance = 0.1f;
};

USTRUCT(BlueprintType)
struct FSBInteractionThrottleConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float ProgressBroadcastInterval = 0.01667f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float RPCRateLimit = 10.0f;
};

UCLASS(BlueprintType)
class SANDBOXINTERACTION_API USBInteractionConfigDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float InteractionRange = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	bool bUseLineTrace = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float TraceRadius = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	FSBInteractionToleranceConfig ToleranceConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	FSBInteractionThrottleConfig ThrottleConfig;
};
