#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBLogicCircuitComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBLogicCircuitTestsSpec, "Sandbox.Inventory.LogicCircuit", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* CircuitActor;
	USBLogicCircuitComponent* LogicComp;
	USBStateComponent* CircuitStateComp;
END_DEFINE_SPEC(FSBLogicCircuitTestsSpec)

void FSBLogicCircuitTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CircuitActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Root = NewObject<USceneComponent>(CircuitActor, TEXT("Root"));
		CircuitActor->SetRootComponent(Root);
		Root->RegisterComponent();

		CircuitStateComp = NewObject<USBStateComponent>(CircuitActor, TEXT("CircuitStateComp"));
		CircuitActor->AddOwnedComponent(CircuitStateComp);

		LogicComp = NewObject<USBLogicCircuitComponent>(CircuitActor, TEXT("LogicComp"));
		CircuitActor->AddOwnedComponent(LogicComp);

		ISBComponentInterface::Execute_OnInitialize(CircuitStateComp);
		ISBComponentInterface::Execute_OnInitialize(LogicComp);
	});

	AfterEach([this]()
	{
		if (CircuitActor)
		{
			CircuitActor->Destroy();
			CircuitActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should evaluate comparator condition (>), emit output signal, and grant ConditionMet tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		LogicComp->SetupComparator(FName(TEXT("Comp_01")), FName(TEXT("IronOre")), ESBLogicComparisonOp::GreaterThan, 50.0f, FName(TEXT("AlarmSignal")));

		// Inset 20 IronOre -> 20 > 50 is false
		LogicComp->InjectSignal(ESBLogicWireColor::RedWire, FName(TEXT("IronOre")), 20.0f);
		TestFalse("Condition not met", LogicComp->IsConditionMet());
		TestTrue("Has ConditionFailed tag", CircuitStateComp->HasTag(Tags.State_Logic_ConditionFailed));

		// Inset 75 IronOre -> 75 > 50 is true
		LogicComp->InjectSignal(ESBLogicWireColor::RedWire, FName(TEXT("IronOre")), 75.0f);
		TestTrue("Condition met", LogicComp->IsConditionMet());
		TestTrue("Has ConditionMet tag", CircuitStateComp->HasTag(Tags.State_Logic_ConditionMet));
		TestEqual("Output signal emitted", LogicComp->ReadSignal(ESBLogicWireColor::RedWire, FName(TEXT("AlarmSignal"))), 1.0f);
	});

	It("Should evaluate boolean AND gate requiring both inputs to be positive", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		LogicComp->SetupLogicGate(FName(TEXT("Gate_AND")), ESBLogicNodeType::LogicGateAND, FName(TEXT("SwitchA")), FName(TEXT("SwitchB")), FName(TEXT("DoorOpen")));

		// Only SwitchA is ON
		LogicComp->InjectSignal(ESBLogicWireColor::RedWire, FName(TEXT("SwitchA")), 1.0f);
		TestFalse("AND not satisfied with single input", LogicComp->IsConditionMet());
		TestEqual("DoorOpen output is 0", LogicComp->ReadSignal(ESBLogicWireColor::RedWire, FName(TEXT("DoorOpen"))), 0.0f);

		// SwitchB is also ON via GreenWire
		LogicComp->InjectSignal(ESBLogicWireColor::GreenWire, FName(TEXT("SwitchB")), 1.0f);
		TestTrue("AND satisfied with both inputs", LogicComp->IsConditionMet());
		TestTrue("Has ConditionMet tag", CircuitStateComp->HasTag(Tags.State_Logic_ConditionMet));
		TestEqual("DoorOpen output is 1", LogicComp->ReadSignal(ESBLogicWireColor::RedWire, FName(TEXT("DoorOpen"))), 1.0f);
	});

	It("Should compute arithmetic multiplication operation and update signal bus", [this]()
	{
		LogicComp->SetupArithmeticProcessor(FName(TEXT("Arith_01")), FName(TEXT("FluidVolume")), ESBLogicArithmeticOp::Multiply, 2.0f, FName(TEXT("DoubleFluid")));

		// Inject FluidVolume = 40.0
		LogicComp->InjectSignal(ESBLogicWireColor::RedWire, FName(TEXT("FluidVolume")), 40.0f);

		TestTrue("Processor evaluated", LogicComp->IsConditionMet());
		TestEqual("Output value is 80", LogicComp->GetLogicGateData().OutputValue, 80.0f);
		TestEqual("Signal bus has DoubleFluid = 80", LogicComp->ReadSignal(ESBLogicWireColor::RedWire, FName(TEXT("DoubleFluid"))), 80.0f);
	});

	It("Should maintain RS-Latch state with Set pulse until Reset pulse is received", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		LogicComp->SetupRSLatch(FName(TEXT("Latch_01")), FName(TEXT("SetPulse")), FName(TEXT("ResetPulse")), FName(TEXT("PowerRelay")));
		TestFalse("Initial latch is OFF", LogicComp->IsConditionMet());

		// Send Set pulse
		LogicComp->InjectSignal(ESBLogicWireColor::RedWire, FName(TEXT("SetPulse")), 1.0f);
		TestTrue("Latch turned ON", LogicComp->IsConditionMet());
		TestTrue("Has ConditionMet tag", CircuitStateComp->HasTag(Tags.State_Logic_ConditionMet));
		TestEqual("Relay output is 1", LogicComp->ReadSignal(ESBLogicWireColor::RedWire, FName(TEXT("PowerRelay"))), 1.0f);

		// Clear pulse, latch memory must remain ON!
		LogicComp->ClearSignals(ESBLogicWireColor::RedWire);
		TestTrue("Latch memory holds ON state", LogicComp->IsConditionMet());

		// Send Reset pulse
		LogicComp->InjectSignal(ESBLogicWireColor::RedWire, FName(TEXT("ResetPulse")), 1.0f);
		TestFalse("Latch turned OFF after reset", LogicComp->IsConditionMet());
		TestTrue("Has ConditionFailed tag", CircuitStateComp->HasTag(Tags.State_Logic_ConditionFailed));
	});
}
