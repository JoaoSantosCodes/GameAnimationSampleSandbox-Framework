#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Animation/ISBAnimLayerInterface.h"
#include "SBAnimLayerConfigDataAsset.generated.h"

class UAnimInstance;

USTRUCT(BlueprintType)
struct FSBAnimLayerMappingEntry
{
	GENERATED_BODY()

	// Tag de estado associada (ex: State.Character.Crouching)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	FGameplayTag StateTag;

	// Sub-ABP correspondente implementing ISBAnimLayerInterface
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> AnimLayerClass;

	// Prioridade de sobreposição (Altas prioridades vinculam por último e vencem colisões)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	int32 Priority = 0;
};

UCLASS(BlueprintType)
class SANDBOXCHARACTER_API USBAnimLayerConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Mapeamentos de tags de estado para layers de animação modular
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TArray<FSBAnimLayerMappingEntry> LayerMappings;

	// Helper para encontrar a classe e a prioridade da layer mapeada para uma tag
	bool FindMappingForTag(const FGameplayTag& Tag, TSubclassOf<UAnimInstance>& OutLayerClass, int32& OutPriority) const
	{
		for (const FSBAnimLayerMappingEntry& Entry : LayerMappings)
		{
			if (Entry.StateTag == Tag)
			{
				OutLayerClass = Entry.AnimLayerClass;
				OutPriority = Entry.Priority;
				return true;
			}
		}
		return false;
	}
};
