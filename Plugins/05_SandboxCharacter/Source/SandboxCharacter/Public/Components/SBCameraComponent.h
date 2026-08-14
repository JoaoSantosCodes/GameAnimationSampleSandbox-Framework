#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "GameplayTagContainer.h"
#include "Types/SBCommonTypes.h"
#include "Camera/Modes/SBCameraMode.h"
#include "Camera/DataAssets/SBCameraModeDefinition.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBCameraComponent.generated.h"

class USBCameraMode;
class USBCameraModeDefinition;
class USBStateComponent;
class USpringArmComponent;
class UCameraComponent;

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBCameraComponent : public UGameFrameworkComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBCameraComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override;
	virtual void OnShutdown_Implementation() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Modos de câmera configurados para o personagem
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox|Camera")
	TArray<TObjectPtr<USBCameraModeDefinition>> CameraModeConfigs;

	// Instâncias pré-alocadas para reutilização (evita instanciamento constante)
	UPROPERTY(Transient)
	TArray<TObjectPtr<USBCameraMode>> AvailableCameraModes;

	// Pilha ativa de modos (ordenada por prioridade decrescente - maior prioridade no índice 0)
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Sandbox|Camera")
	TArray<TObjectPtr<USBCameraMode>> ActiveCameraModes;

	// Componentes cacheados
	UPROPERTY(Transient)
	TObjectPtr<USBStateComponent> CachedStateComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USpringArmComponent> CachedSpringArmComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> CachedCameraComponent = nullptr;

	// Sinaliza que a pilha mudou neste tick e deve ser reconstruída/reordenada
	bool bStackChangePending = false;

	// Ouve as atualizações de tags do StateComponent
	UFUNCTION()
	void OnStateTagChanged(FGameplayTag Tag, bool bAdded);

	// Reconstrói e ordena a pilha de modos ativos com base nas tags ativas
	void RebuildCameraStack();

	// Encontra ou instancia o modo de câmera a partir da sua definição
	USBCameraMode* GetOrCreateCameraModeInstance(USBCameraModeDefinition* Def);
};

UCLASS()
class SANDBOXCHARACTER_API USBMockCameraMode : public USBCameraMode
{
	GENERATED_BODY()

public:
	int32 UpdateCount = 0;

	virtual void Update_Implementation(float DeltaTime, const FSBCameraContext& Context) override;
};

UCLASS()
class SANDBOXCHARACTER_API USBTestCameraComponent : public USBCameraComponent
{
	GENERATED_BODY()

public:
	const TArray<TObjectPtr<USBCameraMode>>& GetActiveCameraModes() const { return ActiveCameraModes; }
	bool GetStackChangePending() const { return bStackChangePending; }
	void SetCameraModeConfigs(const TArray<TObjectPtr<USBCameraModeDefinition>>& Configs) { CameraModeConfigs = Configs; }
	void TriggerDirectRebuild() { RebuildCameraStack(); }
};
