// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBPersistenceTypes.h"
#include "SBPersistenceComponent.generated.h"

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCORE_API USBPersistenceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USBPersistenceComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Persistence")
	FSBEntityId PersistentId;

#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditImport() override;
	void AutoGenerateGuid();
#endif
};
