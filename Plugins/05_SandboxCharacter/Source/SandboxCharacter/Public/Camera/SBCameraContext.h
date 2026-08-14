#pragma once

#include "CoreMinimal.h"
#include "SBCameraContext.generated.h"

class ACharacter;
class USpringArmComponent;
class UCameraComponent;

USTRUCT(BlueprintType)
struct FSBCameraContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	TObjectPtr<ACharacter> Character = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	float DeltaTime = 0.0f;
};
