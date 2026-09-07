// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/SBCosmeticSaturationSubsystem.h"
#include "Sound/SoundWave.h"
#include "Components/SceneComponent.h"

BEGIN_DEFINE_SPEC(FSBCosmeticLimiterTestsSpec, "Sandbox.Cosmetics.Limiter", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	USBCosmeticSaturationSubsystem* Subsystem;
	USoundWave* TestSoundA;
	USoundWave* TestSoundB;
	USceneComponent* TestEffectA;
END_DEFINE_SPEC(FSBCosmeticLimiterTestsSpec)

void FSBCosmeticLimiterTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		Subsystem = TestWorld->GetSubsystem<USBCosmeticSaturationSubsystem>();

		TestSoundA = NewObject<USoundWave>(TestWorld, TEXT("DummySoundA"));
		TestSoundB = NewObject<USoundWave>(TestWorld, TEXT("DummySoundB"));
		TestEffectA = NewObject<USceneComponent>(TestWorld, TEXT("DummyEffectA"));
	});

	AfterEach([this]()
	{
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
		}
	});

	Describe("Audio and Cosmetic Saturation Protection", [this]()
	{
		It("Should allow the first play request of a sound at a location", [this]()
		{
			FVector Location(100.0f, 200.0f, 300.0f);
			bool bAllowed = Subsystem->AllowSound(TestSoundA, Location, 0.1f);
			TestTrue("Primeira chamada do som deve ser permitida", bAllowed);
		});

		It("Should suppress consecutive play requests of the same sound at the same location within MinInterval", [this]()
		{
			FVector Location(100.0f, 200.0f, 300.0f);
			
			// Primeira chamada
			bool bAllowed1 = Subsystem->AllowSound(TestSoundA, Location, 0.1f);
			TestTrue("Primeira chamada permitida", bAllowed1);

			// Segunda chamada imediata (mesmo frame/tempo)
			bool bAllowed2 = Subsystem->AllowSound(TestSoundA, Location, 0.1f);
			TestFalse("Segunda chamada imediata deve ser suprimida", bAllowed2);
		});

		It("Should allow play requests of the same sound at different spatial grid locations", [this]()
		{
			FVector LocationA(100.0f, 200.0f, 300.0f);
			FVector LocationB(1000.0f, 2000.0f, 3000.0f); // Longe o suficiente para cair em outra célula

			bool bAllowedA = Subsystem->AllowSound(TestSoundA, LocationA, 0.1f);
			bool bAllowedB = Subsystem->AllowSound(TestSoundA, LocationB, 0.1f);

			TestTrue("Som na Localizacao A permitido", bAllowedA);
			TestTrue("Som na Localizacao B permitido no mesmo instante", bAllowedB);
		});

		It("Should allow play requests of different sounds at the same location", [this]()
		{
			FVector Location(100.0f, 200.0f, 300.0f);

			bool bAllowedSoundA = Subsystem->AllowSound(TestSoundA, Location, 0.1f);
			bool bAllowedSoundB = Subsystem->AllowSound(TestSoundB, Location, 0.1f);

			TestTrue("Som A permitido", bAllowedSoundA);
			TestTrue("Som B diferente permitido no mesmo local", bAllowedSoundB);
		});

		It("Should allow playing visual effects with the same logic", [this]()
		{
			FVector Location(150.0f, 250.0f, 350.0f);

			bool bAllowed1 = Subsystem->AllowEffect(TestEffectA, Location, 0.1f);
			bool bAllowed2 = Subsystem->AllowEffect(TestEffectA, Location, 0.1f);

			TestTrue("Efeito visual na primeira chamada permitido", bAllowed1);
			TestFalse("Efeito visual na segunda chamada imediata deve ser suprimido", bAllowed2);
		});

		It("Should allow play requests of the same sound at the same location after the interval has elapsed", [this]()
		{
			FVector Location(100.0f, 200.0f, 300.0f);

			bool bAllowed1 = Subsystem->AllowSound(TestSoundA, Location, 0.1f);
			TestTrue("Primeira chamada permitida", bAllowed1);

			// Segunda chamada com MinInterval = -0.1f para simular passagem de tempo
			bool bAllowed2 = Subsystem->AllowSound(TestSoundA, Location, -0.1f);
			TestTrue("Chamada permitida após expiração do cooldown", bAllowed2);
		});
	});
}
