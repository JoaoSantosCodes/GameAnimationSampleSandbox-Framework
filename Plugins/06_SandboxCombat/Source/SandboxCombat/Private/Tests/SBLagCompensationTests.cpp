#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Character/SBCharacter.h"
#include "Subsystems/SBLagCompensationSubsystem.h"

BEGIN_DEFINE_SPEC(FSBLagCompensationTestsSpec, "Sandbox.LagCompensation", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TargetCharacter;
	USBLagCompensationSubsystem* LagCompSubsystem;
END_DEFINE_SPEC(FSBLagCompensationTestsSpec)

void FSBLagCompensationTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		FActorSpawnParameters SpawnParams;
		TargetCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		
		LagCompSubsystem = TestWorld->GetSubsystem<USBLagCompensationSubsystem>();
	});

	AfterEach([this]()
	{
		if (TargetCharacter)
		{
			TargetCharacter->Destroy();
			TargetCharacter = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(true);
			TestWorld = nullptr;
		}
	});

	It("Should record, interpolate, rewind, and restore positions correctly", [this]()
	{
		TestNotNull("Subsystem deve existir", LagCompSubsystem);
		TestNotNull("Target character deve existir", TargetCharacter);

		// Limpa qualquer dado residual no histórico para fins de isolamento do teste
		// (Normalmente limpo ao criar o UWorld novo, mas bom para garantir)
		LagCompSubsystem->RecordPositions();

		// 1. Grava a Posição 1 no tempo T = 1.0s
		TargetCharacter->SetActorLocation(FVector(0.f, 0.f, 0.f));
		float Time1 = 1.0f;
		TestWorld->TimeSeconds = Time1;
		LagCompSubsystem->RecordPositions();

		// 2. Grava a Posição 2 no tempo T = 2.0s
		TargetCharacter->SetActorLocation(FVector(100.f, 0.f, 0.f));
		float Time2 = 2.0f;
		TestWorld->TimeSeconds = Time2;
		LagCompSubsystem->RecordPositions();

		// O personagem está atualmente na Posição 2 (100.f)
		TestEqual("Personagem deve estar na Posição 2 inicialmente", TargetCharacter->GetActorLocation().X, 100.0);

		// 3. Rebobina para o tempo T = 1.5s (metade do caminho entre 0.f e 100.f)
		TMap<TWeakObjectPtr<ACharacter>, FTransform> OriginalTransforms;
		float TargetTime = 1.5f;
		LagCompSubsystem->RewindPositions(TargetTime, OriginalTransforms);

		// A posição rebobinada interpolada deve ser FVector(50.f, 0.f, 0.f)
		FVector ExpectedRewoundLoc(50.f, 0.f, 0.f);
		TestTrue("Posição do personagem deve ter sido rebobinada para a metade (50.f)", TargetCharacter->GetActorLocation().Equals(ExpectedRewoundLoc, 0.1f));
		TestEqual("Mapa de originais deve conter 1 entrada", OriginalTransforms.Num(), 1);

		// 4. Restaura as posições
		LagCompSubsystem->RestorePositions(OriginalTransforms);

		// Deve retornar à Posição 2 (100.f)
		TestEqual("Personagem deve retornar para a Posição atual de autoridade (100.f)", TargetCharacter->GetActorLocation().X, 100.0);
	});
}
