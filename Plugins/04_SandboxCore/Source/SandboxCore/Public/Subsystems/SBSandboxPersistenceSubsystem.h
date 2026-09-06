#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/SBPersistenceTypes.h"
#include "SBSandboxPersistenceSubsystem.generated.h"

/**
 * Subsistema mundial C++ para gerenciar o ciclo de vida de persistência (Save/Load) dos Atores.
 */
UCLASS(BlueprintType)
class SANDBOXCORE_API USBSandboxPersistenceSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USBSandboxPersistenceSubsystem();

	// Salva o estado físico de todos os Atores persistentes no World atual para o Payload
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Persistence")
	void SaveWorldState(UObject* SavePayload);

	// Carrega e restaura o estado físico dos Atores persistentes do Payload
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Persistence")
	void LoadWorldState(UObject* SavePayload);

	// Registra a destruição voluntária de um Ator para que não seja restaurado no load
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Persistence")
	void RecordActorDestruction(FGuid ActorGuid);

	UPROPERTY(BlueprintReadOnly, Category = "Sandbox|Persistence")
	TMap<FGuid, FSBSerializedActorData> SerializedActors;
};
