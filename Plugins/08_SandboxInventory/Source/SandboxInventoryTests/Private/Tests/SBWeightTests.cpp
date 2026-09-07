// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SBInventoryComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBMovementComponent.h"
#include "Movement/Behaviors/SBMovementBehaviorSprint.h"
#include "Movement/DataAssets/SBMovementBehaviorDefinition.h"
#include "Items/SBItemDefinition.h"
#include "Items/SBItemFragment_Weight.h"
#include "Interfaces/SBItemDurabilityInterface.h"
#include "GameplayTagsManager.h"
#include "SBGameplayTags.h"
#include "Character/SBCharacter.h"

BEGIN_DEFINE_SPEC(FSBWeightTestsSpec, "Sandbox.Inventory.Weight", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TestCharacter;
	USBInventoryComponent* InventoryComp;
	USBStateComponent* StateComp;
	USBAttributeComponent* AttributeComp;
	USBMovementComponent* MovementComp;
	USBItemDefinition* HeavyItemDef;
	USBItemFragment_Weight* WeightFragment;
END_DEFINE_SPEC(FSBWeightTestsSpec)

void FSBWeightTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		UGameInstance* GI = nullptr;
		if (GEngine)
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if (Context.OwningGameInstance)
				{
					GI = Context.OwningGameInstance;
					break;
				}
			}
		}
		if (GI)
		{
			TestWorld->SetGameInstance(GI);
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("WeightTestPlayer");
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		// Cria e registra componentes
		AttributeComp = NewObject<USBAttributeComponent>(TestCharacter, TEXT("AttributeComponent"));
		AttributeComp->RegisterComponent();

		StateComp = NewObject<USBStateComponent>(TestCharacter, TEXT("StateComponent"));
		StateComp->RegisterComponent();

		InventoryComp = NewObject<USBInventoryComponent>(TestCharacter, TEXT("InventoryComponent"));
		InventoryComp->RegisterComponent();

		MovementComp = NewObject<USBMovementComponent>(TestCharacter, TEXT("MovementComponent"));
		MovementComp->RegisterComponent();

		// Inicializa componentes
		ISBComponentInterface::Execute_OnInitialize(AttributeComp);
		ISBComponentInterface::Execute_OnReady(AttributeComp);
		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnReady(StateComp);
		ISBComponentInterface::Execute_OnInitialize(InventoryComp);
		ISBComponentInterface::Execute_OnPostInitialize(InventoryComp);
		ISBComponentInterface::Execute_OnReady(InventoryComp);
		ISBComponentInterface::Execute_OnInitialize(MovementComp);
		ISBComponentInterface::Execute_OnReady(MovementComp);

		// Inicializa tag speed
		FGameplayTag SpeedTag = FSBGameplayTags::Get().Attribute_Speed;
		FSBAttribute SpeedAttr;
		SpeedAttr.BaseValue = 600.0f;
		SpeedAttr.CurrentValue = 600.0f;
		SpeedAttr.MaxValue = 1000.0f;
		SpeedAttr.MinValue = 0.0f;
		AttributeComp->RegisterAttribute(SpeedTag, SpeedAttr);

		// Cria item pesado
		HeavyItemDef = NewObject<USBItemDefinition>(TestWorld, TEXT("HeavyItemDef"));
		const_cast<FText&>(HeavyItemDef->DisplayName) = FText::FromString(TEXT("Minério de Ferro"));
		const_cast<int32&>(HeavyItemDef->MaxStackCount) = 100;

		WeightFragment = NewObject<USBItemFragment_Weight>(HeavyItemDef);
		WeightFragment->Weight = 10.0f; // 10kg por unidade
		const_cast<TArray<TObjectPtr<USBItemFragment>>&>(HeavyItemDef->Fragments).Add(WeightFragment);
	});

	AfterEach([this]()
	{
		if (TestCharacter)
		{
			TestCharacter->Destroy();
			TestCharacter = nullptr;
		}
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("ShouldAccumulateWeightCorrectly", [this]()
	{
		// 1. Inicialmente o peso deve ser 0
		float InitialWeight = AttributeComp->GetAttributeValue(FSBGameplayTags::Get().Attribute_Weight);
		TestEqual("Peso inicial deve ser 0", InitialWeight, 0.0f);

		// 2. Adiciona 5 minérios de ferro (5 * 10 = 50kg)
		InventoryComp->ServerAddItem(HeavyItemDef, 5);

		float CalculatedWeight = AttributeComp->GetAttributeValue(FSBGameplayTags::Get().Attribute_Weight);
		TestEqual("Peso deve subir para 50kg", CalculatedWeight, 50.0f);
	});

	It("ShouldTriggerEncumberedStateWhenOverLimit", [this]()
	{
		FGameplayTag EncumberedTag = FSBGameplayTags::Get().State_Character_Encumbered;

		// Limite padrão é 100.f. Adiciona 9 minérios (90kg)
		InventoryComp->ServerAddItem(HeavyItemDef, 9);
		TestFalse("Não deve estar sobrecarregado com 90kg", StateComp->HasTag(EncumberedTag));

		// Adiciona mais 2 minérios (20kg, total = 110kg)
		InventoryComp->ServerAddItem(HeavyItemDef, 2);
		TestTrue("Deve ficar sobrecarregado com 110kg", StateComp->HasTag(EncumberedTag));

		// Remove 3 minérios (30kg, total = 80kg)
		TArray<USBItemInstance*> Items = InventoryComp->GetAllItems();
		TestTrue("Deve possuir itens no inventário", Items.Num() > 0);
		if (Items.Num() > 0)
		{
			InventoryComp->ServerRemoveItem(Items[0], 3);
		}

		TestFalse("Deve remover sobrecarga ao cair abaixo do limite", StateComp->HasTag(EncumberedTag));
	});

	It("ShouldBlockSprintWhenEncumbered", [this]()
	{
		FGameplayTag EncumberedTag = FSBGameplayTags::Get().State_Character_Encumbered;

		USBMovementBehaviorSprint* SprintBehavior = NewObject<USBMovementBehaviorSprint>(MovementComp);
		USBMovementBehaviorDefinition* SprintDef = NewObject<USBMovementBehaviorDefinition>();
		SprintDef->BehaviorTag = FSBGameplayTags::Get().State_Character_Sprinting;
		
		FSBGameplayContext GpContext;
		GpContext.Character = TestCharacter;
		GpContext.Pawn = TestCharacter;

		FSBBehaviorContext Context;
		Context.GameplayContext = &GpContext;

		SprintBehavior->Initialize(MovementComp, SprintDef);

		// 1. Sem sobrecarga, CanEnter deve retornar verdadeiro
		TestTrue("Deve permitir correr sem sobrecarga", SprintBehavior->CanEnter(Context));

		// 2. Com sobrecarga, CanEnter deve retornar falso
		StateComp->AddTag(EncumberedTag);
		TestFalse("Deve bloquear corrida com sobrecarga", SprintBehavior->CanEnter(Context));
	});

	It("ShouldReduceWalkSpeedByHalfWhenEncumbered", [this]()
	{
		FGameplayTag EncumberedTag = FSBGameplayTags::Get().State_Character_Encumbered;

		// Ajusta a velocidade base CMC
		TestCharacter->GetCharacterMovement()->MaxWalkSpeed = 600.0f;

		// 1. Sem sobrecarga, velocidade deve ser normal (600.f)
		float NormalSpeed = MovementComp->GetCalculatedMaxSpeed();
		TestEqual("Velocidade normal deve ser 600", NormalSpeed, 600.0f);

		// 2. Com sobrecarga, velocidade deve cair pela metade (300.f)
		StateComp->AddTag(EncumberedTag);
		float SlowSpeed = MovementComp->GetCalculatedMaxSpeed();
		TestEqual("Velocidade sobrecarregado deve ser 300", SlowSpeed, 300.0f);
	});
}
