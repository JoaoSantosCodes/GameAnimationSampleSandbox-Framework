#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SBAmbientZoneTrigger.generated.h"

class UBoxComponent;
class UAudioComponent;
class USoundBase;

UCLASS(BlueprintType)
class SANDBOXCORE_API USBAreaDiscoveryPayload : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Area")
	FText AreaName;

	UPROPERTY(BlueprintReadOnly, Category = "Area")
	FText AreaDescription;

	UPROPERTY(BlueprintReadOnly, Category = "Area")
	TSoftObjectPtr<UObject> PresentationSequence;

	UPROPERTY(BlueprintReadOnly, Category = "Area")
	bool bIsFirstDiscovery = false;
};

UCLASS(BlueprintType)
class SANDBOXCORE_API ASBAmbientZoneTrigger : public AActor
{
	GENERATED_BODY()

public:
	ASBAmbientZoneTrigger();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> ActiveAudioComponent;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Ambient|Area")
	bool bHasBeenDiscovered = false;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient")
	TObjectPtr<USoundBase> AmbientSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient")
	float FadeInDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient")
	float FadeOutDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient")
	float VolumeMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient|Area")
	bool bEnableAreaDiscovery = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient|Area", meta = (EditCondition = "bEnableAreaDiscovery"))
	FText AreaName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient|Area", meta = (EditCondition = "bEnableAreaDiscovery"))
	FText AreaDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient|Area", meta = (EditCondition = "bEnableAreaDiscovery"))
	TSoftObjectPtr<UObject> PresentationSequence;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient|Area", meta = (EditCondition = "bEnableAreaDiscovery"))
	bool bTriggerSequenceOnlyOnce = true;
};
