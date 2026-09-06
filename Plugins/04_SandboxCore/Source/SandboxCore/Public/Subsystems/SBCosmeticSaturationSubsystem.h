// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SBCosmeticSaturationSubsystem.generated.h"

class USoundBase;

/**
 * Registro de controle de taxa de reprodução de cosméticos.
 */
USTRUCT(BlueprintType)
struct FSBCosmeticLimitRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Sandbox|Cosmetics")
	float LastPlayTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Sandbox|Cosmetics")
	int32 FramePlayCount = 0;
};

/**
 * Subsistema responsável por prevenir a saturação de áudio e efeitos visuais
 * sob latências extremas ou rajadas de pacotes (Packet Burst).
 */
UCLASS()
class SANDBOXCORE_API USBCosmeticSaturationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USBCosmeticSaturationSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Verifica se um som está autorizado a tocar baseado no limite de taxa por localização.
	 * @param Sound - O som a ser reproduzido.
	 * @param Location - Localização da reprodução.
	 * @param MinInterval - Intervalo mínimo em segundos entre sons idênticos na mesma célula do grid.
	 * @return true se puder tocar, false se deve ser suprimido.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Cosmetics")
	bool AllowSound(USoundBase* Sound, const FVector& Location, float MinInterval = 0.05f);

	/**
	 * Verifica se um efeito visual (Niagara/Cascade/etc) está autorizado a tocar baseado no limite de taxa por localização.
	 * @param EffectAsset - O asset do efeito a ser instanciado.
	 * @param Location - Localização da reprodução.
	 * @param MinInterval - Intervalo mínimo em segundos entre efeitos idênticos na mesma célula do grid.
	 * @return true se puder tocar, false se deve ser suprimido.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Cosmetics")
	bool AllowEffect(UObject* EffectAsset, const FVector& Location, float MinInterval = 0.05f);

	/** Limpa periodicamente registros obsoletos da TMap para economizar memória */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Cosmetics")
	void ResetObsoleteRecords();

private:
	UPROPERTY(Transient)
	TMap<FString, FSBCosmeticLimitRecord> SoundRecords;

	UPROPERTY(Transient)
	TMap<FString, FSBCosmeticLimitRecord> EffectRecords;

	FTimerHandle CleanupTimerHandle;

	FString GetSpatialKey(UObject* Asset, const FVector& Location) const;
};
