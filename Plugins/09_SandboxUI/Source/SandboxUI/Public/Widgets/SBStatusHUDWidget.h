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
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

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
	void RefreshResourceBars();

	void SetBarPercent(UProgressBar* Bar, FGameplayTag AttributeTag) const;

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
