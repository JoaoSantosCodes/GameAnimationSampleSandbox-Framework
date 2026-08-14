#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ISBAnimLayerInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class USBAnimLayerInterface : public UInterface
{
	GENERATED_BODY()
};

class SANDBOXCHARACTER_API ISBAnimLayerInterface
{
	GENERATED_BODY()

public:
	// Contrato base para Linked Anim Graphs no Sandbox Framework.
	// As funções de override reais serão definidas nos grafos de animação (AnimBP)
	// que implementam esta interface para substituição de pose.
};
