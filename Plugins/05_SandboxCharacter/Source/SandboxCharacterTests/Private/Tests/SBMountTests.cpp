#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBMountComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBMountTestsSpec, "Sandbox.Character.MountsAndRiding", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* MountActor;
	AActor* RiderActor;
	USBMountComponent* MountComp;
	USBStateComponent* MountStateComp;
	USBStateComponent* RiderStateComp;
END_DEFINE_SPEC(FSBMountTestsSpec)

void FSBMountTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Mount Actor
		MountActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* MountRoot = NewObject<USceneComponent>(MountActor, TEXT("MountRoot"));
		MountActor->SetRootComponent(MountRoot);
		MountRoot->RegisterComponent();

		MountStateComp = NewObject<USBStateComponent>(MountActor, TEXT("MountStateComp"));
		MountActor->AddOwnedComponent(MountStateComp);

		MountComp = NewObject<USBMountComponent>(MountActor, TEXT("MountComp"));
		MountActor->AddOwnedComponent(MountComp);

		ISBComponentInterface::Execute_OnInitialize(MountStateComp);
		ISBComponentInterface::Execute_OnInitialize(MountComp);

		// Rider Actor
		RiderActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(50.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* RiderRoot = NewObject<USceneComponent>(RiderActor, TEXT("RiderRoot"));
		RiderActor->SetRootComponent(RiderRoot);
		RiderRoot->RegisterComponent();
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		RiderActor->SetActorLocation(FVector(50.0f, 0.0f, 0.0f));

		RiderStateComp = NewObject<USBStateComponent>(RiderActor, TEXT("RiderStateComp"));
		RiderActor->AddOwnedComponent(RiderStateComp);

		ISBComponentInterface::Execute_OnInitialize(RiderStateComp);
	});

	AfterEach([this]()
	{
		if (MountActor)
		{
			MountActor->Destroy();
			MountActor = nullptr;
		}

		if (RiderActor)
		{
			RiderActor->Destroy();
			RiderActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should mount rider, apply state tags, and broadcast delegate", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		bool bStateChangedFired = false;
		ESBMountState FiredState = ESBMountState::Unmounted;
		AActor* FiredRider = nullptr;

		MountComp->OnMountStateChanged.AddLambda([&bStateChangedFired, &FiredState, &FiredRider](ESBMountState NewState, AActor* InRider)
		{
			bStateChangedFired = true;
			FiredState = NewState;
			FiredRider = InRider;
		});

		bool bMounted = MountComp->Mount(RiderActor);

		TestTrue("Mount succeeded", bMounted);
		TestTrue("IsMounted is true", MountComp->IsMounted());
		TestEqual("Mount state is Mounted", (int32)MountComp->GetMountState(), (int32)ESBMountState::Mounted);
		TestEqual("Rider matches", MountComp->GetRider(), RiderActor);
		TestTrue("Mount actor has Mounted tag", MountStateComp->HasTag(Tags.State_Movement_Mounted));
		TestTrue("Rider actor has Mounted tag", RiderStateComp->HasTag(Tags.State_Movement_Mounted));
		TestTrue("Delegate fired", bStateChangedFired);
		TestEqual("Delegate state is Mounted", (int32)FiredState, (int32)ESBMountState::Mounted);
		TestEqual("Delegate rider matches", FiredRider, RiderActor);
	});

	It("Should reject mounting when already mounted by a rider", [this]()
	{
		MountComp->Mount(RiderActor);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* SecondRider = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		USceneComponent* SecondRiderRoot = NewObject<USceneComponent>(SecondRider, TEXT("SecondRiderRoot"));
		SecondRider->SetRootComponent(SecondRiderRoot);
		SecondRiderRoot->RegisterComponent();
		SecondRider->SetActorLocation(FVector(100.0f, 0.0f, 0.0f));

		bool bSecondMount = MountComp->Mount(SecondRider);

		TestFalse("Second mount attempt rejected", bSecondMount);
		TestEqual("Rider is still first rider", MountComp->GetRider(), RiderActor);

		SecondRider->Destroy();
	});

	It("Should change gait to gallop and apply Galloping state tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		MountComp->Mount(RiderActor);

		bool bGaitFired = false;
		ESBMountGait FiredGait = ESBMountGait::Walk;
		MountComp->OnMountGaitChanged.AddLambda([&bGaitFired, &FiredGait](ESBMountGait NewGait)
		{
			bGaitFired = true;
			FiredGait = NewGait;
		});

		bool bGaitChanged = MountComp->SetGait(ESBMountGait::Gallop);

		TestTrue("SetGait succeeded", bGaitChanged);
		TestEqual("Current gait is Gallop", (int32)MountComp->GetCurrentGait(), (int32)ESBMountGait::Gallop);
		TestTrue("Mount has Galloping tag", MountStateComp->HasTag(Tags.State_Movement_Galloping));
		TestEqual("Gallop speed matches settings", MountComp->GetSpeedForGait(ESBMountGait::Gallop), 1000.0f);
		TestTrue("Gait delegate fired", bGaitFired);
		TestEqual("Gait delegate matches Gallop", (int32)FiredGait, (int32)ESBMountGait::Gallop);

		// Switch back to Trot
		MountComp->SetGait(ESBMountGait::Trot);
		TestFalse("Mount no longer has Galloping tag", MountStateComp->HasTag(Tags.State_Movement_Galloping));
		TestEqual("Trot speed matches settings", MountComp->GetSpeedForGait(ESBMountGait::Trot), 600.0f);
	});

	It("Should dismount cleanly, restore state, and clear tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		MountComp->Mount(RiderActor);
		MountComp->SetGait(ESBMountGait::Gallop);

		TestTrue("Mounted before dismount", MountComp->IsMounted());
		TestTrue("Mount has Mounted tag before dismount", MountStateComp->HasTag(Tags.State_Movement_Mounted));
		TestTrue("Rider has Mounted tag before dismount", RiderStateComp->HasTag(Tags.State_Movement_Mounted));

		bool bDismounted = MountComp->Dismount();

		TestTrue("Dismount succeeded", bDismounted);
		TestFalse("IsMounted is false after dismount", MountComp->IsMounted());
		TestEqual("Mount state is Unmounted", (int32)MountComp->GetMountState(), (int32)ESBMountState::Unmounted);
		TestNull("Rider is null", MountComp->GetRider());
		TestFalse("Mount no longer has Mounted tag", MountStateComp->HasTag(Tags.State_Movement_Mounted));
		TestFalse("Mount no longer has Galloping tag", MountStateComp->HasTag(Tags.State_Movement_Galloping));
		TestFalse("Rider no longer has Mounted tag", RiderStateComp->HasTag(Tags.State_Movement_Mounted));
	});
}
