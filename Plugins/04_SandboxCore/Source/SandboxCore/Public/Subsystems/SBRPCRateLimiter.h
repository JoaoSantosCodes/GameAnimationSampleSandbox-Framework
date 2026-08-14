#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "SBRPCRateLimiter.generated.h"

USTRUCT(BlueprintType)
struct SANDBOXCORE_API FSBRPCRateLimiter
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Security")
	float LastCallTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Security")
	int32 CallCount = 0;

	/**
	 * Verifica se a chamada do RPC é permitida na janela de tempo atual.
	 * @param World - Contexto do mundo para ler o tempo de execução.
	 * @param LimitPerSecond - Limite máximo de chamadas permitidas por segundo.
	 * @return true se permitido, false se a taxa limite foi excedida.
	 */
	bool AllowRPC(const UWorld* World, float LimitPerSecond)
	{
		if (!World)
		{
			return false;
		}

		float CurrentTime = World->GetTimeSeconds();

		// Se a janela de tempo de 1 segundo passou, reseta a contagem
		if (CurrentTime - LastCallTime >= 1.0f)
		{
			LastCallTime = CurrentTime;
			CallCount = 0;
		}

		CallCount++;

		// Se a contagem exceder o limite, bloqueia
		return (CallCount <= LimitPerSecond);
	}
};
