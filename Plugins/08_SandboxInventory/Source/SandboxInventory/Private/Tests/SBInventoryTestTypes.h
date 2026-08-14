#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/Actor.h"
#include "Interfaces/SBInteractableInterface.h"
#include "Items/SBItemDefinition.h"
#include "Items/SBItemInstance.h"
#include "GameplayTagContainer.h"
#include "SBInventoryTestTypes.generated.h"

UCLASS()
class USBTestInventoryListener : public UObject
{
	GENERATED_BODY()

public:
	int32 SlotUpdateCount = 0;
	int32 EquipCount = 0;
	int32 UnequipCount = 0;

	UPROPERTY()
	TObjectPtr<USBItemInstance> LastUpdatedInstance = nullptr;

	UPROPERTY()
	TObjectPtr<USBItemInstance> LastEquippedInstance = nullptr;

	void OnSlotUpdated(FGameplayTag EventTag, UObject* Payload)
	{
		SlotUpdateCount++;
		if (Payload)
		{
			FObjectProperty* Prop = CastField<FObjectProperty>(Payload->GetClass()->FindPropertyByName(TEXT("ItemInstance")));
			if (Prop)
			{
				LastUpdatedInstance = Cast<USBItemInstance>(Prop->GetPropertyValue_InContainer(Payload));
			}
		}
	}

	void OnEquipped(FGameplayTag EventTag, UObject* Payload)
	{
		EquipCount++;
		if (Payload)
		{
			FObjectProperty* Prop = CastField<FObjectProperty>(Payload->GetClass()->FindPropertyByName(TEXT("ItemInstance")));
			if (Prop)
			{
				LastEquippedInstance = Cast<USBItemInstance>(Prop->GetPropertyValue_InContainer(Payload));
			}
		}
	}

	void OnUnequipped(FGameplayTag EventTag, UObject* Payload)
	{
		UnequipCount++;
	}
};

UCLASS()
class ASBTestLootPickupActor : public AActor, public ISBInteractableInterface
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<AActor> LockedBy = nullptr;

	UPROPERTY()
	TObjectPtr<USBItemDefinition> LootItemDef = nullptr;

	virtual float GetInteractionDuration_Implementation(AActor* Interactor) const override { return 0.0f; }
	
	virtual bool IsInteractionLocked_Implementation(AActor* Interactor) const override
	{
		return LockedBy != nullptr && LockedBy != Interactor;
	}

	virtual void LockInteraction_Implementation(AActor* Interactor) override
	{
		LockedBy = Interactor;
	}

	virtual void UnlockInteraction_Implementation(AActor* Interactor) override
	{
		if (LockedBy == Interactor)
		{
			LockedBy = nullptr;
		}
	}

	virtual void Interact_Implementation(AActor* Interactor) override
	{
		if (Interactor && LootItemDef)
		{
			if (USBInventoryComponent* Inv = Interactor->FindComponentByClass<USBInventoryComponent>())
			{
				Inv->ServerAddItem(LootItemDef, 1);
			}
		}
		Destroy();
	}
};
