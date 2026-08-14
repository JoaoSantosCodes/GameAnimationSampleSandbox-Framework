#include "HUD/SBHUD.h"
#include "Subsystems/SBUIManager.h"
#include "Interfaces/SBCharacterInterface.h"
#include "DataAssets/SBPawnDataAsset.h"
#include "GameFramework/Character.h"
#include "Utilities/SBLogCategories.h"

ASBHUD::ASBHUD()
{
}

void ASBHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetOwningPlayerController();
	if (PC && PC->IsLocalController())
	{
		ULocalPlayer* LP = PC->GetLocalPlayer();
		if (!LP) return;

		USBUIManager* UIManager = LP->GetSubsystem<USBUIManager>();
		if (!UIManager) return;

		TSubclassOf<USBUserWidget> HUDWidgetClass = nullptr;

		APawn* Pawn = PC->GetPawn();
		if (Pawn && Pawn->GetClass()->ImplementsInterface(USBCharacterInterface::StaticClass()))
		{
			UObject* RawPawnData = ISBCharacterInterface::Execute_GetPawnData(Pawn);
			if (USBPawnDataAsset* PawnData = Cast<USBPawnDataAsset>(RawPawnData))
			{
				if (PawnData->HUDLayoutClass)
				{
					HUDWidgetClass = Cast<UClass>(PawnData->HUDLayoutClass);
				}
			}
		}

		if (!HUDWidgetClass)
		{
			HUDWidgetClass = MainHUDWidgetClass;
		}

		if (HUDWidgetClass)
		{
			UIManager->PushWidget(HUDWidgetClass, ESBUILayer::GameHUD);
			UE_LOG(LogSandboxUI, Log, TEXT("Successfully spawned HUD layout widget"));
		}
	}
}
