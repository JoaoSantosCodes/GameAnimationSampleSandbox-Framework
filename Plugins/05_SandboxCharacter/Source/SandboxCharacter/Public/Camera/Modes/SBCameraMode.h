#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Camera/SBCameraContext.h"
#include "SBCameraMode.generated.h"

class USBCameraModeDefinition;
class USBCameraComponent;

UCLASS(Blueprintable, BlueprintType)
class SANDBOXCHARACTER_API USBCameraMode : public UObject
{
	GENERATED_BODY()

public:
	USBCameraMode();

	// Inicializa a instância do modo de câmera
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Camera")
	void Initialize(USBCameraComponent* InComponent, USBCameraModeDefinition* InDefinition);

	// Verifica se este modo pode ser ativado no frame
	UFUNCTION(BlueprintNativeEvent, Category = "Sandbox|Camera")
	bool CanEnter(const FSBCameraContext& Context) const;
	virtual bool CanEnter_Implementation(const FSBCameraContext& Context) const;

	// Invocado ao entrar no modo de câmera
	UFUNCTION(BlueprintNativeEvent, Category = "Sandbox|Camera")
	void Enter(const FSBCameraContext& Context);
	virtual void Enter_Implementation(const FSBCameraContext& Context) {}

	// Invocado a cada frame se o modo estiver ativo na pilha
	UFUNCTION(BlueprintNativeEvent, Category = "Sandbox|Camera")
	void Update(float DeltaTime, const FSBCameraContext& Context);
	virtual void Update_Implementation(float DeltaTime, const FSBCameraContext& Context) {}

	// Invocado ao sair do modo de câmera
	UFUNCTION(BlueprintNativeEvent, Category = "Sandbox|Camera")
	void Exit(const FSBCameraContext& Context);
	virtual void Exit_Implementation(const FSBCameraContext& Context) {}

	// Retorna a prioridade definida no Data Asset
	UFUNCTION(BlueprintPure, Category = "Sandbox|Camera")
	int32 GetPriority() const;

	// Retorna a definição associada
	UFUNCTION(BlueprintPure, Category = "Sandbox|Camera")
	USBCameraModeDefinition* GetDefinition() const { return Definition; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USBCameraComponent> CameraComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USBCameraModeDefinition> Definition = nullptr;
};
