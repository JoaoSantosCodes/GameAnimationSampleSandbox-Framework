// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "SBBackgroundSimInterface.generated.h"

USTRUCT(BlueprintType)
struct SANDBOXINTERFACES_API FSBSimulatedEntityData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "BackgroundSim")
	FGuid EntityId;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "BackgroundSim")
	FGameplayTag EntitySimType;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "BackgroundSim")
	TMap<FGameplayTag, float> NumericStates;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "BackgroundSim")
	TMap<FGameplayTag, FString> StringStates;
};

UINTERFACE(MinimalAPI, BlueprintType)
class USBBackgroundSimInterface : public UInterface
{
	GENERATED_BODY()
};

class SANDBOXINTERFACES_API ISBBackgroundSimInterface
{
	GENERATED_BODY()

public:
	// Prepara os dados de simulação antes de o ator ser destruído / descarregado
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Sandbox|BackgroundSim")
	void PrepareForBackgroundSim(UPARAM(ref) FSBSimulatedEntityData& OutData);

	// Restaura o estado a partir dos dados de simulação quando o ator é instanciado / carregado
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Sandbox|BackgroundSim")
	void ResumeFromBackgroundSim(const FSBSimulatedEntityData& InData);
};
