#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBPowerGridComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBPowerGridTestsSpec, "Sandbox.Inventory.PowerGrid", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* GeneratorActor;
	AActor* BatteryActor;
	AActor* ConsumerActor;
	USBPowerGridComponent* GeneratorComp;
	USBPowerGridComponent* BatteryComp;
	USBPowerGridComponent* ConsumerComp;
	USBStateComponent* GeneratorStateComp;
	USBStateComponent* BatteryStateComp;
	USBStateComponent* ConsumerStateComp;
END_DEFINE_SPEC(FSBPowerGridTestsSpec)

void FSBPowerGridTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Generator
		GeneratorActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* GenRoot = NewObject<USceneComponent>(GeneratorActor, TEXT("GenRoot"));
		GeneratorActor->SetRootComponent(GenRoot);
		GenRoot->RegisterComponent();

		GeneratorStateComp = NewObject<USBStateComponent>(GeneratorActor, TEXT("GeneratorStateComp"));
		GeneratorActor->AddOwnedComponent(GeneratorStateComp);

		GeneratorComp = NewObject<USBPowerGridComponent>(GeneratorActor, TEXT("GeneratorComp"));
		GeneratorActor->AddOwnedComponent(GeneratorComp);

		// Battery
		BatteryActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(500.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* BatRoot = NewObject<USceneComponent>(BatteryActor, TEXT("BatRoot"));
		BatteryActor->SetRootComponent(BatRoot);
		BatRoot->RegisterComponent();
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		BatteryActor->SetActorLocation(FVector(500.0f, 0.0f, 0.0f));

		BatteryStateComp = NewObject<USBStateComponent>(BatteryActor, TEXT("BatteryStateComp"));
		BatteryActor->AddOwnedComponent(BatteryStateComp);

		BatteryComp = NewObject<USBPowerGridComponent>(BatteryActor, TEXT("BatteryComp"));
		BatteryActor->AddOwnedComponent(BatteryComp);

		// Consumer
		ConsumerActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(1000.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* ConRoot = NewObject<USceneComponent>(ConsumerActor, TEXT("ConRoot"));
		ConsumerActor->SetRootComponent(ConRoot);
		ConRoot->RegisterComponent();
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		ConsumerActor->SetActorLocation(FVector(1000.0f, 0.0f, 0.0f));

		ConsumerStateComp = NewObject<USBStateComponent>(ConsumerActor, TEXT("ConsumerStateComp"));
		ConsumerActor->AddOwnedComponent(ConsumerStateComp);

		ConsumerComp = NewObject<USBPowerGridComponent>(ConsumerActor, TEXT("ConsumerComp"));
		ConsumerActor->AddOwnedComponent(ConsumerComp);

		ISBComponentInterface::Execute_OnInitialize(GeneratorStateComp);
		ISBComponentInterface::Execute_OnInitialize(GeneratorComp);
		ISBComponentInterface::Execute_OnInitialize(BatteryStateComp);
		ISBComponentInterface::Execute_OnInitialize(BatteryComp);
		ISBComponentInterface::Execute_OnInitialize(ConsumerStateComp);
		ISBComponentInterface::Execute_OnInitialize(ConsumerComp);
	});

	AfterEach([this]()
	{
		if (ConsumerActor)
		{
			ConsumerActor->Destroy();
			ConsumerActor = nullptr;
		}

		if (BatteryActor)
		{
			BatteryActor->Destroy();
			BatteryActor = nullptr;
		}

		if (GeneratorActor)
		{
			GeneratorActor->Destroy();
			GeneratorActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should connect generator to consumer, satisfy power demand, and grant Powered tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		GeneratorComp->SetupNode(ESBPowerNodeType::Generator, 1000.0f, 0.0f, 0.0f);
		ConsumerComp->SetupNode(ESBPowerNodeType::Consumer, 0.0f, 400.0f, 0.0f);

		bool bConnected = ConsumerComp->ConnectToPowerNode(GeneratorComp);

		TestTrue("Connected succeeded", bConnected);
		TestTrue("Consumer is powered", ConsumerComp->IsPowered());
		TestEqual("Consumer state is Powered", (int32)ConsumerComp->GetGridState(), (int32)ESBPowerGridState::Powered);
		TestTrue("Consumer has Powered tag", ConsumerStateComp->HasTag(Tags.State_Power_Powered));
	});

	It("Should connect battery to circuit, absorb surplus generation, and grant Charging tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		GeneratorComp->SetupNode(ESBPowerNodeType::Generator, 1000.0f, 0.0f, 0.0f);
		ConsumerComp->SetupNode(ESBPowerNodeType::Consumer, 0.0f, 400.0f, 0.0f);
		BatteryComp->SetupNode(ESBPowerNodeType::Battery, 0.0f, 0.0f, 5000.0f);

		ConsumerComp->ConnectToPowerNode(GeneratorComp);
		BatteryComp->ConnectToPowerNode(GeneratorComp);

		TestEqual("Battery state is Charging", (int32)BatteryComp->GetGridState(), (int32)ESBPowerGridState::Charging);
		TestTrue("Battery has Charging tag", BatteryStateComp->HasTag(Tags.State_Power_Charging));
	});

	It("Should discharge battery when generator shuts down, keeping consumer powered with Discharging tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		GeneratorComp->SetupNode(ESBPowerNodeType::Generator, 1000.0f, 0.0f, 0.0f);
		ConsumerComp->SetupNode(ESBPowerNodeType::Consumer, 0.0f, 400.0f, 0.0f);
		BatteryComp->SetupNode(ESBPowerNodeType::Battery, 0.0f, 0.0f, 5000.0f);

		ConsumerComp->ConnectToPowerNode(GeneratorComp);
		BatteryComp->ConnectToPowerNode(GeneratorComp);

		// Shut down generator
		GeneratorComp->SetupNode(ESBPowerNodeType::Generator, 0.0f, 0.0f, 0.0f);

		TestTrue("Consumer still powered by battery", ConsumerComp->IsPowered());
		TestEqual("Battery state is Discharging", (int32)BatteryComp->GetGridState(), (int32)ESBPowerGridState::Discharging);
		TestTrue("Battery has Discharging tag", BatteryStateComp->HasTag(Tags.State_Power_Discharging));
	});

	It("Should trip circuit breaker on severe overload, set Overloaded state, and unpower consumers", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		GeneratorComp->SetupNode(ESBPowerNodeType::Generator, 1000.0f, 0.0f, 0.0f);
		ConsumerComp->SetupNode(ESBPowerNodeType::Consumer, 0.0f, 50000.0f, 0.0f);

		ConsumerComp->ConnectToPowerNode(GeneratorComp);

		TestFalse("Consumer not powered due to overload", ConsumerComp->IsPowered());
		TestEqual("Consumer state is Overloaded", (int32)ConsumerComp->GetGridState(), (int32)ESBPowerGridState::Overloaded);
		TestTrue("Consumer has Overloaded tag", ConsumerStateComp->HasTag(Tags.State_Power_Overloaded));
	});
}
