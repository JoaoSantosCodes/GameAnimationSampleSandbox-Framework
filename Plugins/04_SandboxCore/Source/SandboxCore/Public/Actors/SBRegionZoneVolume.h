#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/SBRegionTypes.h"
#include "SBRegionZoneVolume.generated.h"

class UBoxComponent;

/**
 * Volume de Trigger colocado no level para definir os limites de uma Região ou Zona de Perigo
 */
UCLASS(Blueprintable)
class SANDBOXCORE_API ASBRegionZoneVolume : public AActor
{
	GENERATED_BODY()

public:
	ASBRegionZoneVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Region")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	FSBRegionData RegionData;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
