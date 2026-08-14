#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SBCameraModeDefinition.generated.h"

class USBCameraMode;

UCLASS(BlueprintType)
class SANDBOXCHARACTER_API USBCameraModeDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	// Classe operacional deste modo de câmera (se nulo, usa a classe base USBCameraMode)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	TSubclassOf<USBCameraMode> CameraModeClass;

	// Tag de estado que ativa este modo de câmera (ex: State.Character.Aiming)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	FGameplayTag ActivationTag;

	// Prioridade na pilha (modos de maior prioridade ditam o enquadramento final)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	int32 Priority = 0;

	// Field of View alvo (ex: 90.0 para locomoção, 65.0 para mira)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float TargetFOV = 90.0f;

	// Comprimento do Spring Arm alvo (ex: 300.0)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float TargetArmLength = 300.0f;

	// Offset relativo da câmera (ex: X=0, Y=50, Z=20 para shoulder offset)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	FVector TargetSocketOffset = FVector::ZeroVector;

	// Velocidade de interpolação nas transições (ex: 5.0f)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float BlendSpeed = 5.0f;
};
