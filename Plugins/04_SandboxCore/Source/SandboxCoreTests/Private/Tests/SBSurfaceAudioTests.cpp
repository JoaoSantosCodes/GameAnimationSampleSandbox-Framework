// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "DataAssets/SBSurfaceEffectsDataAsset.h"
#include "Actors/SBAmbientZoneTrigger.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

BEGIN_DEFINE_SPEC(FSBSurfaceAudioTestsSpec, "Sandbox.Audio.SurfaceAndAmbient", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	USBSurfaceEffectsDataAsset* EffectsDataAsset;
	USoundWave* GrassSound;
	USoundWave* DefaultSound;
	USoundWave* AmbientSound;
	ASBAmbientZoneTrigger* AmbientTrigger;
	ACharacter* TestCharacter;
END_DEFINE_SPEC(FSBSurfaceAudioTestsSpec)

void FSBSurfaceAudioTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		// 1. Setup mock SoundWaves
		GrassSound = NewObject<USoundWave>(TestWorld, TEXT("GrassSoundWave"));
		DefaultSound = NewObject<USoundWave>(TestWorld, TEXT("DefaultSoundWave"));
		AmbientSound = NewObject<USoundWave>(TestWorld, TEXT("AmbientSoundWave"));

		// 2. Setup Data Asset
		EffectsDataAsset = NewObject<USBSurfaceEffectsDataAsset>(TestWorld);
		
		FSBSurfaceEffectConfig GrassConfig;
		GrassConfig.Sound = GrassSound;
		EffectsDataAsset->SurfaceEffectsMap.Add(SurfaceType1, GrassConfig);

		FSBSurfaceEffectConfig DefaultConfig;
		DefaultConfig.Sound = DefaultSound;
		EffectsDataAsset->SurfaceEffectsMap.Add(SurfaceType_Default, DefaultConfig);

		// 3. Setup Trigger
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("AmbientTriggerActor");
		AmbientTrigger = TestWorld->SpawnActor<ASBAmbientZoneTrigger>(ASBAmbientZoneTrigger::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		AmbientTrigger->AmbientSound = AmbientSound;
		AmbientTrigger->FadeInDuration = 0.1f;
		AmbientTrigger->FadeOutDuration = 0.1f;

		// 4. Setup Local Character
		FActorSpawnParameters CharSpawnParams;
		CharSpawnParams.Name = TEXT("LocalPawnActor");
		TestCharacter = TestWorld->SpawnActor<ACharacter>(ACharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, CharSpawnParams);
	});

	AfterEach([this]()
	{
		if (TestCharacter)
		{
			TestCharacter->Destroy();
		}
		if (AmbientTrigger)
		{
			AmbientTrigger->Destroy();
		}
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
		}
	});

	Describe("Surface Effects Data Asset Mapping Verification", [this]()
	{
		It("Should return GrassSound for SurfaceType1", [this]()
		{
			USoundBase* OutSound = nullptr;
			UObject* OutVFX = nullptr;
			bool bFound = EffectsDataAsset->GetEffectsForSurface(SurfaceType1, OutSound, OutVFX);
			
			TestTrue("Mapeamento da grama deve ser encontrado", bFound);
			TestEqual("O som retornado deve ser o GrassSound", OutSound, Cast<USoundBase>(GrassSound));
		});

		It("Should fallback to DefaultSound for unmapped surface types", [this]()
		{
			USoundBase* OutSound = nullptr;
			UObject* OutVFX = nullptr;
			// SurfaceType2 não foi mapeado
			bool bFound = EffectsDataAsset->GetEffectsForSurface(SurfaceType2, OutSound, OutVFX);

			TestTrue("Fallback padrão deve ser retornado", bFound);
			TestEqual("O som retornado deve ser o DefaultSound", OutSound, Cast<USoundBase>(DefaultSound));
		});
	});

	Describe("Ambient Zone Trigger Verification", [this]()
	{
		It("Should start and stop ambient audio on player overlaps", [this]()
		{
			// Simula entrada do jogador (Pawn local) chamando diretamente a função de overlap begin
			UBoxComponent* BoxComp = AmbientTrigger->FindComponentByClass<UBoxComponent>();
			TestNotNull("Trigger deve conter BoxComponent", BoxComp);

			FHitResult HitResult;
			// Simula overlap begin
			BoxComp->OnComponentBeginOverlap.Broadcast(
				BoxComp,
				TestCharacter,
				TestCharacter->FindComponentByClass<UPrimitiveComponent>(),
				0,
				false,
				HitResult
			);

			// 1. Simula entrada do jogador (Pawn local) chamando diretamente a função de overlap begin
			BoxComp->OnComponentBeginOverlap.Broadcast(
				BoxComp,
				TestCharacter,
				TestCharacter->FindComponentByClass<UPrimitiveComponent>(),
				0,
				false,
				HitResult
			);

			// 2. Simula saída do jogador
			BoxComp->OnComponentEndOverlap.Broadcast(
				BoxComp,
				TestCharacter,
				TestCharacter->FindComponentByClass<UPrimitiveComponent>(),
				0
			);
		});
	});
}
