#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Subsystems/SBRegionSubsystem.h"
#include "Types/SBRegionTypes.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBRegionZoneTestsSpec, "Sandbox.RegionZones", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	USBRegionSubsystem* RegionSubsystem;
	AActor* TestActor;
END_DEFINE_SPEC(FSBRegionZoneTestsSpec)

void FSBRegionZoneTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
		RegionSubsystem = TestWorld->GetSubsystem<USBRegionSubsystem>();

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		TestActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		// Um AActor puro nao tem RootComponent: sem ele, SetActorLocation, SetActorTransform
		// e TeleportTo falham em silencio e o ator fica preso na origem.
		USceneComponent* TestActorRoot = NewObject<USceneComponent>(TestActor, TEXT("TestActorRoot"));
		TestActor->SetRootComponent(TestActorRoot);
		TestActorRoot->RegisterComponent();
	});

	AfterEach([this]()
	{
		if (TestActor)
		{
			TestActor->Destroy();
			TestActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should apply SafeZone tracking and report IsActorInSafeZone on actor entry and exit", [this]()
	{
		FSBRegionData SafeZone;
		SafeZone.RegionTag = FSBGameplayTags::Get().Zone_Type_SafeZone;
		SafeZone.RegionDisplayName = FText::FromString(TEXT("Town Square"));
		SafeZone.bIsSafeZone = true;
		SafeZone.bIsPvPAllowed = false;

		TestFalse("Actor should not be in safe zone initially", RegionSubsystem->IsActorInSafeZone(TestActor));

		// 1. Entra na SafeZone
		RegionSubsystem->NotifyActorEnteredRegion(TestActor, SafeZone);

		TestTrue("Actor should be in safe zone", RegionSubsystem->IsActorInSafeZone(TestActor));

		FSBRegionData CurrentRegion;
		TestTrue("GetActorCurrentRegion should succeed", RegionSubsystem->GetActorCurrentRegion(TestActor, CurrentRegion));
		TestEqual("Region tag should match", CurrentRegion.RegionTag, SafeZone.RegionTag);

		// 2. Sai da SafeZone
		RegionSubsystem->NotifyActorExitedRegion(TestActor, SafeZone);

		TestFalse("Actor should no longer be in safe zone", RegionSubsystem->IsActorInSafeZone(TestActor));
	});

	It("Should apply PvPAllowed tracking and report IsPvPAllowedForActor on PvP zone entry and exit", [this]()
	{
		FSBRegionData PvPZone;
		PvPZone.RegionTag = FSBGameplayTags::Get().Zone_Type_PvP;
		PvPZone.RegionDisplayName = FText::FromString(TEXT("Wilderness Arena"));
		PvPZone.bIsSafeZone = false;
		PvPZone.bIsPvPAllowed = true;

		TestFalse("Actor should not be in PvP zone initially", RegionSubsystem->IsPvPAllowedForActor(TestActor));

		// 1. Entra na PvPZone
		RegionSubsystem->NotifyActorEnteredRegion(TestActor, PvPZone);

		TestTrue("Actor should be in PvP zone", RegionSubsystem->IsPvPAllowedForActor(TestActor));

		// 2. Sai da PvPZone
		RegionSubsystem->NotifyActorExitedRegion(TestActor, PvPZone);

		TestFalse("Actor should no longer be in PvP zone", RegionSubsystem->IsPvPAllowedForActor(TestActor));
	});

	It("Should apply Hazard tracking and detect hazard regions", [this]()
	{
		FSBRegionData HazardZone;
		HazardZone.RegionTag = FSBGameplayTags::Get().Zone_Type_Hazard;
		HazardZone.RegionDisplayName = FText::FromString(TEXT("Poison Swamp"));
		HazardZone.EnvironmentalDamagePerSecond = 15.0f;

		TestFalse("Actor should not be in hazard initially", RegionSubsystem->IsActorInHazard(TestActor));

		// 1. Entra na HazardZone
		RegionSubsystem->NotifyActorEnteredRegion(TestActor, HazardZone);

		TestTrue("Actor should be in hazard zone", RegionSubsystem->IsActorInHazard(TestActor));

		// 2. Tick de dano ambiental
		RegionSubsystem->Tick(1.0f);

		// 3. Sai da HazardZone
		RegionSubsystem->NotifyActorExitedRegion(TestActor, HazardZone);

		TestFalse("Actor should no longer be in hazard zone", RegionSubsystem->IsActorInHazard(TestActor));
	});
}
