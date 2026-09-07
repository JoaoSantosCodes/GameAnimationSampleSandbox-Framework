#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBMachineryComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBMachineryTestsSpec, "Sandbox.Character.MachineryAndHydraulics", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* MachineryActor;
	AActor* OperatorActor;
	USBMachineryComponent* MachineryComp;
	USBStateComponent* MachineryStateComp;
	USBStateComponent* OperatorStateComp;
END_DEFINE_SPEC(FSBMachineryTestsSpec)

void FSBMachineryTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		MachineryActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* MachRoot = NewObject<USceneComponent>(MachineryActor, TEXT("MachRoot"));
		MachineryActor->SetRootComponent(MachRoot);
		MachRoot->RegisterComponent();

		MachineryStateComp = NewObject<USBStateComponent>(MachineryActor, TEXT("MachineryStateComp"));
		MachineryActor->AddOwnedComponent(MachineryStateComp);

		MachineryComp = NewObject<USBMachineryComponent>(MachineryActor, TEXT("MachineryComp"));
		MachineryActor->AddOwnedComponent(MachineryComp);

		OperatorActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* OpRoot = NewObject<USceneComponent>(OperatorActor, TEXT("OpRoot"));
		OperatorActor->SetRootComponent(OpRoot);
		OpRoot->RegisterComponent();

		OperatorStateComp = NewObject<USBStateComponent>(OperatorActor, TEXT("OperatorStateComp"));
		OperatorActor->AddOwnedComponent(OperatorStateComp);

		ISBComponentInterface::Execute_OnInitialize(MachineryStateComp);
		ISBComponentInterface::Execute_OnInitialize(MachineryComp);
		ISBComponentInterface::Execute_OnInitialize(OperatorStateComp);
	});

	AfterEach([this]()
	{
		if (OperatorActor)
		{
			OperatorActor->Destroy();
			OperatorActor = nullptr;
		}

		if (MachineryActor)
		{
			MachineryActor->Destroy();
			MachineryActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should enter machinery as operator, start pump, build pressure, and grant machinery tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		bool bOpFired = false;
		MachineryComp->OnMachineryOperatorChanged.AddLambda([&bOpFired](AActor* Op)
		{
			bOpFired = true;
		});

		bool bEntered = MachineryComp->EnterMachinery(OperatorActor);

		TestTrue("Enter succeeded", bEntered);
		TestTrue("Is operator", MachineryComp->IsOperator(OperatorActor));
		TestEqual("State is Idling", (int32)MachineryComp->GetMachineryState(), (int32)ESBMachineryState::Idling);
		TestTrue("Machine has Machinery tag", MachineryStateComp->HasTag(Tags.State_Vehicle_Machinery));
		TestTrue("Operator has Machinery tag", OperatorStateComp->HasTag(Tags.State_Movement_Machinery));
		TestTrue("Operator delegate fired", bOpFired);
	});

	It("Should articulate boom and arm under pressure, set Operating state, and grant Operating tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		MachineryComp->EnterMachinery(OperatorActor);
		MachineryComp->UpdateHydraulicPhysics(1.0f);
		MachineryComp->SetBoomInput(1.0f);
		MachineryComp->UpdateHydraulicPhysics(1.0f);

		TestTrue("Boom rotated", MachineryComp->GetHydraulicData().BoomAngle > 0.0f);
		TestEqual("State is Operating", (int32)MachineryComp->GetMachineryState(), (int32)ESBMachineryState::Operating);
		TestTrue("Operator has Operating tag", OperatorStateComp->HasTag(Tags.State_Movement_Machinery_Operating));
	});

	It("Should attach payload within capacity, set Lifting state, and grant Lifting tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		MachineryComp->EnterMachinery(OperatorActor);
		bool bAttached = MachineryComp->AttachPayload(5000.0f);

		TestTrue("Payload attached", bAttached);
		TestEqual("State is Lifting", (int32)MachineryComp->GetMachineryState(), (int32)ESBMachineryState::Lifting);
		TestTrue("Operator has Lifting tag", OperatorStateComp->HasTag(Tags.State_Movement_Machinery_Lifting));

		MachineryComp->DetachPayload();
		TestEqual("State is Idling after detach", (int32)MachineryComp->GetMachineryState(), (int32)ESBMachineryState::Idling);
		TestFalse("Operator Lifting tag removed", OperatorStateComp->HasTag(Tags.State_Movement_Machinery_Lifting));
	});

	It("Should trigger excavation cycle, grant Excavating tag, exit machinery, and clear all tags cleanly", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		MachineryComp->EnterMachinery(OperatorActor);
		MachineryComp->UpdateHydraulicPhysics(1.5f);

		bool bExcavating = MachineryComp->TriggerExcavateAction();
		TestTrue("Excavation triggered", bExcavating);
		TestEqual("State is Excavating", (int32)MachineryComp->GetMachineryState(), (int32)ESBMachineryState::Excavating);
		TestTrue("Operator has Excavating tag", OperatorStateComp->HasTag(Tags.State_Movement_Machinery_Excavating));

		bool bExited = MachineryComp->ExitMachinery(OperatorActor);

		TestTrue("Exit succeeded", bExited);
		TestFalse("No longer operator", MachineryComp->IsOperator(OperatorActor));
		TestEqual("State is Parked", (int32)MachineryComp->GetMachineryState(), (int32)ESBMachineryState::Parked);
		TestFalse("Operator Machinery tag removed", OperatorStateComp->HasTag(Tags.State_Movement_Machinery));
		TestFalse("Operator Operating tag removed", OperatorStateComp->HasTag(Tags.State_Movement_Machinery_Operating));
		TestFalse("Operator Lifting tag removed", OperatorStateComp->HasTag(Tags.State_Movement_Machinery_Lifting));
		TestFalse("Operator Excavating tag removed", OperatorStateComp->HasTag(Tags.State_Movement_Machinery_Excavating));
	});
}
