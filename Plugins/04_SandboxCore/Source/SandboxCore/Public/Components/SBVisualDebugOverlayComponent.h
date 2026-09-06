#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/SBComponentInterface.h"
#include "Types/SBVisualDebuggerTypes.h"
#include "SBVisualDebugOverlayComponent.generated.h"

/**
 * Componente que expõe dados visuais do ator ao subsistema de depuração do Viewport
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SANDBOXCORE_API USBVisualDebugOverlayComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBVisualDebugOverlayComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override;
	virtual void OnShutdown_Implementation() override;

	/** Categoria de overlay do componente */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|VisualDebugger")
	ESBOverlayCategory ComponentCategory = ESBOverlayCategory::PowerGrid;

	/** Enfileira uma linha de depuração até uma localização alvo */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|VisualDebugger")
	void PushDebugLine(const FVector& TargetLocation, const FColor& Color, const FString& Label);

	/** Handler chamado ao alternar o estado de uma categoria de overlay */
	UFUNCTION()
	void HandleCategoryToggled(ESBOverlayCategory Category, bool bEnabled);

	/** Retorna o total de linhas enfileiradas localmente */
	UFUNCTION(BlueprintPure, Category = "Sandbox|VisualDebugger")
	int32 GetPushedLinesCount() const { return PushedLinesCount; }

private:
	int32 PushedLinesCount = 0;
	void SyncTags();
};
