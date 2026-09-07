// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Subsystems/SBSaveSubsystemConcrete.h"

BEGIN_DEFINE_SPEC(FSBSaveMigrationTestsSpec, "Sandbox.SaveSystem.Migration", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UGameInstance* GameInstance;
	USBSaveSubsystemConcrete* SaveSubsystem;
END_DEFINE_SPEC(FSBSaveMigrationTestsSpec)

void FSBSaveMigrationTestsSpec::Define()
{
	BeforeEach([this]()
	{
		GameInstance = NewObject<UGameInstance>(GEngine);
		SaveSubsystem = NewObject<USBSaveSubsystemConcrete>(GameInstance);
	});

	AfterEach([this]()
	{
		SaveSubsystem = nullptr;
		GameInstance = nullptr;
	});

	It("Should execute registered migration steps sequentially on legacy save payload", [this]()
	{
		USBSavePayload* Payload = NewObject<USBSavePayload>();
		
		// 1. Dados iniciais legados (v1)
		TArray<uint8> OldData = { 10, 20, 30 };
		Payload->WriteBinaryData(TEXT("Legacy_Key"), OldData);

		// 2. Registra passo de migração v1 -> v2: Transfere "Legacy_Key" para "Modern_Key" com transformação
		SaveSubsystem->RegisterMigrationStep(1, 2, [](USBSavePayload* InPayload)
		{
			TArray<uint8> ReadBytes;
			if (InPayload->ReadBinaryData(TEXT("Legacy_Key"), ReadBytes))
			{
				// Adiciona um byte extra na migração para v2
				ReadBytes.Add(40);
				InPayload->WriteBinaryData(TEXT("Modern_Key"), ReadBytes);
				InPayload->ObjectDataMap.Remove(TEXT("Legacy_Key"));
				return true;
			}
			return false;
		}, TEXT("Migrate Legacy_Key to Modern_Key"));

		// 3. Registra passo de migração v2 -> v3: Duplica o tamanho de "Modern_Key"
		SaveSubsystem->RegisterMigrationStep(2, 3, [](USBSavePayload* InPayload)
		{
			TArray<uint8> ReadBytes;
			if (InPayload->ReadBinaryData(TEXT("Modern_Key"), ReadBytes))
			{
				ReadBytes.Add(50);
				InPayload->WriteBinaryData(TEXT("V3_Key"), ReadBytes);
				InPayload->ObjectDataMap.Remove(TEXT("Modern_Key"));
				return true;
			}
			return false;
		}, TEXT("Migrate Modern_Key to V3_Key"));

		// 4. Executa a migração a partir da versão 1 até a versão 3
		int32 CurrentVersion = 1;
		bool bSuccess = SaveSubsystem->MigratePayload(Payload, CurrentVersion, 3);

		TestTrue("MigratePayload should return true", bSuccess);
		TestEqual("CurrentVersion should be updated to 3", CurrentVersion, 3);

		// 5. Valida os dados transformados
		TArray<uint8> V3Data;
		TestTrue("V3_Key must exist in migrated payload", Payload->ReadBinaryData(TEXT("V3_Key"), V3Data));
		TestEqual("V3Data should have 5 bytes", V3Data.Num(), 5);
		if (V3Data.Num() == 5)
		{
			TestEqual("Byte 0", V3Data[0], static_cast<uint8>(10));
			TestEqual("Byte 1", V3Data[1], static_cast<uint8>(20));
			TestEqual("Byte 2", V3Data[2], static_cast<uint8>(30));
			TestEqual("Byte 3", V3Data[3], static_cast<uint8>(40));
			TestEqual("Byte 4", V3Data[4], static_cast<uint8>(50));
		}

		TArray<uint8> DummyData;
		TestFalse("Legacy_Key must be removed after migration", Payload->ReadBinaryData(TEXT("Legacy_Key"), DummyData));
		TestFalse("Modern_Key must be removed after migration", Payload->ReadBinaryData(TEXT("Modern_Key"), DummyData));
	});

	It("Should skip migration when payload is already at target version", [this]()
	{
		USBSavePayload* Payload = NewObject<USBSavePayload>();
		TArray<uint8> TestData = { 1, 2, 3 };
		Payload->WriteBinaryData(TEXT("Current_Data"), TestData);

		int32 CurrentVersion = 2;
		bool bSuccess = SaveSubsystem->MigratePayload(Payload, CurrentVersion, 2);

		TestTrue("MigratePayload should succeed immediately", bSuccess);
		TestEqual("Version should remain 2", CurrentVersion, 2);

		TArray<uint8> ReadBytes;
		TestTrue("Current_Data should remain intact", Payload->ReadBinaryData(TEXT("Current_Data"), ReadBytes));
	});
}
