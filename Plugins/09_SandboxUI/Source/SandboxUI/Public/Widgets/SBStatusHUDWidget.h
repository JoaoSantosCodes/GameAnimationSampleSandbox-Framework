#pragma once

#include "CoreMinimal.h"
#include "Widgets/SBUserWidget.h"
#include "SBStatusHUDWidget.generated.h"

class UProgressBar;

UCLASS(Abstract, Blueprintable)
class SANDBOXUI_API USBStatusHUDWidget : public USBUserWidget
{
	GENERATED_BODY()

public:
	USBStatusHUDWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/**
	 * Barras de recurso sao estado continuo, nao evento.
	 *
	 * Este widget era empurrado por `Event.Attribute.Changed`, publicado a cada mudanca de
	 * atributo de *qualquer* pawn do mundo — e o tick de movimento escreve estamina todo frame
	 * durante corrida e regeneracao. O widget entao descartava tudo que nao fosse do proprio
	 * pawn, mas a alocacao do payload ja tinha sido paga pelo produtor. Ler aqui custa tres
	 * consultas por frame e nenhuma alocacao, atualiza na taxa do display, e a pergunta nunca
	 * chega a ser feita sobre outro pawn.
	 *
	 * A resolucao passa por ISBAttributeComponentInterface porque 09_SandboxUI nao depende de
	 * 05_SandboxCharacter, onde o componente concreto vive.
	 */
	UFUNCTION()
	void RefreshResourceBars();

	void SetBarPercent(UProgressBar* Bar, FGameplayTag AttributeTag) const;

	/**
	 * Atualiza por timer, e nao por NativeTick, de proposito.
	 *
	 * `UUserWidget::UpdateCanTick` so tica um widget de Blueprint quando
	 * `WidgetBlueprintGeneratedClass::ClassRequiresNativeTick()` e verdadeiro — e essa flag e
	 * calculada pelo compilador de Blueprint e **gravada no asset**. Um `.uasset` compilado
	 * antes de esta classe passar a implementar tick continuaria com a flag falsa, e o
	 * `NativeTick` nunca rodaria: as barras parariam de atualizar sem erro, log ou aviso, ate
	 * alguem recompilar o Blueprint. Timer nao depende de nada disso.
	 *
	 * 30 Hz e imperceptivel para barra de recurso e custa metade do trabalho de um tick a 60.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Sandbox|UI", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float ResourceRefreshInterval = 1.0f / 30.0f;

	FTimerHandle ResourceRefreshTimer;

	UPROPERTY(Transient)
	TWeakObjectPtr<UActorComponent> CachedAttributeComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<APawn> CachedPawn;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "Sandbox|UI")
	TObjectPtr<UProgressBar> PB_Health;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "Sandbox|UI")
	TObjectPtr<UProgressBar> PB_Mana;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "Sandbox|UI")
	TObjectPtr<UProgressBar> PB_Stamina;
};
