#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Components/SBAttributeComponent.h"
#include "Interfaces/SBAttributeComponentInterface.h"
#include "SBGameplayTags.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSBAttributeContractTest, "Sandbox.Character.AttributeContract", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSBAttributeContractTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* OwnerActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	USceneComponent* Root = NewObject<USceneComponent>(OwnerActor, TEXT("Root"));
	OwnerActor->SetRootComponent(Root);
	Root->RegisterComponent();

	USBAttributeComponent* AttrComp = NewObject<USBAttributeComponent>(OwnerActor);
	AttrComp->RegisterComponent();

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	FSBAttribute Stamina;
	Stamina.BaseValue = 40.0f;
	Stamina.MinValue = 0.0f;
	Stamina.MaxValue = 80.0f;
	AttrComp->RegisterAttribute(Tags.Attribute_Stamina, Stamina);

	// O componente precisa ser alcancavel pelo contrato, e nao so pela classe concreta:
	// 09_SandboxUI nao depende de 05_SandboxCharacter e resolve por interface.
	UActorComponent* Resolved = OwnerActor->FindComponentByInterface(USBAttributeComponentInterface::StaticClass());
	UTEST_NOT_NULL(TEXT("Componente de atributos deve ser resolvivel por interface"), Resolved);
	UTEST_TRUE(TEXT("Componente resolvido deve implementar o contrato"), Resolved->Implements<USBAttributeComponentInterface>());

	UTEST_EQUAL(TEXT("GetAttributeValue via contrato deve devolver o valor corrente"),
		ISBAttributeComponentInterface::Execute_GetAttributeValue(Resolved, Tags.Attribute_Stamina), 40.0f);

	UTEST_EQUAL(TEXT("GetAttributeMaxValue via contrato deve devolver o teto"),
		ISBAttributeComponentInterface::Execute_GetAttributeMaxValue(Resolved, Tags.Attribute_Stamina), 80.0f);

	// O teto e o dado que torna a proporcao possivel; sem ele a barra so poderia ser
	// alimentada por evento empurrado.
	const float Max = ISBAttributeComponentInterface::Execute_GetAttributeMaxValue(Resolved, Tags.Attribute_Stamina);
	const float Current = ISBAttributeComponentInterface::Execute_GetAttributeValue(Resolved, Tags.Attribute_Stamina);
	UTEST_EQUAL(TEXT("Proporcao formada apenas com o contrato deve ser 0.5"), Current / Max, 0.5f);

	// O teto acompanha alteracoes do valor corrente sem mudar sozinho.
	AttrComp->SetAttributeBaseValue(Tags.Attribute_Stamina, 20.0f);
	UTEST_EQUAL(TEXT("Valor corrente deve refletir a escrita"),
		ISBAttributeComponentInterface::Execute_GetAttributeValue(Resolved, Tags.Attribute_Stamina), 20.0f);
	UTEST_EQUAL(TEXT("Teto nao deve mudar quando o valor corrente muda"),
		ISBAttributeComponentInterface::Execute_GetAttributeMaxValue(Resolved, Tags.Attribute_Stamina), 80.0f);

	// Atributo inexistente devolve 0 em vez de lixo, e o consumidor usa isso para nao dividir.
	UTEST_EQUAL(TEXT("Teto de atributo nao registrado deve ser 0"),
		ISBAttributeComponentInterface::Execute_GetAttributeMaxValue(Resolved, Tags.Attribute_MaxWeight), 0.0f);

	OwnerActor->Destroy();
	World->DestroyWorld(false);

	return true;
}
