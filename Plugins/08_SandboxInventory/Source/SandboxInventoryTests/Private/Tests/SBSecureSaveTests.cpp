// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Subsystems/SBSaveSubsystemConcrete.h"
#include "Components/SBInventoryComponent.h"
#include "Components/SBStateComponent.h"
#include "Character/SBCharacter.h"
#include "Kismet/GameplayStatics.h"

BEGIN_DEFINE_SPEC(FSBSecureSaveTestsSpec, "Sandbox.SaveSystem.Security", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	UGameInstance* GameInstance;
	ASBCharacter* TestCharacter;
	USBInventoryComponent* InventoryComponent;
	USBStateComponent* StateComponent;
	USBSaveSubsystemConcrete* SaveSubsystem;
END_DEFINE_SPEC(FSBSecureSaveTestsSpec)

void FSBSecureSaveTestsSpec::Define()
{
	BeforeEach([this]()
	{
		GameInstance = NewObject<UGameInstance>(GEngine);
		GameInstance->InitializeStandalone();

		FWorldContext& NewContext = GEngine->CreateNewWorldContext(EWorldType::Game);
		NewContext.OwningGameInstance = GameInstance;

		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		NewContext.SetCurrentWorld(TestWorld);
		TestWorld->SetGameInstance(GameInstance);

		SaveSubsystem = GameInstance->GetSubsystem<USBSaveSubsystemConcrete>();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("PersistentCharacter");
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		StateComponent = NewObject<USBStateComponent>(TestCharacter, TEXT("PersistentState"));
		StateComponent->RegisterComponent();

		InventoryComponent = NewObject<USBInventoryComponent>(TestCharacter, TEXT("PersistentInventory"));
		InventoryComponent->RegisterComponent();

		ISBComponentInterface::Execute_OnInitialize(StateComponent);
		ISBComponentInterface::Execute_OnInitialize(InventoryComponent);
	});

	AfterEach([this]()
	{
		if (TestCharacter)
		{
			TestCharacter->Destroy();
		}
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
		}
	});

	Describe("Anti-Save Scumming Security Verification", [this]()
	{
		It("Should successfully save and load save games in normal secure flow", [this]()
		{
			FString SlotName = TEXT("SecureTestSlot_Normal");
			
			// Executa save limpo
			bool bSaveSuccess = SaveSubsystem->SaveGame(SlotName, 0);
			TestTrue("Salvamento seguro deve retornar true", bSaveSuccess);

			// Executa load limpo
			bool bLoadSuccess = SaveSubsystem->LoadGame(SlotName, 0);
			TestTrue("Carregamento seguro legítimo deve retornar true", bLoadSuccess);
		});

		It("Should reject load and raise security alert if encrypted payload is tampered with", [this]()
		{
			FString SlotName = TEXT("SecureTestSlot_TamperedPayload");
			
			// 1. Salva de forma legítima
			bool bSaveSuccess = SaveSubsystem->SaveGame(SlotName, 0);
			TestTrue("Salvamento inicial ok", bSaveSuccess);

			// 2. Carrega o contêiner e corrompe o payload
			USBSecureSaveGame* SecureSave = Cast<USBSecureSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
			TestNotNull("Wrapper de save deve ter sido criado", SecureSave);
			
			if (SecureSave && SecureSave->EncryptedPayload.Num() > 0)
			{
				// Inverte o primeiro byte do payload criptografado
				SecureSave->EncryptedPayload[0] ^= 0xFF;
				
				// Salva de volta o arquivo corrompido/adulterado
				bool bWriteBack = UGameplayStatics::SaveGameToSlot(SecureSave, SlotName, 0);
				TestTrue("Escrever payload adulterado de volta deve ser permitido pela engine", bWriteBack);

				// 3. Tenta carregar usando o subsistema seguro do Sandbox
				bool bLoadResult = SaveSubsystem->LoadGame(SlotName, 0);
				TestFalse("Carregamento de payload adulterado deve ser rejeitado (Anti-Save Scumming)", bLoadResult);
			}
		});

		It("Should reject load and raise security alert if signature itself is tampered with", [this]()
		{
			FString SlotName = TEXT("SecureTestSlot_TamperedSignature");

			// 1. Salva legítimo
			bool bSaveSuccess = SaveSubsystem->SaveGame(SlotName, 0);
			TestTrue("Salvamento inicial ok", bSaveSuccess);

			// 2. Carrega contêiner e corrompe apenas a assinatura digital
			USBSecureSaveGame* SecureSave = Cast<USBSecureSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
			TestNotNull("Wrapper de save deve ter sido criado", SecureSave);

			if (SecureSave)
			{
				// Adulterar assinatura
				SecureSave->Signature = TEXT("INVALID_SIGNATURE_FOR_TESTING");

				// Salva contêiner
				bool bWriteBack = UGameplayStatics::SaveGameToSlot(SecureSave, SlotName, 0);
				TestTrue("Escrever assinatura adulterada de volta ok", bWriteBack);

				// 3. Tenta carregar
				bool bLoadResult = SaveSubsystem->LoadGame(SlotName, 0);
				TestFalse("Carregamento de assinatura adulterada deve ser rejeitado", bLoadResult);
			}
		});
	});
}
