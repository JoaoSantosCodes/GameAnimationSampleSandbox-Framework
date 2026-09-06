#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBFaunaTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBDomesticationComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBDomesticationStateChanged, ESBFaunaDomesticationState, OldState, ESBFaunaDomesticationState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBCreatureTamed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBPregnancyCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBOffspringBirthed, const FSBCreatureGenetics&, OffspringGenetics);

/**
 * Componente de ecologia animal, domesticação, afeto, gestação e reprodução genética
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBDomesticationComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBDomesticationComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	// Setup & Interaction
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Fauna")
	void SetupFauna(FName PreferredFood, const FSBCreatureGenetics& InitialGenetics);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Fauna")
	bool FeedTamingFood(FName FoodID, float FoodNutrition);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Fauna")
	void PetCreature(float AffectionBoost = 0.2f);

	// Breeding & Genetics
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Fauna")
	bool StartBreedingWith(const FSBCreatureGenetics& PartnerGenetics, float GestationTime = 30.0f);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Fauna")
	bool BirthOffspring(FSBCreatureGenetics& OutOffspringGenetics);

	// Simulation Tick
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Fauna")
	void SimulateFaunaTick(float DeltaTime);

	// Queries
	UFUNCTION(BlueprintPure, Category = "Sandbox|Fauna")
	FSBDomesticationData GetDomesticationData() const { return DomesticationData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Fauna")
	bool IsDomesticated() const { return DomesticationData.DomesticationState == ESBFaunaDomesticationState::Domesticated; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Fauna")
	bool IsPregnant() const { return DomesticationData.ReproductiveStage == ESBFaunaReproductiveStage::Pregnant; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Fauna")
	FSBDomesticationStateChanged OnDomesticationStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Fauna")
	FSBCreatureTamed OnCreatureTamed;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Fauna")
	FSBPregnancyCompleted OnPregnancyCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Fauna")
	FSBOffspringBirthed OnOffspringBirthed;

private:
	void SyncTags();

	UPROPERTY()
	FSBDomesticationData DomesticationData;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
