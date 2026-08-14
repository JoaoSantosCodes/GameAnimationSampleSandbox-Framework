#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Subsystems/SBSaveSubsystem.h"
#include "Subsystems/SBSaveSubsystemConcrete.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "Character/SBCharacter.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBSaveTestsSpec, "Sandbox.Core.SaveSystem", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	UGameInstance* GameInstance;
	ASBCharacter* TestCharacter;
	USBAttributeComponent* AttributeComponent;
	USBStateComponent* StateComponent;

	FGameplayTag HealthTag;
	FGameplayTag ManaTag;
END_DEFINE_SPEC(FSBSaveTestsSpec)

void FSBSaveTestsSpec::Define()
{
	BeforeEach([this]()
	{
		GameInstance = NewObject<UGameInstance>(GEngine);
		GameInstance->InitializeStandalone();

		FWorldContext& NewContext = GEngine->CreateNewWorldContext(EWorldType::Game);
		NewContext.OwningGameInstance = GameInstance;

		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		NewContext.SetCurrentWorld(TestWorld);

		TestWorld->SetGameInstance(GameInstance);

		HealthTag = FSBGameplayTags::Get().Attribute_Health;
		ManaTag = FSBGameplayTags::Get().Attribute_Mana;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("PersistentCharacter");
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		StateComponent = NewObject<USBStateComponent>(TestCharacter, TEXT("PersistentState"));
		StateComponent->RegisterComponent();

		AttributeComponent = NewObject<USBAttributeComponent>(TestCharacter, TEXT("PersistentAttributes"));
		AttributeComponent->RegisterComponent();

		ISBComponentInterface::Execute_OnInitialize(StateComponent);
		ISBComponentInterface::Execute_OnInitialize(AttributeComponent);

		ISBComponentInterface::Execute_OnReady(StateComponent);
		ISBComponentInterface::Execute_OnReady(AttributeComponent);
	});

	AfterEach([this]()
	{
		if (TestCharacter)
		{
			TestWorld->DestroyActor(TestCharacter);
			TestCharacter = nullptr;
		}
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
		GEngine->DestroyWorldContext(TestWorld);
	});

	It("Cenário 1: Salvar e Carregar Atributos com Sucesso e Ordem Determinística", [this]()
	{
		// 1. Registra atributos
		FSBAttribute HealthAttr;
		HealthAttr.BaseValue = 100.f;
		HealthAttr.MinValue = 0.f;
		HealthAttr.MaxValue = 100.f;
		AttributeComponent->RegisterAttribute(HealthTag, HealthAttr);

		FSBAttribute ManaAttr;
		ManaAttr.BaseValue = 80.f;
		ManaAttr.MinValue = 0.f;
		ManaAttr.MaxValue = 100.f;
		AttributeComponent->RegisterAttribute(ManaTag, ManaAttr);

		// Modifica valores
		AttributeComponent->SetAttributeBaseValue(HealthTag, 50.f);
		TestEqual("Health deve ser 50 inicialmente", AttributeComponent->GetAttributeValue(HealthTag), 50.f);

		// 2. Salva o jogo via Subsystem
		USBSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<USBSaveSubsystem>();
		TestNotNull("SaveSubsystem deve estar ativo", SaveSubsystem);

		bool bSaveSuccess = SaveSubsystem->SaveGame(TEXT("TestSaveSlot"), 0);
		TestTrue("Salvamento deve ser bem-sucedido", bSaveSuccess);

		// Modifica na memória
		AttributeComponent->SetAttributeBaseValue(HealthTag, 99.f);
		TestEqual("Health deve ser alterado na memória", AttributeComponent->GetAttributeValue(HealthTag), 99.f);

		// 3. Destrói e recria o personagem
		TestCharacter->Rename(TEXT("OldCharacterForDestruction"));
		AttributeComponent->Rename(TEXT("OldAttributes"));
		TestWorld->DestroyActor(TestCharacter);
		TestCharacter = nullptr;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("PersistentCharacter");
		ASBCharacter* NewCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TestCharacter = NewCharacter;

		USBStateComponent* NewState = NewObject<USBStateComponent>(NewCharacter, TEXT("PersistentState"));
		NewState->RegisterComponent();
		USBAttributeComponent* NewAttributes = NewObject<USBAttributeComponent>(NewCharacter, TEXT("PersistentAttributes"));
		NewAttributes->RegisterComponent();

		ISBComponentInterface::Execute_OnInitialize(NewState);
		ISBComponentInterface::Execute_OnInitialize(NewAttributes);

		ISBComponentInterface::Execute_OnReady(NewState);
		ISBComponentInterface::Execute_OnReady(NewAttributes);

		NewAttributes->RegisterAttribute(HealthTag, HealthAttr);
		NewAttributes->RegisterAttribute(ManaTag, ManaAttr);

		// 4. Carrega o jogo
		bool bLoadSuccess = SaveSubsystem->LoadGame(TEXT("TestSaveSlot"), 0);
		TestTrue("Carregamento deve ser bem-sucedido", bLoadSuccess);

		// 5. Valida restauração
		TestEqual("Health carregado deve voltar a ser 50.f", NewAttributes->GetAttributeValue(HealthTag), 50.f);
		TestEqual("Mana carregado deve voltar a ser 80.f", NewAttributes->GetAttributeValue(ManaTag), 80.f);
	});
}
