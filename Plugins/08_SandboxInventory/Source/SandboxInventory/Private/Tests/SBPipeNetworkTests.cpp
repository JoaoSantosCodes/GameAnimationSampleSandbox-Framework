#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBPipeNetworkComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBPipeNetworkTestsSpec, "Sandbox.Inventory.PipeNetwork", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* PumpActor;
	AActor* PipeActor;
	AActor* TankActor;
	AActor* ValveActor;
	AActor* FragilePipeActor;
	USBPipeNetworkComponent* PumpComp;
	USBPipeNetworkComponent* PipeComp;
	USBPipeNetworkComponent* TankComp;
	USBPipeNetworkComponent* ValveComp;
	USBPipeNetworkComponent* FragilePipeComp;
	USBStateComponent* PumpStateComp;
	USBStateComponent* PipeStateComp;
	USBStateComponent* TankStateComp;
	USBStateComponent* ValveStateComp;
	USBStateComponent* FragilePipeStateComp;
END_DEFINE_SPEC(FSBPipeNetworkTestsSpec)

void FSBPipeNetworkTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Pump
		PumpActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* PumpRoot = NewObject<USceneComponent>(PumpActor, TEXT("PumpRoot"));
		PumpActor->SetRootComponent(PumpRoot);
		PumpRoot->RegisterComponent();

		PumpStateComp = NewObject<USBStateComponent>(PumpActor, TEXT("PumpStateComp"));
		PumpActor->AddOwnedComponent(PumpStateComp);

		PumpComp = NewObject<USBPipeNetworkComponent>(PumpActor, TEXT("PumpComp"));
		PumpActor->AddOwnedComponent(PumpComp);

		// Pipe
		PipeActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(300.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* PipeRoot = NewObject<USceneComponent>(PipeActor, TEXT("PipeRoot"));
		PipeActor->SetRootComponent(PipeRoot);
		PipeRoot->RegisterComponent();
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		PipeActor->SetActorLocation(FVector(300.0f, 0.0f, 0.0f));

		PipeStateComp = NewObject<USBStateComponent>(PipeActor, TEXT("PipeStateComp"));
		PipeActor->AddOwnedComponent(PipeStateComp);

		PipeComp = NewObject<USBPipeNetworkComponent>(PipeActor, TEXT("PipeComp"));
		PipeActor->AddOwnedComponent(PipeComp);

		// Tank
		TankActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(600.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* TankRoot = NewObject<USceneComponent>(TankActor, TEXT("TankRoot"));
		TankActor->SetRootComponent(TankRoot);
		TankRoot->RegisterComponent();
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		TankActor->SetActorLocation(FVector(600.0f, 0.0f, 0.0f));

		TankStateComp = NewObject<USBStateComponent>(TankActor, TEXT("TankStateComp"));
		TankActor->AddOwnedComponent(TankStateComp);

		TankComp = NewObject<USBPipeNetworkComponent>(TankActor, TEXT("TankComp"));
		TankActor->AddOwnedComponent(TankComp);

		// Valve
		ValveActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(150.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* ValveRoot = NewObject<USceneComponent>(ValveActor, TEXT("ValveRoot"));
		ValveActor->SetRootComponent(ValveRoot);
		ValveRoot->RegisterComponent();
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		ValveActor->SetActorLocation(FVector(150.0f, 0.0f, 0.0f));

		ValveStateComp = NewObject<USBStateComponent>(ValveActor, TEXT("ValveStateComp"));
		ValveActor->AddOwnedComponent(ValveStateComp);

		ValveComp = NewObject<USBPipeNetworkComponent>(ValveActor, TEXT("ValveComp"));
		ValveActor->AddOwnedComponent(ValveComp);

		// Fragile Pipe
		FragilePipeActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(900.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* FPipeRoot = NewObject<USceneComponent>(FragilePipeActor, TEXT("FPipeRoot"));
		FragilePipeActor->SetRootComponent(FPipeRoot);
		FPipeRoot->RegisterComponent();
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		FragilePipeActor->SetActorLocation(FVector(900.0f, 0.0f, 0.0f));

		FragilePipeStateComp = NewObject<USBStateComponent>(FragilePipeActor, TEXT("FragilePipeStateComp"));
		FragilePipeActor->AddOwnedComponent(FragilePipeStateComp);

		FragilePipeComp = NewObject<USBPipeNetworkComponent>(FragilePipeActor, TEXT("FragilePipeComp"));
		FragilePipeActor->AddOwnedComponent(FragilePipeComp);

		ISBComponentInterface::Execute_OnInitialize(PumpStateComp);
		ISBComponentInterface::Execute_OnInitialize(PumpComp);
		ISBComponentInterface::Execute_OnInitialize(PipeStateComp);
		ISBComponentInterface::Execute_OnInitialize(PipeComp);
		ISBComponentInterface::Execute_OnInitialize(TankStateComp);
		ISBComponentInterface::Execute_OnInitialize(TankComp);
		ISBComponentInterface::Execute_OnInitialize(ValveStateComp);
		ISBComponentInterface::Execute_OnInitialize(ValveComp);
		ISBComponentInterface::Execute_OnInitialize(FragilePipeStateComp);
		ISBComponentInterface::Execute_OnInitialize(FragilePipeComp);
	});

	AfterEach([this]()
	{
		if (FragilePipeActor)
		{
			FragilePipeActor->Destroy();
			FragilePipeActor = nullptr;
		}

		if (ValveActor)
		{
			ValveActor->Destroy();
			ValveActor = nullptr;
		}

		if (TankActor)
		{
			TankActor->Destroy();
			TankActor = nullptr;
		}

		if (PipeActor)
		{
			PipeActor->Destroy();
			PipeActor = nullptr;
		}

		if (PumpActor)
		{
			PumpActor->Destroy();
			PumpActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should connect pump to pipe and tank, pressurize water flow, and grant Flowing and Pressurized tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		PumpComp->SetupNode(ESBPipeNodeType::SourcePump, ESBFluidType::Water, 1000.0f, 20.0f);
		PipeComp->SetupNode(ESBPipeNodeType::PipeSegment, ESBFluidType::None, 100.0f, 20.0f);
		TankComp->SetupNode(ESBPipeNodeType::FluidTank, ESBFluidType::None, 5000.0f, 20.0f);

		PipeComp->ConnectPipe(PumpComp);
		TankComp->ConnectPipe(PipeComp);

		PumpComp->SetPumpActive(true);

		TestTrue("Pipe is flowing", PipeComp->IsFlowing());
		TestEqual("Pipe state is Flowing", (int32)PipeComp->GetFlowState(), (int32)ESBPipeFlowState::Flowing);
		TestTrue("Pipe has Flowing tag", PipeStateComp->HasTag(Tags.State_Fluid_Flowing));
		TestTrue("Pipe has Pressurized tag", PipeStateComp->HasTag(Tags.State_Fluid_Pressurized));
	});

	It("Should close valve, block downstream fluid flow, and grant Blocked tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		PumpComp->SetupNode(ESBPipeNodeType::SourcePump, ESBFluidType::Water, 1000.0f, 20.0f);
		ValveComp->SetupNode(ESBPipeNodeType::Valve, ESBFluidType::Water, 100.0f, 20.0f);

		ValveComp->ConnectPipe(PumpComp);
		PumpComp->SetPumpActive(true);

		ValveComp->SetValveOpenPercentage(0.0f);

		TestEqual("Valve state is Blocked", (int32)ValveComp->GetFlowState(), (int32)ESBPipeFlowState::Blocked);
		TestTrue("Valve has Blocked tag", ValveStateComp->HasTag(Tags.State_Fluid_Blocked));
	});

	It("Should rupture pipe on extreme overpressure exceeding safe limit, dump fluid, and grant Ruptured tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		PumpComp->SetupNode(ESBPipeNodeType::SourcePump, ESBFluidType::Fuel, 1000.0f, 20.0f);
		FragilePipeComp->SetupNode(ESBPipeNodeType::PipeSegment, ESBFluidType::Fuel, 100.0f, 2.0f);

		FragilePipeComp->ConnectPipe(PumpComp);
		PumpComp->SetPumpActive(true);

		TestEqual("Fragile pipe state is Ruptured", (int32)FragilePipeComp->GetFlowState(), (int32)ESBPipeFlowState::Ruptured);
		TestTrue("Fragile pipe has Ruptured tag", FragilePipeStateComp->HasTag(Tags.State_Fluid_Ruptured));
	});

	It("Should inject fluid into tank, retain stored volume, and extract fluid on demand", [this]()
	{
		TankComp->SetupNode(ESBPipeNodeType::FluidTank, ESBFluidType::None, 5000.0f, 20.0f);

		float Injected = TankComp->InjectFluid(ESBFluidType::CrudeOil, 2500.0f);
		TestEqual("Injected 2500L", Injected, 2500.0f);
		TestEqual("Current volume is 2500L", TankComp->GetNodeData().FluidAmount, 2500.0f);
		TestEqual("Fluid type is CrudeOil", (int32)TankComp->GetNodeData().FluidType, (int32)ESBFluidType::CrudeOil);

		float Extracted = TankComp->ExtractFluid(1000.0f);
		TestEqual("Extracted 1000L", Extracted, 1000.0f);
		TestEqual("Remaining volume is 1500L", TankComp->GetNodeData().FluidAmount, 1500.0f);
	});
}
