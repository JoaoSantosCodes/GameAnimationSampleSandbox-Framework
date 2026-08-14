#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBBehaviorStackComponent.h"
#include "Behaviors/SBGameplayBehavior.h"
#include "Behaviors/SBGameplayBehaviorDefinition.h"
#include "GameplayTagsManager.h"

BEGIN_DEFINE_SPEC(FSBBehaviorStackTestsSpec, "Sandbox.Common.BehaviorStack", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* TestActor;
	USBBehaviorStackComponent* StackComponent;
END_DEFINE_SPEC(FSBBehaviorStackTestsSpec)

void FSBBehaviorStackTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		FActorSpawnParameters SpawnParams;
		TestActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		
		StackComponent = NewObject<USBBehaviorStackComponent>(TestActor);
		StackComponent->RegisterComponent();

		UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
		TagsManager.AddNativeGameplayTag(TEXT("State.Character.Crouching"));
		TagsManager.AddNativeGameplayTag(TEXT("State.Character.Sprinting"));
		TagsManager.AddNativeGameplayTag(TEXT("State.Character.Stance"));
		TagsManager.AddNativeGameplayTag(TEXT("State.Character.Reentrant"));
	});

	AfterEach([this]()
	{
		if (StackComponent)
		{
			StackComponent->DestroyComponent();
		}
		if (TestActor)
		{
			TestActor->Destroy();
		}
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
		}
	});

	Describe("Behavior Priority and Exclusivity", [this]()
	{
		It("Should sort behaviors by descending priority", [this]()
		{
			FGameplayTag TagA = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Crouching"));
			FGameplayTag TagB = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Sprinting"));

			USBGameplayBehaviorDefinition* DefA = NewObject<USBGameplayBehaviorDefinition>(StackComponent);
			DefA->BehaviorTag = TagA;
			DefA->StackPriority = 10;

			USBGameplayBehaviorDefinition* DefB = NewObject<USBGameplayBehaviorDefinition>(StackComponent);
			DefB->BehaviorTag = TagB;
			DefB->StackPriority = 20;

			USBMockGameplayBehavior* BehaviorA = NewObject<USBMockGameplayBehavior>(StackComponent);
			BehaviorA->Initialize(StackComponent, DefA);
			StackComponent->AddAvailableBehavior(BehaviorA);

			USBMockGameplayBehavior* BehaviorB = NewObject<USBMockGameplayBehavior>(StackComponent);
			BehaviorB->Initialize(StackComponent, DefB);
			StackComponent->AddAvailableBehavior(BehaviorB);

			StackComponent->RequestBehavior(TagA);
			StackComponent->RequestBehavior(TagB);

			// Como B tem prioridade 20 e A tem 10, B deve ser o topo (índice 0)
			TestTrue("B deve ser o comportamento atual (prioridade maior)", StackComponent->GetCurrentBehavior() == BehaviorB);
		});

		It("Should eject conflicting behavior in same ExclusivityGroup", [this]()
		{
			FGameplayTag TagA = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Crouching"));
			FGameplayTag TagB = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Sprinting"));
			FGameplayTag Group = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Stance"));

			USBGameplayBehaviorDefinition* DefA = NewObject<USBGameplayBehaviorDefinition>(StackComponent);
			DefA->BehaviorTag = TagA;
			DefA->ExclusivityGroup = Group;
			DefA->StackPriority = 10;

			USBGameplayBehaviorDefinition* DefB = NewObject<USBGameplayBehaviorDefinition>(StackComponent);
			DefB->BehaviorTag = TagB;
			DefB->ExclusivityGroup = Group;
			DefB->StackPriority = 20;

			USBMockGameplayBehavior* BehaviorA = NewObject<USBMockGameplayBehavior>(StackComponent);
			BehaviorA->Initialize(StackComponent, DefA);
			StackComponent->AddAvailableBehavior(BehaviorA);

			USBMockGameplayBehavior* BehaviorB = NewObject<USBMockGameplayBehavior>(StackComponent);
			BehaviorB->Initialize(StackComponent, DefB);
			StackComponent->AddAvailableBehavior(BehaviorB);

			StackComponent->RequestBehavior(TagA);
			TestTrue("A deve ter entrado", BehaviorA->bEntered);

			StackComponent->RequestBehavior(TagB);
			TestTrue("B deve ter entrado", BehaviorB->bEntered);
			TestTrue("A deve ter sido ejetado", BehaviorA->bExited);
			TestFalse("A nao deve estar mais ativo", StackComponent->HasBehavior(TagA));
		});
	});

	Describe("Reentrancy and Recursion Protection", [this]()
	{
		It("Should defer reentrant requests correctly in exit cascades", [this]()
		{
			FGameplayTag TagA = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Crouching"));
			FGameplayTag TagB = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Sprinting"));

			USBGameplayBehaviorDefinition* DefA = NewObject<USBGameplayBehaviorDefinition>(StackComponent);
			DefA->BehaviorTag = TagA;
			DefA->StackPriority = 10;

			USBGameplayBehaviorDefinition* DefB = NewObject<USBGameplayBehaviorDefinition>(StackComponent);
			DefB->BehaviorTag = TagB;
			DefB->StackPriority = 20;

			USBMockGameplayBehavior* BehaviorA = NewObject<USBMockGameplayBehavior>(StackComponent);
			BehaviorA->Initialize(StackComponent, DefA);
			StackComponent->AddAvailableBehavior(BehaviorA);

			USBMockGameplayBehavior* BehaviorB = NewObject<USBMockGameplayBehavior>(StackComponent);
			BehaviorB->Initialize(StackComponent, DefB);
			// Configura B para pedir A ao sair
			BehaviorB->StackToRequestOnExit = StackComponent;
			BehaviorB->TagToRequestOnExit = TagA;
			StackComponent->AddAvailableBehavior(BehaviorB);

			StackComponent->RequestBehavior(TagB);
			TestTrue("B deve estar ativo", StackComponent->HasBehavior(TagB));

			// Para B, o que causara o disparo reentrante para pedir A
			StackComponent->StopBehavior(TagB);

			TestFalse("B deve ter saido", StackComponent->HasBehavior(TagB));
			TestTrue("A deve ter sido ativado deferredly apos a saida de B", StackComponent->HasBehavior(TagA));
		});
	});
}
