#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Widgets/SBUserWidget.h"
#include "SBHUD.generated.h"

UCLASS()
class SANDBOXUI_API ASBHUD : public AHUD
{
	GENERATED_BODY()

public:
	ASBHUD();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox|UI")
	TSubclassOf<USBUserWidget> MainHUDWidgetClass = nullptr;
};
