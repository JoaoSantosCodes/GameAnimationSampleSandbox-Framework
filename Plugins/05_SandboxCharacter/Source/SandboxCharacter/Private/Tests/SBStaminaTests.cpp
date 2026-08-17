#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Character/SBCharacter.h"
#include "Components/SBMovementComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "GameplayTagsManager.h"
#include "SBGameplayTags.h"
#include "Movement/Behaviors/SBMovementBehavior.h"
#include "Movement/DataAssets/SBMovementBehaviorDefinition.h"

BEGIN_DEFINE_SPEC(FSBStaminaTestsSpec, "Sandbox.Stamina", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TestCharacter;
	USBStateComponent* StateComponent;
	USBAttributeComponent* AttributeComponent;
	USBMovementComponent* MovementComponent;

	FGameplayTag StaminaTag;
	FGameplayTag SprintTag;
	FGameplayTag ExhaustedTag;
END_DEFINE_SPEC(FSBStaminaTestsSpec)

void FSBStaminaTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		FActorSpawnParameters SpawnParams;
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TestCharacter->SetRole(ROLE_Authority);

		// Inicializa Tags
		UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
		StaminaTag = TagsManager.AddNativeGameplayTag(TEXT("Attribute.Stamina"));
		SprintTag = TagsManager.AddNativeGameplayTag(TEXT("State.Character.Sprinting"));
		ExhaustedTag = TagsManager.AddNativeGameplayTag(TEXT("State.Character.Exhausted"));
		TagsManager.AddNativeGameplayTag(TEXT("Movement.Action.Sprint"));

		// Instancia componentes
		StateComponent = NewObject<USBStateComponent>(TestCharacter);
		StateComponent->RegisterComponent();
		
		AttributeComponent = NewObject<USBAttributeComponent>(TestCharacter);
		AttributeComponent->RegisterComponent();

		MovementComponent = NewObject<USBMovementComponent>(TestCharacter);
		MovementComponent->RegisterComponent();

		// Inicialização
		ISBComponentInterface::Execute_OnInitialize(StateComponent);
		ISBComponentInterface::Execute_OnInitialize(AttributeComponent);
		ISBComponentInterface::Execute_OnInitialize(MovementComponent);

		ISBComponentInterface::Execute_OnReady(StateComponent);
		ISBComponentInterface::Execute_OnReady(AttributeComponent);
		ISBComponentInterface::Execute_OnReady(MovementComponent);

		// Configura atributos de estamina iniciais
		FSBAttribute StaminaAttr;
		StaminaAttr.BaseValue = 100.f;
		StaminaAttr.CurrentValue = 100.f;
		StaminaAttr.MaxValue = 100.f;
		StaminaAttr.MinValue = 0.f;
		StaminaAttr.bIsPrivate = true;
		AttributeComponent->RegisterAttribute(StaminaTag, StaminaAttr);
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
			TestWorld->DestroyWorld(true);
			TestWorld = nullptr;
		}
	});

	It("Should consume stamina when sprinting and regenerate after delay", [this]()
	{
		// 1. Inicia o Sprint
		StateComponent->AddTag(SprintTag);
		TestTrue("Deve estar sprintando", StateComponent->HasTag(SprintTag));

		// Tick de 1 segundo consumindo estamina (Custo do Sprint = 15.f/s)
		MovementComponent->TickComponent(1.0f, LEVELTICK_All, nullptr);
		AttributeComponent->TickComponent(1.0f, LEVELTICK_All, nullptr);

		TestEqual("Estamina deve cair para 85", AttributeComponent->GetAttributeValue(StaminaTag), 85.f);

		// 2. Para o Sprint
		StateComponent->RemoveTag(SprintTag);
		TestFalse("Não deve estar sprintando", StateComponent->HasTag(SprintTag));

		// Tick de 1 segundo (dentro do delay de regeneração = 1.5s). Não deve regenerar.
		TestWorld->TimeSeconds = 1.0f;
		MovementComponent->TickComponent(1.0f, LEVELTICK_All, nullptr);
		AttributeComponent->TickComponent(1.0f, LEVELTICK_All, nullptr);

		TestEqual("Estamina deve continuar 85 dentro do delay", AttributeComponent->GetAttributeValue(StaminaTag), 85.f);

		// Tick de mais 1 segundo (tempo total = 2.0s, maior que o delay de 1.5s). Deve começar a regenerar.
		// RegenRate = 10.f/s. Ticked 1.0s -> deve recuperar 10.f.
		TestWorld->TimeSeconds = 2.0f;
		MovementComponent->TickComponent(1.0f, LEVELTICK_All, nullptr);
		AttributeComponent->TickComponent(1.0f, LEVELTICK_All, nullptr);

		TestEqual("Estamina deve recuperar 10 e ir para 95", AttributeComponent->GetAttributeValue(StaminaTag), 95.f);
	});

	It("Should consume stamina when jumping and block jump if insufficient", [this]()
	{
		// Estamina inicial = 100.f. Jump cost = 20.f.
		// 1. Primeiro pulo
		bool bJump1 = MovementComponent->ConsumeJumpStamina();
		TestTrue("Pulo 1 deve ser permitido", bJump1);
		TestEqual("Estamina deve ir para 80", AttributeComponent->GetAttributeValue(StaminaTag), 80.f);

		// Reduz estamina manualmente para 15.f (insuficiente para o pulo de 20.f)
		AttributeComponent->SetAttributeBaseValue(StaminaTag, 15.f);

		// 2. Segundo pulo (insuficiente)
		bool bJump2 = MovementComponent->ConsumeJumpStamina();
		TestFalse("Pulo 2 deve ser negado por falta de estamina", bJump2);
		TestEqual("Estamina deve continuar 15", AttributeComponent->GetAttributeValue(StaminaTag), 15.f);
	});

	It("Should enter exhausted state when stamina hits 0 and recover after threshold", [this]()
	{
		// 1. Zera estamina
		AttributeComponent->SetAttributeBaseValue(StaminaTag, 0.f);
		
		// Tick de movimento para processar a exaustão
		MovementComponent->TickComponent(0.1f, LEVELTICK_All, nullptr);

		TestTrue("Deve entrar em exaustão", StateComponent->HasTag(ExhaustedTag));

		// 2. Tenta sprintar enquanto exausto. O sprint deve ser rejeitado.
		// Criamos a definição para simular o sprint
		USBMovementBehaviorDefinition* SprintDef = NewObject<USBMovementBehaviorDefinition>();
		SprintDef->BehaviorTag = FGameplayTag::RequestGameplayTag(TEXT("Movement.Action.Sprint"));
		
		// 3. Regenera estamina para 25.f (abaixo do limiar de 30.f). Deve continuar exausto.
		AttributeComponent->SetAttributeBaseValue(StaminaTag, 25.f);
		TestWorld->TimeSeconds = 5.0f; // Passou o delay
		MovementComponent->TickComponent(0.1f, LEVELTICK_All, nullptr);

		TestTrue("Deve continuar em exaustão abaixo de 30.f", StateComponent->HasTag(ExhaustedTag));

		// 4. Regenera estamina para 35.f (acima do limiar de 30.f). Deve limpar a exaustão.
		AttributeComponent->SetAttributeBaseValue(StaminaTag, 35.f);
		MovementComponent->TickComponent(0.1f, LEVELTICK_All, nullptr);

		TestFalse("Deve sair da exaustão acima de 30.f", StateComponent->HasTag(ExhaustedTag));
	});
}
