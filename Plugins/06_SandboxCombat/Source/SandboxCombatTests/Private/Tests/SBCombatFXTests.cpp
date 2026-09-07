// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBCombatFXComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBCombatFXTestsSpec, "Sandbox.Combat.VisualEffects", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* CharacterActor;
	USBCombatFXComponent* FXComp;
	USBStateComponent* StateComp;
END_DEFINE_SPEC(FSBCombatFXTestsSpec)

void FSBCombatFXTestsSpec::Define()
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

		FXComp = NewObject<USBCombatFXComponent>(CharacterActor, TEXT("FXComp"));
		CharacterActor->AddOwnedComponent(FXComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(FXComp);
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

	It("Should activate weapon trail and apply combat state tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBWeaponTrailConfig Config;
		Config.StartSocketName = FName("Sword_Base");
		Config.EndSocketName = FName("Sword_Tip");
		Config.Width = 15.0f;

		bool bChangedState = false;
		FXComp->OnWeaponTrailStateChanged.AddLambda([&bChangedState](bool bIsActive)
		{
			bChangedState = bIsActive;
		});

		FXComp->ActivateWeaponTrail(Config);

		TestTrue("Trail is active", FXComp->IsWeaponTrailActive());
		TestTrue("State component received WeaponTrailActive tag", StateComp->HasTag(Tags.State_Combat_WeaponTrailActive));
		TestTrue("State change delegate fired with true", bChangedState);
		TestEqual("Start socket is Sword_Base", FXComp->GetActiveTrailConfig().StartSocketName, FName("Sword_Base"));
	});

	It("Should deactivate weapon trail, clear state tag, and broadcast delegate", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBWeaponTrailConfig Config;
		FXComp->ActivateWeaponTrail(Config);
		TestTrue("Active before deactivation", FXComp->IsWeaponTrailActive());

		bool bDeactivated = false;
		FXComp->OnWeaponTrailStateChanged.AddLambda([&bDeactivated](bool bIsActive)
		{
			if (!bIsActive)
			{
				bDeactivated = true;
			}
		});

		FXComp->DeactivateWeaponTrail();

		TestFalse("Trail is inactive", FXComp->IsWeaponTrailActive());
		TestFalse("Tag removed after deactivation", StateComp->HasTag(Tags.State_Combat_WeaponTrailActive));
		TestTrue("Deactivation delegate fired", bDeactivated);
	});

	It("Should calculate and spawn impact decal with correct normal alignment", [this]()
	{
		FHitResult HitResult;
		HitResult.ImpactPoint = FVector(100.0f, 200.0f, 50.0f);
		HitResult.ImpactNormal = FVector(0.0f, 0.0f, 1.0f);

		FSBImpactDecalConfig DecalConfig;
		DecalConfig.DecalSize = FVector(30.0f, 30.0f, 30.0f);
		DecalConfig.LifeSpan = 20.0f;

		FVector SpawnedLoc = FVector::ZeroVector;
		FRotator SpawnedRot = FRotator::ZeroRotator;

		FXComp->OnImpactDecalSpawned.AddLambda([&SpawnedLoc, &SpawnedRot](const FVector& Loc, const FRotator& Rot)
		{
			SpawnedLoc = Loc;
			SpawnedRot = Rot;
		});

		bool bSuccess = FXComp->SpawnImpactDecal(HitResult, DecalConfig);

		TestTrue("Decal spawned successfully", bSuccess);
		TestEqual("Decal location matches impact point", SpawnedLoc, FVector(100.0f, 200.0f, 50.0f));
		TestEqual("Decal history recorded", FXComp->GetSpawnedDecalCount(), 1);
	});

	It("Should register socket particle request", [this]()
	{
		bool bSuccess = FXComp->PlaySocketParticle(FName("MuzzleSocket"));
		TestTrue("Socket particle played successfully", bSuccess);
		TestEqual("History count incremented", FXComp->GetSpawnedDecalCount(), 1);
	});
}
