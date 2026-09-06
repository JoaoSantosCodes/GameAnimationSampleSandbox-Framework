#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBZiplineComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBZiplineTestsSpec, "Sandbox.Character.ZiplineAndCableSliding", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* CharacterActor;
	USBZiplineComponent* ZiplineComp;
	USBStateComponent* StateComp;
END_DEFINE_SPEC(FSBZiplineTestsSpec)

void FSBZiplineTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CharacterActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Root = NewObject<USceneComponent>(CharacterActor, TEXT("Root"));
		CharacterActor->SetRootComponent(Root);
		Root->RegisterComponent();

		StateComp = NewObject<USBStateComponent>(CharacterActor, TEXT("StateComp"));
		CharacterActor->AddOwnedComponent(StateComp);

		ZiplineComp = NewObject<USBZiplineComponent>(CharacterActor, TEXT("ZiplineComp"));
		CharacterActor->AddOwnedComponent(ZiplineComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(ZiplineComp);
	});

	AfterEach([this]()
	{
		if (CharacterActor)
		{
			CharacterActor->Destroy();
			CharacterActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should attach to zipline, initialize ride data, and apply Ziplining state tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		bool bStateChangedFired = false;
		ESBZiplineState FiredState = ESBZiplineState::None;
		ZiplineComp->OnZiplineStateChanged.AddLambda([&bStateChangedFired, &FiredState](ESBZiplineState NewState)
		{
			bStateChangedFired = true;
			FiredState = NewState;
		});

		const FVector Start(0.0f, 0.0f, 1000.0f);
		const FVector End(1000.0f, 0.0f, 0.0f);
		bool bAttached = ZiplineComp->AttachToZipline(Start, End);

		TestTrue("Attach succeeded", bAttached);
		TestTrue("Is riding", ZiplineComp->IsRiding());
		TestEqual("State is Sliding", (int32)ZiplineComp->GetZiplineState(), (int32)ESBZiplineState::Sliding);
		TestTrue("Has Ziplining tag", StateComp->HasTag(Tags.State_Movement_Ziplining));
		TestTrue("Has Sliding tag", StateComp->HasTag(Tags.State_Movement_Ziplining_Sliding));
		TestTrue("Delegate fired", bStateChangedFired);
		TestEqual("Delegate state is Sliding", (int32)FiredState, (int32)ESBZiplineState::Sliding);
	});

	It("Should update zipline travel progress and broadcast progress alpha", [this]()
	{
		const FVector Start(0.0f, 0.0f, 0.0f);
		const FVector End(2000.0f, 0.0f, 0.0f);
		ZiplineComp->AttachToZipline(Start, End);

		bool bProgressFired = false;
		float ProgressAlpha = 0.0f;
		ZiplineComp->OnZiplineProgress.AddLambda([&bProgressFired, &ProgressAlpha](float Alpha)
		{
			bProgressFired = true;
			ProgressAlpha = Alpha;
		});

		ZiplineComp->UpdateZiplineTravel(0.5f); // 0.5s at 800 uu/s = 400 uu traveled (20% of 2000)

		TestTrue("Progress delegate fired", bProgressFired);
		TestEqual("Distance incremented", ZiplineComp->GetRideData().CurrentDistance, 400.0f);
		TestNearlyEqual("Progress alpha is approx 0.2", ProgressAlpha, 0.2f, 0.01f);
	});

	It("Should accelerate speed on downward sloping ziplines", [this]()
	{
		const FVector Start(0.0f, 0.0f, 1000.0f);
		const FVector End(1000.0f, 0.0f, 0.0f);
		ZiplineComp->AttachToZipline(Start, End);

		const float InitialSpeed = ZiplineComp->GetRideData().CurrentSpeed;
		ZiplineComp->UpdateZiplineTravel(0.5f);

		TestTrue("Speed accelerated due to downward gravity", ZiplineComp->GetRideData().CurrentSpeed > InitialSpeed);
	});

	It("Should detach cleanly on end of cable or manual dismount with launch impulse and clear tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		const FVector Start(0.0f, 0.0f, 0.0f);
		const FVector End(1000.0f, 0.0f, 0.0f);
		ZiplineComp->AttachToZipline(Start, End);

		bool bDismountFired = false;
		FVector ExitVel = FVector::ZeroVector;
		ZiplineComp->OnZiplineDismounted.AddLambda([&bDismountFired, &ExitVel](const FVector& Vel)
		{
			bDismountFired = true;
			ExitVel = Vel;
		});

		bool bDetached = ZiplineComp->DetachFromZipline(true);

		TestTrue("Detach succeeded", bDetached);
		TestFalse("Not riding after detach", ZiplineComp->IsRiding());
		TestEqual("State is None", (int32)ZiplineComp->GetZiplineState(), (int32)ESBZiplineState::None);
		TestFalse("Ziplining tag removed", StateComp->HasTag(Tags.State_Movement_Ziplining));
		TestFalse("Sliding tag removed", StateComp->HasTag(Tags.State_Movement_Ziplining_Sliding));
		TestTrue("Dismount delegate fired", bDismountFired);
		TestTrue("Exit velocity is forward in cable direction", ExitVel.X > 0.0f);
	});
}
