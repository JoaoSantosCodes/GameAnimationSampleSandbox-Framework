#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Types/SBCommonTypes.h"
#include "SBStatusEffectDefinition.generated.h"

USTRUCT(BlueprintType)
struct FSBStatusEffectModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	FGameplayTag AttributeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	FSBAttributeModifier Modifier;
};

/**
 * Definição estática de um Status Effect (Buff/Debuff/DOT/HOT).
 */
UCLASS(BlueprintType, Const)
class SANDBOXCHARACTER_API USBStatusEffectDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StatusEffect")
	FGameplayTag EffectTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StatusEffect")
	float DefaultDuration = 0.0f; // <= 0 significa infinito/permanente

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StatusEffect")
	float DefaultPeriod = 0.0f; // <= 0 significa sem ticks periódicos (apenas passivo)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StatusEffect")
	FGameplayTagContainer GrantedTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StatusEffect")
	TArray<FSBStatusEffectModifier> AttributeModifiers;

	// Configuração de Ticks Periódicos (ex: -5 de Vida a cada tick de veneno)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StatusEffect")
	FGameplayTag PeriodAttributeTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StatusEffect")
	float PeriodAttributeChange = 0.0f;
};
