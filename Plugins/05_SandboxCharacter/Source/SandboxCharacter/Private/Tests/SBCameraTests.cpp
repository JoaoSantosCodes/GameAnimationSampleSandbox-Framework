#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SBCameraComponent.h"
#include "Components/SBStateComponent.h"
#include "Camera/Modes/SBCameraMode.h"
#include "Camera/DataAssets/SBCameraModeDefinition.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Mock classes are now defined in SBCameraComponent.h

BEGIN_DEFINE_SPEC(FSBCameraTestsSpec, "Sandbox.Character.Camera", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ACharacter* TestCharacter;
	USBStateComponent* StateComponent;
	USBTestCameraComponent* CameraComponent;
	
	USpringArmComponent* SpringArmComponent;
	UCameraComponent* NativeCameraComponent;

	USBCameraModeDefinition* WalkDef;
	USBCameraModeDefinition* AimDef;

	FGameplayTag WalkTag;
	FGameplayTag AimTag;
END_DEFINE_SPEC(FSBCameraTestsSpec)

void FSBCameraTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		FActorSpawnParameters SpawnParams;
		TestCharacter = TestWorld->SpawnActor<ACharacter>(ACharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		
		// Instancia e registra o Spring Arm e a Câmera nativos na malha
		SpringArmComponent = NewObject<USpringArmComponent>(TestCharacter);
		SpringArmComponent->RegisterComponent();
		
		NativeCameraComponent = NewObject<UCameraComponent>(TestCharacter);
		NativeCameraComponent->RegisterComponent();

		// Instancia e registra componentes do Sandbox
		StateComponent = NewObject<USBStateComponent>(TestCharacter);
		StateComponent->RegisterComponent();

		CameraComponent = NewObject<USBTestCameraComponent>(TestCharacter);
		CameraComponent->RegisterComponent();

		// Inicialização
		ISBComponentInterface::Execute_OnInitialize(StateComponent);
		ISBComponentInterface::Execute_OnInitialize(CameraComponent);

		ISBComponentInterface::Execute_OnReady(StateComponent);
		ISBComponentInterface::Execute_OnReady(CameraComponent);

		// Configura PlayerController fictício para fingir ser LocallyControlled nos testes
		TestCharacter->SetOwner(TestWorld->GetFirstPlayerController());

		// Tags de teste
		WalkTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Walking"));
		AimTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Aiming"));

		// Cria as definições em memória
		WalkDef = NewObject<USBCameraModeDefinition>();
		WalkDef->ActivationTag = WalkTag;
		WalkDef->Priority = 0;
		WalkDef->TargetFOV = 90.0f;
		WalkDef->TargetArmLength = 300.0f;
		WalkDef->CameraModeClass = USBMockCameraMode::StaticClass();

		AimDef = NewObject<USBCameraModeDefinition>();
		AimDef->ActivationTag = AimTag;
		AimDef->Priority = 20;
		AimDef->TargetFOV = 65.0f;
		AimDef->TargetArmLength = 150.0f;
		AimDef->CameraModeClass = USBMockCameraMode::StaticClass();

		TArray<TObjectPtr<USBCameraModeDefinition>> Configs;
		Configs.Add(WalkDef);
		Configs.Add(AimDef);
		CameraComponent->SetCameraModeConfigs(Configs);
	});

	AfterEach([this]()
	{
		if (TestCharacter)
		{
			TestCharacter->Destroy();
			TestCharacter = nullptr;
		}
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Cenário 1: Coalescimento e prioridade de enquadramento", [this]()
	{
		// 1. Simula mudança de tags no StateComponent
		StateComponent->AddTag(WalkTag);
		StateComponent->AddTag(AimTag);

		TestTrue("Reconstrução da pilha de câmera deve estar agendada", CameraComponent->GetStackChangePending());

		// 2. Dispara a reconstrução direta
		CameraComponent->TriggerDirectRebuild();

		const TArray<TObjectPtr<USBCameraMode>>& ActiveModes = CameraComponent->GetActiveCameraModes();

		TestEqual("Deve haver 2 modos de câmera ativos na pilha", ActiveModes.Num(), 2);
		if (ActiveModes.Num() == 2)
		{
			// AimMode tem prioridade 20, então deve estar no topo (índice 0)
			TestEqual("O modo do topo (índice 0) deve ser Aim (prioridade 20)", ActiveModes[0]->GetDefinition(), AimDef);
			TestEqual("O modo inferior (índice 1) deve ser Walk (prioridade 0)", ActiveModes[1]->GetDefinition(), WalkDef);
		}
	});

	It("Cenário 2: Ciclo de Atualização Contínuo dos modos inativos", [this]()
	{
		// 1. Ativa as tags e reconstrói a pilha
		StateComponent->AddTag(WalkTag);
		StateComponent->AddTag(AimTag);
		CameraComponent->TriggerDirectRebuild();

		const TArray<TObjectPtr<USBCameraMode>>& ActiveModes = CameraComponent->GetActiveCameraModes();
		TestEqual("Pilha com 2 modos ativos", ActiveModes.Num(), 2);

		// 2. Roda o Tick do componente de câmera
		CameraComponent->TickComponent(0.016f, LEVELTICK_All, nullptr);

		// 3. Valida que o Update rodou para ambos os modos na pilha (ativo e em background)
		if (ActiveModes.Num() == 2)
		{
			USBMockCameraMode* MockAim = Cast<USBMockCameraMode>(ActiveModes[0]);
			USBMockCameraMode* MockWalk = Cast<USBMockCameraMode>(ActiveModes[1]);

			TestTrue("MockAim deve ter rodado Update", MockAim && MockAim->UpdateCount > 0);
			TestTrue("MockWalk deve ter rodado Update mesmo estando no fundo da pilha", MockWalk && MockWalk->UpdateCount > 0);
		}
	});
}
