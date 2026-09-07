// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBDismembermentComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBDismembermentTestsSpec, "Sandbox.Combat.Dismemberment", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* CharacterActor;
	USBDismembermentComponent* DismemberComp;
	USBStateComponent* StateComp;
END_DEFINE_SPEC(FSBDismembermentTestsSpec)

void FSBDismembermentTestsSpec::Define()
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
		StateComp->RegisterComponent();
		CharacterActor->AddOwnedComponent(StateComp);

		DismemberComp = NewObject<USBDismembermentComponent>(CharacterActor, TEXT("DismemberComp"));
		DismemberComp->RegisterComponent();
		CharacterActor->AddOwnedComponent(DismemberComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(DismemberComp);

		// Setup default limbs
		FSBLimbDismemberDefinition ArmDef;
		ArmDef.LimbType = ESBLimbType::LeftArm;
		ArmDef.BoneName = FName("upperarm_l");
		ArmDef.SocketName = FName("arm_l_socket");
		DismemberComp->RegisterLimbDefinition(ArmDef);

		FSBLimbDismemberDefinition HeadDef;
		HeadDef.LimbType = ESBLimbType::Head;
		HeadDef.BoneName = FName("head");
		HeadDef.SocketName = FName("neck_socket");
		DismemberComp->RegisterLimbDefinition(HeadDef);

		FSBLimbDismemberDefinition LegDef;
		LegDef.LimbType = ESBLimbType::RightLeg;
		LegDef.BoneName = FName("thigh_r");
		LegDef.SocketName = FName("leg_r_socket");
		DismemberComp->RegisterLimbDefinition(LegDef);
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

	It("Should register limb definition and sever limb with tags and delegate", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		bool bSeveredCalled = false;
		FVector SeveredImpulse = FVector::ZeroVector;
		DismemberComp->OnLimbSevered.AddLambda([&bSeveredCalled, &SeveredImpulse](ESBLimbType Limb, const FVector& Impulse)
		{
			if (Limb == ESBLimbType::LeftArm)
			{
				bSeveredCalled = true;
				SeveredImpulse = Impulse;
			}
		});

		FSBSeverLimbRequest Req;
		Req.LimbType = ESBLimbType::LeftArm;
		Req.ImpulseDirection = FVector(0.0f, 1.0f, 0.0f);
		Req.ImpulseStrength = 600.0f;

		bool bSuccess = DismemberComp->SeverLimb(Req);

		TestTrue("SeverLimb succeeded", bSuccess);
		TestTrue("LeftArm is severed", DismemberComp->IsLimbSevered(ESBLimbType::LeftArm));
		TestTrue("Has Dismembered state tag", StateComp->HasTag(Tags.State_Combat_Dismembered));
		TestTrue("Has Dismember_Arm tag", StateComp->HasTag(Tags.Combat_Dismember_Arm));
		TestTrue("Delegate fired", bSeveredCalled);
		TestEqual("Impulse calculated correctly", SeveredImpulse, FVector(0.0f, 600.0f, 0.0f));
	});

	It("Should reject severing already severed limb", [this]()
	{
		FSBSeverLimbRequest Req;
		Req.LimbType = ESBLimbType::LeftArm;

		TestTrue("First sever succeeds", DismemberComp->SeverLimb(Req));
		TestFalse("Second sever on same limb fails", DismemberComp->SeverLimb(Req));
	});

	It("Should track multiple severed limbs and count", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBSeverLimbRequest ReqArm;
		ReqArm.LimbType = ESBLimbType::LeftArm;
		DismemberComp->SeverLimb(ReqArm);

		FSBSeverLimbRequest ReqHead;
		ReqHead.LimbType = ESBLimbType::Head;
		DismemberComp->SeverLimb(ReqHead);

		FSBSeverLimbRequest ReqLeg;
		ReqLeg.LimbType = ESBLimbType::RightLeg;
		DismemberComp->SeverLimb(ReqLeg);

		TestEqual("Severed limb count is 3", DismemberComp->GetSeveredLimbCount(), 3);
		TestTrue("Head is severed", DismemberComp->IsLimbSevered(ESBLimbType::Head));
		TestTrue("Leg is severed", DismemberComp->IsLimbSevered(ESBLimbType::RightLeg));
		TestTrue("Has Head tag", StateComp->HasTag(Tags.Combat_Dismember_Head));
		TestTrue("Has Leg tag", StateComp->HasTag(Tags.Combat_Dismember_Leg));
	});

	It("Should reset dismemberment and clear state tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBSeverLimbRequest Req;
		Req.LimbType = ESBLimbType::Head;
		DismemberComp->SeverLimb(Req);

		TestTrue("Head severed before reset", DismemberComp->IsLimbSevered(ESBLimbType::Head));

		DismemberComp->ResetDismemberment();

		TestEqual("Severed count is 0 after reset", DismemberComp->GetSeveredLimbCount(), 0);
		TestFalse("Head no longer severed", DismemberComp->IsLimbSevered(ESBLimbType::Head));
		TestFalse("Dismembered state tag removed", StateComp->HasTag(Tags.State_Combat_Dismembered));
		TestFalse("Head tag removed", StateComp->HasTag(Tags.Combat_Dismember_Head));
	});
}
