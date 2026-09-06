#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Subsystems/SBAsyncSerializationSubsystem.h"
#include "Components/SBAsyncSerializableComponent.h"
#include "SBCoreTestTypes.h"
#include "SBGameplayTags.h"
#include "GameFramework/Actor.h"

BEGIN_DEFINE_SPEC(FSBAsyncSerializationTestsSpec, "Sandbox.Core.AsyncSerialization", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
	UWorld* TestWorld = nullptr;
	USBAsyncSerializationSubsystem* SaveSubsystem = nullptr;
	AActor* TestActor = nullptr;
	USBCoreTestStateComponent* StateComp = nullptr;
	USBAsyncSerializableComponent* SerializableComp = nullptr;
END_DEFINE_SPEC(FSBAsyncSerializationTestsSpec)

void FSBAsyncSerializationTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("AsyncSaveTestWorld"));
		if (TestWorld)
		{
			SaveSubsystem = TestWorld->GetSubsystem<USBAsyncSerializationSubsystem>();
			if (SaveSubsystem)
			{
				SaveSubsystem->ResetSubsystem();
			}

			TestActor = TestWorld->SpawnActor<AActor>();
			if (TestActor)
			{
				StateComp = NewObject<USBCoreTestStateComponent>(TestActor, TEXT("StateComp"));
				TestActor->AddInstanceComponent(StateComp);
				StateComp->RegisterComponent();

				SerializableComp = NewObject<USBAsyncSerializableComponent>(TestActor, TEXT("SerializableComp"));
				TestActor->AddInstanceComponent(SerializableComp);
				SerializableComp->RegisterComponent();
			}
		}
	});

	AfterEach([this]()
	{
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
			SaveSubsystem = nullptr;
			TestActor = nullptr;
			StateComp = nullptr;
			SerializableComp = nullptr;
		}
	});

	It("Should calculate deterministic hash and serialize/deserialize snapshot cleanly", [this]()
	{
		TestNotNull("SaveSubsystem valid", SaveSubsystem);

		TArray<FSBAsyncSaveRecord> Records;
		TArray<uint8> DataA = { 1, 2, 3, 4, 5, 6, 7, 8 };
		TArray<uint8> DataB = { 9, 10, 11, 12 };

		FGuid GuidA = FGuid::NewGuid();
		FGuid GuidB = FGuid::NewGuid();

		Records.Add(FSBAsyncSaveRecord(GuidA, TEXT("Player_Stats"), DataA));
		Records.Add(FSBAsyncSaveRecord(GuidB, TEXT("World_Climate"), DataB));

		int32 BytesSaved = SaveSubsystem->SerializeSnapshotSync(Records, FName("Slot_Alpha"));
		TestEqual("Saved 12 bytes total", BytesSaved, 12);
		TestTrue("Slot_Alpha exists", SaveSubsystem->HasSlot(FName("Slot_Alpha")));

		TArray<FSBAsyncSaveRecord> LoadedRecords;
		bool bLoaded = SaveSubsystem->DeserializeSnapshotSync(FName("Slot_Alpha"), LoadedRecords);
		TestTrue("Deserialize succeeded", bLoaded);
		TestEqual("Loaded 2 records", LoadedRecords.Num(), 2);

		if (LoadedRecords.Num() == 2)
		{
			TestEqual("Record A GUID matches", LoadedRecords[0].EntityGuid, GuidA);
			TestEqual("Record A Tag matches", LoadedRecords[0].RecordTag, FString(TEXT("Player_Stats")));
			TestEqual("Record A byte count matches", LoadedRecords[0].BinaryData.Num(), 8);
			TestFalse("Record A hash is not empty", LoadedRecords[0].ChecksumHash.IsEmpty());

			TestEqual("Record B GUID matches", LoadedRecords[1].EntityGuid, GuidB);
			TestEqual("Record B byte count matches", LoadedRecords[1].BinaryData.Num(), 4);
		}

		FSBAsyncSaveMetrics Metrics = SaveSubsystem->GetMetrics();
		TestEqual("Total saves count is 1", Metrics.TotalSavesCompleted, 1);
		TestEqual("Total loads count is 1", Metrics.TotalLoadsCompleted, 1);
		TestEqual("Total bytes serialized is 12", Metrics.TotalBytesSerialized, 12);
	});

	It("Should partition multiple records into chunks and reconstruct entire payload without byte loss", [this]()
	{
		TestNotNull("SaveSubsystem valid", SaveSubsystem);

		TArray<FSBAsyncSaveRecord> MassRecords;
		const int32 RecordCount = 5;
		const int32 BytesPerRecord = 100;

		for (int32 i = 0; i < RecordCount; ++i)
		{
			TArray<uint8> ChunkData;
			ChunkData.SetNumUninitialized(BytesPerRecord);
			for (int32 b = 0; b < BytesPerRecord; ++b)
			{
				ChunkData[b] = (uint8)((i + b) % 255);
			}

			MassRecords.Add(FSBAsyncSaveRecord(FGuid::NewGuid(), FString::Printf(TEXT("ItemNode_%d"), i), ChunkData));
		}

		int32 TotalBytes = SaveSubsystem->SerializeSnapshotSync(MassRecords, FName("Slot_Bravo"));
		TestEqual("Total bytes serialized is 500", TotalBytes, RecordCount * BytesPerRecord);

		TArray<FSBAsyncSaveRecord> LoadedRecords;
		bool bLoaded = SaveSubsystem->DeserializeSnapshotSync(FName("Slot_Bravo"), LoadedRecords);
		TestTrue("Deserialize mass chunks succeeded", bLoaded);
		TestEqual("Loaded exactly 5 records", LoadedRecords.Num(), RecordCount);

		int32 ReconstructedBytes = 0;
		for (const FSBAsyncSaveRecord& Rec : LoadedRecords)
		{
			ReconstructedBytes += Rec.BinaryData.Num();
		}
		TestEqual("Zero data loss across all records", ReconstructedBytes, 500);
	});

	It("Should detect data corruption through checksum mismatch and reject invalid snapshot", [this]()
	{
		TestNotNull("SaveSubsystem valid", SaveSubsystem);

		TArray<uint8> CleanData = { 100, 101, 102, 103, 104 };
		TArray<FSBAsyncSaveRecord> CorruptibleBatch;
		CorruptibleBatch.Add(FSBAsyncSaveRecord(FGuid::NewGuid(), TEXT("IntegrityTest"), CleanData));

		SaveSubsystem->SerializeSnapshotSync(CorruptibleBatch, FName("Slot_Corrupted"));

		TArray<FSBAsyncSaveRecord> ValidLoad;
		TestTrue("Valid load succeeds initially", SaveSubsystem->DeserializeSnapshotSync(FName("Slot_Corrupted"), ValidLoad));
		TestEqual("Valid load returned 1 record", ValidLoad.Num(), 1);

		// Simula gravação com hash adulterado / forjando payload
		TArray<FSBAsyncSaveRecord> ForgedBatch;
		FSBAsyncSaveRecord ForgedRecord(FGuid::NewGuid(), TEXT("ForgedPayload"), CleanData);
		ForgedRecord.ChecksumHash = TEXT("INVALID_FAKE_CHECKSUM_HEX_12345");
		ForgedBatch.Add(ForgedRecord);

		SaveSubsystem->SerializeSnapshotSync(ForgedBatch, FName("Slot_FakeHash"));

		TArray<FSBAsyncSaveRecord> CorruptedLoad;
		bool bCorruptedResult = SaveSubsystem->DeserializeSnapshotSync(FName("Slot_FakeHash"), CorruptedLoad);
		TestFalse("Corrupted snapshot rejected by checksum validation", bCorruptedResult);
		TestEqual("Output records empty on rejection", CorruptedLoad.Num(), 0);
	});

	It("Should auto-generate GUID, capture state record, grant State.Save.Serialized tag, and restore state cleanly", [this]()
	{
		TestNotNull("SerializableComp valid", SerializableComp);
		TestNotNull("StateComp valid", StateComp);

		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		SerializableComp->SaveCategory = TEXT("Character_Main");
		SerializableComp->OnInitialize_Implementation();

		TestTrue("EntityGuid auto-generated and valid", SerializableComp->UniquePersistentGuid.IsValid());

		TArray<uint8> StatePayload = { 42, 84, 126, 168 };
		FSBAsyncSaveRecord Captured = SerializableComp->CaptureSaveRecord(StatePayload);

		TestEqual("Captured GUID matches component GUID", Captured.EntityGuid, SerializableComp->UniquePersistentGuid);
		TestEqual("Captured RecordTag matches", Captured.RecordTag, FString(TEXT("Character_Main")));
		TestEqual("Captured bytes count is 4", Captured.BinaryData.Num(), 4);
		TestFalse("Checksum computed", Captured.ChecksumHash.IsEmpty());
		TestTrue("State.Save.Serialized tag granted to actor", StateComp->HasTag(Tags.State_Save_Serialized));

		// Modifica dados e restaura
		TArray<uint8> NewPayload = { 99, 98, 97 };
		FSBAsyncSaveRecord UpdateRecord(SerializableComp->UniquePersistentGuid, TEXT("Character_Main"), NewPayload);
		UpdateRecord.ChecksumHash = USBAsyncSerializationSubsystem::ComputePayloadHash(NewPayload);

		bool bApplied = SerializableComp->ApplyLoadRecord(UpdateRecord);
		TestTrue("Applied valid load record", bApplied);
		TestEqual("LastBinaryData updated", SerializableComp->GetLastBinaryData().Num(), 3);

		// Shutdown
		SerializableComp->OnShutdown_Implementation();
		TestFalse("State.Save.Serialized tag removed on shutdown", StateComp->HasTag(Tags.State_Save_Serialized));
	});
}
