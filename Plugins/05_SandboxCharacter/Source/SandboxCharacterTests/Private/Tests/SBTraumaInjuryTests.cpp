// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBTraumaInjuryComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"
#include "SBCharacterTestTypes.h"

BEGIN_DEFINE_SPEC(FSBTraumaInjuryTestsSpec, "Sandbox.Character.TraumaInjury", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* CharacterActor;
	USBTraumaInjuryComponent* TraumaComp;
	USBStateComponent* CharacterStateComp;
END_DEFINE_SPEC(FSBTraumaInjuryTestsSpec)

void FSBTraumaInjuryTestsSpec::Define()
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

		CharacterStateComp = NewObject<USBStateComponent>(CharacterActor, TEXT("CharacterStateComp"));
		CharacterActor->AddOwnedComponent(CharacterStateComp);

		TraumaComp = NewObject<USBTraumaInjuryComponent>(CharacterActor, TEXT("TraumaComp"));
		CharacterActor->AddOwnedComponent(TraumaComp);

		ISBComponentInterface::Execute_OnInitialize(CharacterStateComp);
		ISBComponentInterface::Execute_OnInitialize(TraumaComp);
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

	It("Should inflict blunt damage causing arm bone fracture, apply Fracture Arm tag, and stabilize with splint", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		TraumaComp->SetupTraumaSystem(5.0f);

		USBCharacterTestListener* FractureListener = NewObject<USBCharacterTestListener>(TraumaComp);
		TraumaComp->OnLimbFractured.AddDynamic(FractureListener, &USBCharacterTestListener::OnLimb);

		TraumaComp->InflictLimbDamage(ESBBodyLimb::RightArm, 40.0f, true, ESBBleedType::None);

		TestTrue("Fracture delegate fired for right arm", FractureListener->bFired && FractureListener->LimbArg == ESBBodyLimb::RightArm);
		TestTrue("Right arm is fractured", TraumaComp->IsLimbFractured(ESBBodyLimb::RightArm));
		TestTrue("Has Fracture Arm tag", CharacterStateComp->HasTag(Tags.State_Trauma_Fracture_Arm));

		bool bSplintApplied = TraumaComp->ApplySplint(ESBBodyLimb::RightArm);
		TestTrue("Splint successfully applied", bSplintApplied);
		TestTrue("Limb is splinted", TraumaComp->GetLimbData(ESBBodyLimb::RightArm).bIsSplinted);
	});

	It("Should inflict deep slashing damage causing leg arterial hemorrhage and apply ArterialBleed and Bleeding tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		TraumaComp->SetupTraumaSystem(5.0f);

		USBCharacterTestListener* BleedListener = NewObject<USBCharacterTestListener>(TraumaComp);
		TraumaComp->OnBleedStateChanged.AddDynamic(BleedListener, &USBCharacterTestListener::OnLimbBleed);

		TraumaComp->InflictLimbDamage(ESBBodyLimb::LeftLeg, 30.0f, false, ESBBleedType::Arterial);

		TestTrue("Bleed state delegate fired for left leg arterial", BleedListener->bFired && BleedListener->LimbArg == ESBBodyLimb::LeftLeg && BleedListener->BleedArg == ESBBleedType::Arterial);
		TestTrue("Is bleeding", TraumaComp->IsBleeding());
		TestTrue("Is arterial bleeding", TraumaComp->IsArterialBleeding());
		TestTrue("Has Bleeding tag", CharacterStateComp->HasTag(Tags.State_Trauma_Bleeding));
		TestTrue("Has ArterialBleed tag", CharacterStateComp->HasTag(Tags.State_Trauma_ArterialBleed));
	});

	It("Should drain blood volume from continuous arterial bleed below 3.5L and trigger Hypovolemic Shock", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		TraumaComp->SetupTraumaSystem(4.0f);
		TraumaComp->InflictLimbDamage(ESBBodyLimb::LeftLeg, 20.0f, false, ESBBleedType::Arterial);

		USBCharacterTestListener* ShockListener = NewObject<USBCharacterTestListener>(TraumaComp);
		TraumaComp->OnHypovolemicShockTriggered.AddDynamic(ShockListener, &USBCharacterTestListener::OnVoid);

		TraumaComp->SimulateTraumaTick(2.0f); // 4.0 - (0.3 * 2.0 = 0.6) = 3.4L

		TestTrue("Hypovolemic shock delegate fired", ShockListener->bFired);
		TestTrue("In hypovolemic shock state", TraumaComp->GetTraumaData().bInHypovolemicShock);
		TestTrue("Has HypovolemicShock tag", CharacterStateComp->HasTag(Tags.State_Trauma_HypovolemicShock));
	});

	It("Should apply tourniquet to halt arterial bleeding and transfuse blood restoring volume and resolving shock", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		TraumaComp->SetupTraumaSystem(3.2f);
		TraumaComp->InflictLimbDamage(ESBBodyLimb::LeftLeg, 10.0f, false, ESBBleedType::Arterial);
		TraumaComp->SimulateTraumaTick(0.1f);

		TestTrue("Has HypovolemicShock tag initially", CharacterStateComp->HasTag(Tags.State_Trauma_HypovolemicShock));

		USBCharacterTestListener* TourniquetListener = NewObject<USBCharacterTestListener>(TraumaComp);
		TraumaComp->OnTourniquetStateChanged.AddDynamic(TourniquetListener, &USBCharacterTestListener::OnLimbBool);

		bool bApplied = TraumaComp->ApplyTourniquet(ESBBodyLimb::LeftLeg);
		TestTrue("Tourniquet applied successfully", bApplied);
		TestTrue("Tourniquet delegate fired for left leg applied", TourniquetListener->bFired && TourniquetListener->LimbArg == ESBBodyLimb::LeftLeg && TourniquetListener->BoolArg);
		TestTrue("Has TourniquetApplied tag", CharacterStateComp->HasTag(Tags.State_Trauma_TourniquetApplied));
		TestFalse("Arterial bleeding stopped by tourniquet", TraumaComp->IsArterialBleeding());

		USBCharacterTestListener* RecoveryListener = NewObject<USBCharacterTestListener>(TraumaComp);
		TraumaComp->OnHypovolemicShockRecovered.AddDynamic(RecoveryListener, &USBCharacterTestListener::OnVoid);

		TraumaComp->TransfuseBlood(1.0f);

		TestTrue("Shock recovery delegate fired", RecoveryListener->bFired);
		TestFalse("Hypovolemic shock tag cleared", CharacterStateComp->HasTag(Tags.State_Trauma_HypovolemicShock));
	});
}
