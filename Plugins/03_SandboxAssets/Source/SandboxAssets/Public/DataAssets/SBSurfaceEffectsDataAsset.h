#pragma once

#include "CoreMinimal.h"
#include "DataAssets/SBPrimaryDataAsset.h"
#include "Chaos/ChaosEngineInterface.h"
#include "SBSurfaceEffectsDataAsset.generated.h"

class USoundBase;

USTRUCT(BlueprintType)
struct FSBSurfaceEffectConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	TObjectPtr<USoundBase> Sound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	TObjectPtr<UObject> VisualEffect = nullptr;
};

UCLASS(BlueprintType)
class SANDBOXASSETS_API USBSurfaceEffectsDataAsset : public USBPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	TMap<TEnumAsByte<EPhysicalSurface>, FSBSurfaceEffectConfig> SurfaceEffectsMap;

	UFUNCTION(BlueprintCallable, Category = "Effects")
	bool GetEffectsForSurface(EPhysicalSurface SurfaceType, USoundBase*& OutSound, UObject*& OutVisualEffect) const;
};
