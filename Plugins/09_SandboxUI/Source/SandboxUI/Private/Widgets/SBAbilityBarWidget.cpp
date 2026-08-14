#include "Widgets/SBAbilityBarWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Subsystems/SBEventPayloads.h"
#include "Utilities/SBLogCategories.h"

USBAbilityBarWidget::USBAbilityBarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bHasScriptImplementedTick = false; // Permite NativeTick no C++ sem overhead do Blueprint tick
}

void USBAbilityBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IMG_CooldownMask)
	{
		IMG_CooldownMask->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (TXT_CooldownTime)
	{
		TXT_CooldownTime->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (!WatchedAbilityTag.IsValid())
	{
		UE_LOG(LogSandboxUI, Warning, TEXT("USBAbilityBarWidget: WatchedAbilityTag is invalid! Cooldown tracking will not function for this slot. Please configure it in the editor."));
	}

	FSBBlueprintEventDelegate StartDelegate;
	StartDelegate.BindDynamic(this, &USBAbilityBarWidget::OnCooldownStarted);
	SubscribeToEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Ability.CooldownStarted")), StartDelegate);

	FSBBlueprintEventDelegate EndDelegate;
	EndDelegate.BindDynamic(this, &USBAbilityBarWidget::OnCooldownEnded);
	SubscribeToEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Ability.CooldownEnded")), EndDelegate);
}

void USBAbilityBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsCooldownActive)
	{
		CooldownRemaining -= InDeltaTime;
		if (CooldownRemaining <= 0.0f)
		{
			CooldownRemaining = 0.0f;
			bIsCooldownActive = false;

			if (IMG_CooldownMask)
			{
				IMG_CooldownMask->SetVisibility(ESlateVisibility::Collapsed);
			}
			if (TXT_CooldownTime)
			{
				TXT_CooldownTime->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
		else
		{
			if (TXT_CooldownTime)
			{
				TXT_CooldownTime->SetText(FText::AsNumber(FMath::CeilToInt(CooldownRemaining)));
			}
		}
	}
}

void USBAbilityBarWidget::OnCooldownStarted(FGameplayTag EventTag, UObject* Payload)
{
	if (!Payload) return;

	USBCooldownEventPayload* CoolPayload = Cast<USBCooldownEventPayload>(Payload);
	if (!CoolPayload) return;

	if (CoolPayload->TargetPawn != GetOwningPlayerPawn()) return;

	// Filtra pela habilidade observada por esta instância de slot (fail-closed por segurança)
	if (!WatchedAbilityTag.IsValid() || CoolPayload->AbilityTag != WatchedAbilityTag) return;

	CooldownDuration = CoolPayload->Duration;
	CooldownRemaining = CoolPayload->Duration;
	bIsCooldownActive = CooldownDuration > 0.0f;

	if (bIsCooldownActive)
	{
		if (IMG_CooldownMask)
		{
			IMG_CooldownMask->SetVisibility(ESlateVisibility::Visible);
		}
		if (TXT_CooldownTime)
		{
			TXT_CooldownTime->SetVisibility(ESlateVisibility::Visible);
			TXT_CooldownTime->SetText(FText::AsNumber(FMath::CeilToInt(CooldownRemaining)));
		}
	}
}

void USBAbilityBarWidget::OnCooldownEnded(FGameplayTag EventTag, UObject* Payload)
{
	if (!Payload) return;

	USBCooldownEventPayload* CoolPayload = Cast<USBCooldownEventPayload>(Payload);
	if (!CoolPayload) return;

	if (CoolPayload->TargetPawn != GetOwningPlayerPawn()) return;

	// Filtra pela habilidade observada por esta instância de slot (fail-closed por segurança)
	if (!WatchedAbilityTag.IsValid() || CoolPayload->AbilityTag != WatchedAbilityTag) return;

	bIsCooldownActive = false;
	CooldownRemaining = 0.0f;

	if (IMG_CooldownMask)
	{
		IMG_CooldownMask->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (TXT_CooldownTime)
	{
		TXT_CooldownTime->SetVisibility(ESlateVisibility::Collapsed);
	}
}
