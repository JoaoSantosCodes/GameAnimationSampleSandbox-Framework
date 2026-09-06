#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/SBPortalTypes.h"
#include "SBPortalActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/**
 * Ator de Portal ou Waystone colocado no level
 */
UCLASS(Blueprintable)
class SANDBOXCORE_API ASBPortalActor : public AActor
{
	GENERATED_BODY()

public:
	ASBPortalActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<UStaticMeshComponent> PortalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FSBPortalInfo PortalInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FSBPortalDestination Destination;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	bool bAutoTeleportOnOverlap = true;

	UFUNCTION(BlueprintCallable, Category = "Portal")
	FVector GetTeleportSpawnLocation() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
