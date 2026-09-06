#include "Widgets/SBStatusHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Interfaces/SBAttributeComponentInterface.h"
#include "GameFramework/Pawn.h"
#include "SBGameplayTags.h"

USBStatusHUDWidget::USBStatusHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USBStatusHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshResourceBars();
}

void USBStatusHUDWidget::RefreshResourceBars()
{
	APawn* OwningPawn = GetOwningPlayerPawn();

	// Repossessao troca o pawn sob o widget; o cache precisa acompanhar em vez de continuar
	// apontando para o componente do pawn anterior.
	if (OwningPawn != CachedPawn.Get())
	{
		CachedPawn = OwningPawn;
		CachedAttributeComponent = nullptr;
	}

	if (!OwningPawn)
	{
		return;
	}

	if (!CachedAttributeComponent.IsValid())
	{
		CachedAttributeComponent = OwningPawn->FindComponentByInterface(USBAttributeComponentInterface::StaticClass());
		if (!CachedAttributeComponent.IsValid())
		{
			return;
		}
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	SetBarPercent(PB_Health, Tags.Attribute_Health);
	SetBarPercent(PB_Mana, Tags.Attribute_Mana);
	SetBarPercent(PB_Stamina, Tags.Attribute_Stamina);
}

void USBStatusHUDWidget::SetBarPercent(UProgressBar* Bar, FGameplayTag AttributeTag) const
{
	if (!Bar || !AttributeTag.IsValid() || !CachedAttributeComponent.IsValid())
	{
		return;
	}

	UActorComponent* AttrComp = CachedAttributeComponent.Get();
	const float MaxValue = ISBAttributeComponentInterface::Execute_GetAttributeMaxValue(AttrComp, AttributeTag);
	if (MaxValue <= 0.0f)
	{
		return;
	}

	const float CurrentValue = ISBAttributeComponentInterface::Execute_GetAttributeValue(AttrComp, AttributeTag);
	Bar->SetPercent(FMath::Clamp(CurrentValue / MaxValue, 0.0f, 1.0f));
}
