// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Subsystems/SBLiveConfigSubsystem.h"
#include "Components/SBLiveConfigObserverComponent.h"
#include "SBCoreTestTypes.h"
#include "SBGameplayTags.h"
#include "GameFramework/Actor.h"

BEGIN_DEFINE_SPEC(FSBLiveConfigTestsSpec, "Sandbox.Core.LiveConfig", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
	UWorld* TestWorld = nullptr;
	USBLiveConfigSubsystem* ConfigSubsystem = nullptr;
	AActor* TestActor = nullptr;
	USBCoreTestStateComponent* StateComp = nullptr;
	USBLiveConfigObserverComponent* ObserverComp = nullptr;
END_DEFINE_SPEC(FSBLiveConfigTestsSpec)

void FSBLiveConfigTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("LiveConfigTestWorld"));
		if (TestWorld)
		{
			ConfigSubsystem = TestWorld->GetSubsystem<USBLiveConfigSubsystem>();
			if (ConfigSubsystem)
			{
				ConfigSubsystem->ResetSubsystem();
			}

			TestActor = TestWorld->SpawnActor<AActor>();
			if (TestActor)
			{
				StateComp = NewObject<USBCoreTestStateComponent>(TestActor, TEXT("StateComp"));
				TestActor->AddInstanceComponent(StateComp);
				StateComp->RegisterComponent();

				ObserverComp = NewObject<USBLiveConfigObserverComponent>(TestActor, TEXT("ObserverComp"));
				TestActor->AddInstanceComponent(ObserverComp);
				ObserverComp->RegisterComponent();
			}
		}
	});

	AfterEach([this]()
	{
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
			ConfigSubsystem = nullptr;
			TestActor = nullptr;
			StateComp = nullptr;
			ObserverComp = nullptr;
		}
	});

	It("Should register schema and retrieve typed float and string config properties", [this]()
	{
		TestNotNull("ConfigSubsystem valid", ConfigSubsystem);

		TMap<FName, float> FloatProps;
		FloatProps.Add(FName("BaseDamage"), 25.0f);
		FloatProps.Add(FName("AttackSpeed"), 1.2f);

		TMap<FName, FString> StringProps;
		StringProps.Add(FName("WeaponName"), TEXT("IronSword"));

		ConfigSubsystem->RegisterSchema(FName("Balance_Melee"), FloatProps, StringProps);

		TestEqual("Initial version is 1", ConfigSubsystem->GetSchemaVersion(FName("Balance_Melee")), 1);
		TestNearlyEqual("BaseDamage is 25.0", ConfigSubsystem->GetFloatConfig(FName("Balance_Melee"), FName("BaseDamage")), 25.0f, 0.01f);
		TestNearlyEqual("AttackSpeed is 1.2", ConfigSubsystem->GetFloatConfig(FName("Balance_Melee"), FName("AttackSpeed")), 1.2f, 0.01f);
		TestEqual("WeaponName is IronSword", ConfigSubsystem->GetStringConfig(FName("Balance_Melee"), FName("WeaponName")), FString(TEXT("IronSword")));

		// Valor não existente retorna default
		TestNearlyEqual("Non-existent property returns fallback default", ConfigSubsystem->GetFloatConfig(FName("Balance_Melee"), FName("ArmorPen"), 0.0f), 0.0f, 0.01f);
	});

	It("Should hot-reload schema, increment schema version, and update live property values in place", [this]()
	{
		TestNotNull("ConfigSubsystem valid", ConfigSubsystem);

		TMap<FName, float> InitialFloats;
		InitialFloats.Add(FName("PotionPrice"), 50.0f);
		TMap<FName, FString> InitialStrings;

		ConfigSubsystem->RegisterSchema(FName("Economic_Pricing"), InitialFloats, InitialStrings);
		TestEqual("Version is 1", ConfigSubsystem->GetSchemaVersion(FName("Economic_Pricing")), 1);

		// Executa Hot-Reload
		TMap<FName, float> UpdatedFloats;
		UpdatedFloats.Add(FName("PotionPrice"), 75.0f);
		UpdatedFloats.Add(FName("TaxRate"), 0.15f);
		TMap<FName, FString> UpdatedStrings;

		int32 NewVer = ConfigSubsystem->HotReloadSchema(FName("Economic_Pricing"), UpdatedFloats, UpdatedStrings);
		TestEqual("New version is 2", NewVer, 2);
		TestEqual("GetSchemaVersion is 2", ConfigSubsystem->GetSchemaVersion(FName("Economic_Pricing")), 2);
		TestNearlyEqual("PotionPrice updated to 75.0", ConfigSubsystem->GetFloatConfig(FName("Economic_Pricing"), FName("PotionPrice")), 75.0f, 0.01f);
		TestNearlyEqual("TaxRate added as 0.15", ConfigSubsystem->GetFloatConfig(FName("Economic_Pricing"), FName("TaxRate")), 0.15f, 0.01f);
	});

	It("Should track telemetry metrics for schemas, hot-reloads executed, and properties updated", [this]()
	{
		TestNotNull("ConfigSubsystem valid", ConfigSubsystem);

		TMap<FName, float> F1;
		F1.Add(FName("ValA"), 10.0f);
		TMap<FName, FString> S1;
		ConfigSubsystem->RegisterSchema(FName("Schema_One"), F1, S1);

		TMap<FName, float> F2;
		F2.Add(FName("ValB"), 20.0f);
		TMap<FName, FString> S2;
		ConfigSubsystem->RegisterSchema(FName("Schema_Two"), F2, S2);

		// Recarrega Schema_One duas vezes
		TMap<FName, float> Up1;
		Up1.Add(FName("ValA"), 15.0f);
		ConfigSubsystem->HotReloadSchema(FName("Schema_One"), Up1, S1);

		TMap<FName, float> Up2;
		Up2.Add(FName("ValA"), 25.0f);
		ConfigSubsystem->HotReloadSchema(FName("Schema_One"), Up2, S1);

		FSBLiveConfigMetrics Metrics = ConfigSubsystem->GetMetrics();
		TestEqual("Total schemas is 2", Metrics.TotalSchemasRegistered, 2);
		TestEqual("Total hot reloads is 2", Metrics.TotalHotReloadsExecuted, 2);
		TestTrue("Total properties updated is >= 2", Metrics.TotalPropertiesUpdated >= 2);
	});

	It("Should auto-subscribe observer component, receive hot-reload notification, update state tags, and cleanup", [this]()
	{
		TestNotNull("ObserverComp valid", ObserverComp);
		TestNotNull("StateComp valid", StateComp);

		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		TMap<FName, float> InitialFloats;
		InitialFloats.Add(FName("CalorieDecay"), 1.0f);
		TMap<FName, FString> InitialStrings;
		ConfigSubsystem->RegisterSchema(FName("Survival_Metabolism"), InitialFloats, InitialStrings);

		ObserverComp->WatchedSchema = FName("Survival_Metabolism");
		ObserverComp->OnInitialize_Implementation();

		TestTrue("State.Config.Observing tag granted", StateComp->HasTag(Tags.State_Config_Observing));
		TestTrue("State.Config.SchemaSynced tag granted", StateComp->HasTag(Tags.State_Config_SchemaSynced));
		TestEqual("Initial observed version is 1", ObserverComp->GetObservedVersion(), 1);

		// Dispara hot reload no subsistema
		TMap<FName, float> UpdatedFloats;
		UpdatedFloats.Add(FName("CalorieDecay"), 1.5f);
		ConfigSubsystem->HotReloadSchema(FName("Survival_Metabolism"), UpdatedFloats, InitialStrings);

		TestEqual("Observer updated to version 2", ObserverComp->GetObservedVersion(), 2);
		TestTrue("State.Config.HotReloadActive tag granted on update", StateComp->HasTag(Tags.State_Config_HotReloadActive));

		// Shutdown
		ObserverComp->OnShutdown_Implementation();
		TestFalse("Config tags removed on shutdown", StateComp->HasTag(Tags.State_Config_Observing));
		TestFalse("Synced tag removed on shutdown", StateComp->HasTag(Tags.State_Config_SchemaSynced));
	});
}
