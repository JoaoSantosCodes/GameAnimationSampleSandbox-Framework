#include "DataAssets/SBAssetManager.h"
#include "Utilities/SBLogCategories.h"

USBAssetManager::USBAssetManager()
{
}

USBAssetManager& USBAssetManager::Get()
{
	if (USBAssetManager* Singleton = Cast<USBAssetManager>(GEngine->AssetManager))
	{
		return *Singleton;
	}

	UE_LOG(LogSandbox, Fatal, TEXT("Invalid AssetManager class in DefaultEngine.ini. Must be set to SBAssetManager!"));
	return *CastChecked<USBAssetManager>(GEngine->AssetManager);
}

void USBAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	
	UE_LOG(LogSandbox, Log, TEXT("Sandbox Asset Manager initialized."));
}
