#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBConveyorTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBConveyorNetworkComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBConveyorStateChanged, ESBConveyorState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBConveyorItemTransferred, FName, ItemId, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBConveyorJammed, bool, bIsJammed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBConveyorItemFiltered, FName, ItemId, FGameplayTag, MatchedTag);

/**
 * Componente de esteiras rolantes, divisores, confluências, separadores inteligentes e logística de fábrica
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXINVENTORY_API USBConveyorNetworkComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBConveyorNetworkComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Conveyor Logistics API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Logistics")
	void SetupNode(ESBConveyorNodeType InNodeType, float InBeltSpeed, int32 InCapacity);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Logistics")
	void SetFilterTag(FGameplayTag InTag);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Logistics")
	bool ConnectOutput(USBConveyorNetworkComponent* TargetNode);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Logistics")
	bool ConnectInput(USBConveyorNetworkComponent* SourceNode);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Logistics")
	void DisconnectOutput(USBConveyorNetworkComponent* TargetNode);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Logistics")
	void DisconnectInput(USBConveyorNetworkComponent* SourceNode);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Logistics")
	bool EnqueueItem(FName InItemId, int32 InQuantity, FGameplayTag InTag);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Logistics")
	bool DequeueItem(FSBConveyorItemSlot& OutItem);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Building|Logistics")
	bool PeekNextItem(FSBConveyorItemSlot& OutItem) const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Logistics")
	void SimulateConveyorTick(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Building|Logistics")
	FSBConveyorNodeData GetNodeData() const { return NodeData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Building|Logistics")
	ESBConveyorState GetConveyorState() const { return NodeData.ConveyorState; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Building|Logistics")
	bool IsJammed() const { return NodeData.bIsJammed; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Building|Logistics")
	TArray<FSBConveyorItemSlot> GetItemQueue() const { return ItemQueue; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Building|Logistics")
	FSBConveyorSettings Settings;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Building|Logistics")
	FSBConveyorStateChanged OnConveyorStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Building|Logistics")
	FSBConveyorItemTransferred OnConveyorItemTransferred;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Building|Logistics")
	FSBConveyorJammed OnConveyorJammed;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Building|Logistics")
	FSBConveyorItemFiltered OnConveyorItemFiltered;

private:
	void SyncConveyorState();
	void SyncTags();

	UPROPERTY()
	FSBConveyorNodeData NodeData;

	UPROPERTY()
	TArray<FSBConveyorItemSlot> ItemQueue;

	UPROPERTY()
	TArray<TWeakObjectPtr<USBConveyorNetworkComponent>> InputConnections;

	UPROPERTY()
	TArray<TWeakObjectPtr<USBConveyorNetworkComponent>> OutputConnections;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
