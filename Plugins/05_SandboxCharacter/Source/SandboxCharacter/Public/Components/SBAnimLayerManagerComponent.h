#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "GameplayTagContainer.h"
#include "Interfaces/SBComponentInterface.h"
#include "Animation/AnimInstance.h"
#include "Animation/ISBAnimLayerInterface.h"
#include "SBAnimLayerManagerComponent.generated.h"

class USBAnimLayerConfigDataAsset;
class USBStateComponent;
class UAnimInstance;

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBAnimLayerManagerComponent : public UGameFrameworkComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBAnimLayerManagerComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override;
	virtual void OnShutdown_Implementation() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Mapeamentos de tag para layers de animação configuradas
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox|Animation")
	TObjectPtr<USBAnimLayerConfigDataAsset> AnimLayerConfigAsset = nullptr;

	// Cache do componente de estado para ouvir atualizações de tags
	UPROPERTY(Transient)
	TObjectPtr<USBStateComponent> CachedStateComponent = nullptr;

	// Classes de animação vinculadas atualmente (controle contra hitches visuais)
	UPROPERTY(Transient)
	TArray<TSubclassOf<UAnimInstance>> CurrentLinkedClasses;

	// Sinaliza que uma ou mais tags mudaram neste frame e o rebuild deve ser executado no final do tick
	bool bRebuildPending = false;

	// Sinaliza que a AnimInstance estava nula durante uma tentativa de vinculação inicial
	bool bPendingInitialLink = false;

	// Método invocado ao ouvir alteração de tags do StateComponent
	UFUNCTION()
	void OnStateTagChanged(FGameplayTag Tag, bool bAdded);

	// Reconstrói a vinculação de sub-layers na malha de forma atômica e otimizada
	void RebuildLinkedLayers();

	// Retorna a AnimInstance ativa da malha principal do Character
	virtual UAnimInstance* GetActiveAnimInstance() const;
};

UCLASS()
class SANDBOXCHARACTER_API USBMockAnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()
};

UCLASS()
class SANDBOXCHARACTER_API USBMockCrouchLayer : public UAnimInstance, public ISBAnimLayerInterface
{
	GENERATED_BODY()
};

UCLASS()
class SANDBOXCHARACTER_API USBMockAimLayer : public UAnimInstance, public ISBAnimLayerInterface
{
	GENERATED_BODY()
};

UCLASS()
class SANDBOXCHARACTER_API USBTestAnimLayerManagerComponent : public USBAnimLayerManagerComponent
{
	GENERATED_BODY()

public:
	const TArray<TSubclassOf<UAnimInstance>>& GetCurrentLinkedClasses() const { return CurrentLinkedClasses; }
	bool GetRebuildPending() const { return bRebuildPending; }
	void SetConfigAsset(USBAnimLayerConfigDataAsset* Asset) { AnimLayerConfigAsset = Asset; }
	void TriggerDirectRebuild() { RebuildLinkedLayers(); }

	UPROPERTY(Transient)
	TObjectPtr<UAnimInstance> MockAnimInstance = nullptr;

	virtual UAnimInstance* GetActiveAnimInstance() const override
	{
		return MockAnimInstance ? MockAnimInstance.Get() : Super::GetActiveAnimInstance();
	}
};
