#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/SBRegionTypes.h"
#include "SBRegionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBOnActorRegionChanged, AActor*, Actor, const FSBRegionData&, RegionData);

USTRUCT()
struct FSBActorRegionPresence
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FSBRegionData> ActiveRegions;
};

/**
 * Subsistema de gerenciamento de regiões, zonas seguras (Safe Zones), áreas PvP e zonas de risco ambiental
 */
UCLASS(BlueprintType)
class SANDBOXCORE_API USBRegionSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	USBRegionSubsystem();

	// UTickableWorldSubsystem implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return true; }

	// Notificações de entrada e saída
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Region")
	void NotifyActorEnteredRegion(AActor* Actor, const FSBRegionData& RegionData);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Region")
	void NotifyActorExitedRegion(AActor* Actor, const FSBRegionData& RegionData);

	// Consultas de estado
	UFUNCTION(BlueprintPure, Category = "Sandbox|Region")
	bool GetActorCurrentRegion(const AActor* Actor, FSBRegionData& OutRegionData) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Region")
	bool IsActorInSafeZone(const AActor* Actor) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Region")
	bool IsPvPAllowedForActor(const AActor* Actor) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Region")
	bool IsActorInHazard(const AActor* Actor) const;

	// Delegates de evento
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Region")
	FSBOnActorRegionChanged OnActorEnteredRegion;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Region")
	FSBOnActorRegionChanged OnActorExitedRegion;

private:
	void ApplyRegionTagsToActor(AActor* Actor, const FSBRegionData& RegionData);
	void RemoveRegionTagsFromActor(AActor* Actor, const FSBRegionData& RegionData);

	UPROPERTY()
	TMap<TWeakObjectPtr<AActor>, FSBActorRegionPresence> ActorPresenceMap;

	float HazardTickTimer = 0.0f;
};
