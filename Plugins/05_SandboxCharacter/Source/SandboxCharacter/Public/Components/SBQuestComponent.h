// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/GameFrameworkComponent.h"
#include "GameplayTagContainer.h"
#include "Interfaces/SBComponentInterface.h"
#include "Interfaces/SBSaveInterface.h"
#include "DataAssets/SBQuestDataAsset.h"
#include "SBQuestComponent.generated.h"

USTRUCT(BlueprintType)
struct FSBQuestObjectiveProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTag ObjectiveTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 CurrentCount = 0;
};

USTRUCT(BlueprintType)
struct FSBActiveQuest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TObjectPtr<USBQuestDataAsset> QuestData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FSBQuestObjectiveProgress> ObjectivesProgress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bIsCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bRewardsClaimed = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBQuestCompletedSignature, USBQuestDataAsset*, QuestData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBQuestObjectiveProgressedSignature, USBQuestDataAsset*, QuestData, FGameplayTag, ObjectiveTag);

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBQuestComponent : public UGameFrameworkComponent, public ISBComponentInterface, public ISBSaveInterface
{
	GENERATED_BODY()

public:
	USBQuestComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(ReplicatedUsing = OnRep_ActiveQuests, BlueprintReadOnly, Category = "Quest")
	TArray<FSBActiveQuest> ActiveQuests;

	UFUNCTION()
	void OnRep_ActiveQuests();

	// Event Bus Listeners
	FDelegateHandle ItemAddedHandle;
	FDelegateHandle FootstepHandle;

	void HandleItemAdded(FGameplayTag EventTag, UObject* Payload);
	void HandleFootstep(FGameplayTag EventTag, UObject* Payload);

public:
	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override {}
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	// ISBSaveInterface
	virtual bool SaveComponentData_Implementation(UObject* SavePayload) override;
	virtual bool LoadComponentData_Implementation(UObject* SavePayload) override;
	virtual int32 GetSavePriority_Implementation() const override { return 90; }

	// Core API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Quests")
	void AcceptQuest(USBQuestDataAsset* Quest);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Quests")
	void ProgressObjective(FGameplayTag ObjectiveTag, int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Quests")
	bool IsQuestActive(USBQuestDataAsset* Quest) const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Quests")
	bool IsQuestCompleted(USBQuestDataAsset* Quest) const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Quests")
	bool ClaimQuestRewards(USBQuestDataAsset* Quest);

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Quests")
	FSBQuestCompletedSignature OnQuestCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Quests")
	FSBQuestObjectiveProgressedSignature OnQuestObjectiveProgressed;
};
