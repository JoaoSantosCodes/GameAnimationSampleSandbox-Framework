#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/SBVisualDebuggerTypes.h"
#include "SBVisualDebuggerSubsystem.generated.h"

/**
 * Subsistema central de gerenciamento e controle de overlays de depuração visual no Viewport
 */
UCLASS()
class SANDBOXCORE_API USBVisualDebuggerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USBVisualDebuggerSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Ativa ou desativa uma categoria de overlay */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|VisualDebugger")
	void SetCategoryEnabled(ESBOverlayCategory Category, bool bEnabled);

	/** Verifica se uma categoria está ativa */
	UFUNCTION(BlueprintPure, Category = "Sandbox|VisualDebugger")
	bool IsCategoryEnabled(ESBOverlayCategory Category) const;

	/** Enfileira um item para renderização no frame atual */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|VisualDebugger")
	void QueueRenderItem(const FSBOverlayRenderItem& Item);

	/** Retorna os itens de renderização enfileirados para uma determinada categoria */
	UFUNCTION(BlueprintPure, Category = "Sandbox|VisualDebugger")
	TArray<FSBOverlayRenderItem> GetQueuedRenderItems(ESBOverlayCategory Category) const;

	/** Limpa a fila de itens de renderização */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|VisualDebugger")
	void ClearRenderItems();

	/** Retorna as métricas operacionais */
	UFUNCTION(BlueprintPure, Category = "Sandbox|VisualDebugger")
	FSBVisualDebuggerMetrics GetMetrics() const;

	/** Reinicializa o subsistema */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|VisualDebugger")
	void ResetSubsystem();

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|VisualDebugger")
	FSBOnOverlayCategoryToggled OnOverlayCategoryToggled;

private:
	uint32 EnabledCategoriesBitmask = 0;
	TArray<FSBOverlayRenderItem> QueuedRenderItems;
};
