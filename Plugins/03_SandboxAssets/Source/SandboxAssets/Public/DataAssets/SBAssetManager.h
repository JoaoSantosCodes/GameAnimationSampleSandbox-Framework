#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "SBAssetManager.generated.h"

UCLASS()
class SANDBOXASSETS_API USBAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	USBAssetManager();

	static USBAssetManager& Get();

	virtual void StartInitialLoading() override;
};
