// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Subsystems/SBFaultToleranceSubsystem.h"
#include "Components/SBFaultResilientComponent.h"
#include "SBCoreTestTypes.h"
#include "SBGameplayTags.h"
#include "GameFramework/Actor.h"

BEGIN_DEFINE_SPEC(FSBFaultToleranceTestsSpec, "Sandbox.Core.FaultTolerance", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
	UWorld* TestWorld = nullptr;
	USBFaultToleranceSubsystem* FaultSubsystem = nullptr;
	AActor* TestActor = nullptr;
	USBCoreTestStateComponent* StateComp = nullptr;
	USBFaultResilientComponent* ResilientComp = nullptr;
END_DEFINE_SPEC(FSBFaultToleranceTestsSpec)

void FSBFaultToleranceTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("FaultToleranceTestWorld"));
		if (TestWorld)
		{
			FaultSubsystem = TestWorld->GetSubsystem<USBFaultToleranceSubsystem>();
			if (FaultSubsystem)
			{
				FaultSubsystem->ResetSubsystem();
			}

			TestActor = TestWorld->SpawnActor<AActor>();
			if (TestActor)
			{
				StateComp = NewObject<USBCoreTestStateComponent>(TestActor, TEXT("StateComp"));
				TestActor->AddInstanceComponent(StateComp);
				StateComp->RegisterComponent();

				ResilientComp = NewObject<USBFaultResilientComponent>(TestActor, TEXT("ResilientComp"));
				TestActor->AddInstanceComponent(ResilientComp);
				ResilientComp->RegisterComponent();
			}
		}
	});

	AfterEach([this]()
	{
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
			FaultSubsystem = nullptr;
			TestActor = nullptr;
			StateComp = nullptr;
			ResilientComp = nullptr;
		}
	});

	It("Should register service and pass through raw values under healthy normal operation", [this]()
	{
		TestNotNull("FaultSubsystem valid", FaultSubsystem);

		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FaultSubsystem->RegisterService(FName("WeatherService"), Tags.State_Fault_Resilient, 22.0f);

		TestEqual("Initial mode is Normal", (int32)FaultSubsystem->GetServiceMode(FName("WeatherService")), (int32)ESBFaultToleranceMode::Normal);
		TestTrue("Service is healthy", FaultSubsystem->IsServiceHealthy(FName("WeatherService")));

		float SafeVal = FaultSubsystem->QuerySafeValue(FName("WeatherService"), 35.0f, true);
		TestNearlyEqual("Healthy value passes through", SafeVal, 35.0f, 0.01f);

		FSBFaultToleranceMetrics Metrics = FaultSubsystem->GetMetrics();
		TestEqual("Registered services count is 1", Metrics.TotalRegisteredServices, 1);
		TestEqual("Active fallback count is 0", Metrics.ActiveFallbackServices, 0);
	});

	It("Should transition to Degraded and Fallback modes upon repeated failures and return safe contingency values", [this]()
	{
		TestNotNull("FaultSubsystem valid", FaultSubsystem);

		const FSBGameplayTags& Tags = FSBGameplayTags::Get();
		FaultSubsystem->RegisterService(FName("DroneFleetService"), Tags.State_Fault_Resilient, 10.0f);

		// Simula 2 falhas -> Degraded
		FaultSubsystem->ReportServiceFailure(FName("DroneFleetService"));
		FaultSubsystem->ReportServiceFailure(FName("DroneFleetService"));
		TestEqual("Mode transitioned to Degraded", (int32)FaultSubsystem->GetServiceMode(FName("DroneFleetService")), (int32)ESBFaultToleranceMode::Degraded);
		TestFalse("Health marked false", FaultSubsystem->IsServiceHealthy(FName("DroneFleetService")));

		// Simula mais 2 falhas (total 4) -> Fallback
		FaultSubsystem->ReportServiceFailure(FName("DroneFleetService"));
		FaultSubsystem->ReportServiceFailure(FName("DroneFleetService"));
		TestEqual("Mode transitioned to Fallback", (int32)FaultSubsystem->GetServiceMode(FName("DroneFleetService")), (int32)ESBFaultToleranceMode::Fallback);

		// Em modo Fallback, valor retornado deve ser o de contingência (10.0f)
		float SafeVal = FaultSubsystem->QuerySafeValue(FName("DroneFleetService"), 80.0f, true);
		TestNearlyEqual("Fallback returns safe default value", SafeVal, 10.0f, 0.01f);

		// Consulta com serviço nulo/inválido
		float InvalidVal = FaultSubsystem->QuerySafeValue(FName("DroneFleetService"), 80.0f, false);
		TestNearlyEqual("Invalid service safely returns fallback value", InvalidVal, 10.0f, 0.01f);

		FSBFaultToleranceMetrics Metrics = FaultSubsystem->GetMetrics();
		TestEqual("Faults intercepted is 4", Metrics.TotalFaultsIntercepted, 4);
		TestEqual("Active fallback services is 1", Metrics.ActiveFallbackServices, 1);
	});

	It("Should recover service back to Normal mode and restore raw value evaluation", [this]()
	{
		TestNotNull("FaultSubsystem valid", FaultSubsystem);

		const FSBGameplayTags& Tags = FSBGameplayTags::Get();
		FaultSubsystem->RegisterService(FName("PowerGridService"), Tags.State_Fault_Resilient, 50.0f);

		// Coloca em Fallback
		FaultSubsystem->SetServiceMode(FName("PowerGridService"), ESBFaultToleranceMode::Fallback);
		TestEqual("Service in Fallback", (int32)FaultSubsystem->GetServiceMode(FName("PowerGridService")), (int32)ESBFaultToleranceMode::Fallback);

		// Recupera o serviço
		FaultSubsystem->ReportServiceRecovered(FName("PowerGridService"));
		TestEqual("Mode restored to Normal", (int32)FaultSubsystem->GetServiceMode(FName("PowerGridService")), (int32)ESBFaultToleranceMode::Normal);
		TestTrue("Service healthy again", FaultSubsystem->IsServiceHealthy(FName("PowerGridService")));

		float SafeVal = FaultSubsystem->QuerySafeValue(FName("PowerGridService"), 120.0f, true);
		TestNearlyEqual("Evaluates raw value after recovery", SafeVal, 120.0f, 0.01f);
		TestEqual("Recoveries count is 1", FaultSubsystem->GetMetrics().TotalRecoveries, 1);
	});

	It("Should evaluate resilient component gracefully, update state tags on service outage, and cleanup on shutdown", [this]()
	{
		TestNotNull("ResilientComp valid", ResilientComp);
		TestNotNull("StateComp valid", StateComp);

		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FaultSubsystem->RegisterService(FName("RadiationService"), Tags.State_Fault_Resilient, 0.0f);
		ResilientComp->MonitoredService = FName("RadiationService");
		ResilientComp->SafeDefaultValue = 0.0f;
		ResilientComp->OnInitialize_Implementation();

		TestTrue("State.Fault.Resilient tag granted", StateComp->HasTag(Tags.State_Fault_Resilient));

		// Avaliação normal
		float HealthyVal = ResilientComp->SafeEvaluate(75.0f, true);
		TestNearlyEqual("Evaluated healthy value", HealthyVal, 75.0f, 0.01f);
		TestFalse("Fallback tag not present", StateComp->HasTag(Tags.State_Fault_FallbackActive));

		// Avaliação com falha / outage do subsistema
		float FallbackVal = ResilientComp->SafeEvaluate(75.0f, false);
		TestNearlyEqual("Evaluated fallback contingency value (0)", FallbackVal, 0.0f, 0.01f);
		TestTrue("State.Fault.FallbackActive tag granted on outage", StateComp->HasTag(Tags.State_Fault_FallbackActive));

		// Shutdown
		ResilientComp->OnShutdown_Implementation();
		TestFalse("State.Fault.Resilient tag removed on shutdown", StateComp->HasTag(Tags.State_Fault_Resilient));
		TestFalse("State.Fault.FallbackActive tag removed on shutdown", StateComp->HasTag(Tags.State_Fault_FallbackActive));
	});
}
