// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Types/SBHerbologyTypes.h"
#include "Types/SBTraumaTypes.h"
#include "Types/SBFaunaTypes.h"
#include "SBCharacterTestTypes.generated.h"

/**
 * Receptor genérico para os delegates dinâmicos dos componentes de personagem.
 *
 * DECLARE_DYNAMIC_MULTICAST_DELEGATE só admite AddDynamic com uma UFUNCTION —
 * AddLambda/AddWeakLambda existem apenas nos delegates não-dinâmicos. Como os
 * delegates do framework precisam permanecer BlueprintAssignable (manifesto,
 * princípio 10), é o teste que fornece o receptor.
 *
 * Use UMA instância por binding: o estado capturado é por objeto, então dois
 * delegates de mesma assinatura não disputam os mesmos campos.
 */
UCLASS()
class USBCharacterTestListener : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void OnVoid() { bFired = true; ++FireCount; }

	UFUNCTION()
	void OnFloat(float Value) { bFired = true; ++FireCount; FloatArg = Value; }

	UFUNCTION()
	void OnBool(bool bValue) { bFired = true; ++FireCount; BoolArg = bValue; }

	UFUNCTION()
	void OnName(FName Value) { bFired = true; ++FireCount; NameArg = Value; NameArgs.Add(Value); }

	UFUNCTION()
	void OnAffliction(ESBAfflictionType Value) { bFired = true; ++FireCount; AfflictionArg = Value; }

	UFUNCTION()
	void OnLimb(ESBBodyLimb Limb) { bFired = true; ++FireCount; LimbArg = Limb; }

	UFUNCTION()
	void OnGenetics(const FSBCreatureGenetics& Value) { bFired = true; ++FireCount; GeneticsArg = Value; }

	UFUNCTION()
	void OnLimbBleed(ESBBodyLimb Limb, ESBBleedType BleedType) { bFired = true; ++FireCount; LimbArg = Limb; BleedArg = BleedType; }

	UFUNCTION()
	void OnLimbBool(ESBBodyLimb Limb, bool bValue) { bFired = true; ++FireCount; LimbArg = Limb; BoolArg = bValue; }

	bool bFired = false;
	int32 FireCount = 0;
	float FloatArg = 0.0f;
	bool BoolArg = false;
	FName NameArg = NAME_None;
	/** Acumula todos os nomes recebidos: um mesmo binding pode disparar para vários. */
	TArray<FName> NameArgs;
	ESBAfflictionType AfflictionArg = ESBAfflictionType();
	ESBBodyLimb LimbArg = ESBBodyLimb();
	ESBBleedType BleedArg = ESBBleedType();
	FSBCreatureGenetics GeneticsArg;
};
