#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Character/SBCharacter.h"
#include "Components/SBMovementComponent.h"
#include "Components/SBCombatComponent.h"
#include "Components/SBStateComponent.h"
#include "GameplayTagsManager.h"
#include "Weapons/SBWeaponBehavior.h"
#include "Weapons/SBWeaponBehaviorHitscan.h"
#include "DataAssets/SBWeaponBehaviorDefinition.h"
#include "GameFramework/CharacterMovementComponent.h"

BEGIN_DEFINE_SPEC(FSBAIBehaviorTestsSpec, "Sandbox.AIBehavior", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TestCharacter;
	USBStateComponent* StateComponent;
	USBMovementComponent* MovementComponent;
	USBCombatComponent* CombatComponent;

	FGameplayTag StunnedTag;
	FGameplayTag FrozenTag;
	FGameplayTag WeaponTag;
END_DEFINE_SPEC(FSBAIBehaviorTestsSpec)

void FSBAIBehaviorTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		FActorSpawnParameters SpawnParams;
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TestCharacter->SetRole(ROLE_Authority);

		// Inicializa Tags
		UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
		StunnedTag = TagsManager.AddNativeGameplayTag(TEXT("State.Character.Stunned"));
		FrozenTag = TagsManager.AddNativeGameplayTag(TEXT("State.Character.Frozen"));
		WeaponTag = TagsManager.AddNativeGameplayTag(TEXT("Combat.Action.Fire"));

		// Instancia componentes
		StateComponent = NewObject<USBStateComponent>(TestCharacter);
		StateComponent->RegisterComponent();
		
		MovementComponent = NewObject<USBMovementComponent>(TestCharacter);
		MovementComponent->RegisterComponent();

		CombatComponent = NewObject<USBCombatComponent>(TestCharacter);
		CombatComponent->RegisterComponent();

		ISBComponentInterface::Execute_OnInitialize(StateComponent);
		ISBComponentInterface::Execute_OnInitialize(MovementComponent);
		ISBComponentInterface::Execute_OnInitialize(CombatComponent);

		ISBComponentInterface::Execute_OnReady(StateComponent);
		ISBComponentInterface::Execute_OnReady(MovementComponent);
		ISBComponentInterface::Execute_OnReady(CombatComponent);
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

	It("Should manage Agro table entries and select the highest agro target correctly", [this]()
	{
		FActorSpawnParameters SpawnParams;
		ASBCharacter* Player1 = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		ASBCharacter* Player2 = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		// Inicialmente não há target de agro
		TestNull("Agro target inicial deve ser nulo", CombatComponent->GetHighestAgroTarget());

		// Adiciona agro para Player 1
		CombatComponent->AddAgro(Player1, 50.0f);
		TestEqual("Highest agro deve ser Player 1", CombatComponent->GetHighestAgroTarget(), Cast<APawn>(Player1));

		// Adiciona mais agro para Player 2 (torna Player 2 maior)
		CombatComponent->AddAgro(Player2, 100.0f);
		TestEqual("Highest agro deve ser Player 2", CombatComponent->GetHighestAgroTarget(), Cast<APawn>(Player2));

		// Limpa agro de Player 2
		CombatComponent->ClearAgro(Player2);
		TestEqual("Highest agro deve voltar para Player 1", CombatComponent->GetHighestAgroTarget(), Cast<APawn>(Player1));

		// Destrói Player 1 e verifica que o agro se resolve para nulo e limpa a tabela
		Player1->Destroy();
		TestNull("Highest agro deve ser nulo apos destruicao", CombatComponent->GetHighestAgroTarget());

		Player2->Destroy();
	});

	It("Should override max speed to zero when Stunned or Frozen state tags are applied", [this]()
	{
		// Velocidade padrão configurada
		TestCharacter->GetCharacterMovement()->MaxWalkSpeed = 600.0f;
		TestEqual("Velocidade padrao deve ser 600.0", MovementComponent->GetCalculatedMaxSpeed(), 600.0f);

		// Aplica Stun
		StateComponent->AddTag(StunnedTag);
		TestEqual("Velocidade deve cair para zero com StunnedTag", MovementComponent->GetCalculatedMaxSpeed(), 0.0f);

		// Remove Stun
		StateComponent->RemoveTag(StunnedTag);
		TestEqual("Velocidade deve restaurar para 600.0", MovementComponent->GetCalculatedMaxSpeed(), 600.0f);

		// Aplica Frozen
		StateComponent->AddTag(FrozenTag);
		TestEqual("Velocidade deve cair para zero com FrozenTag", MovementComponent->GetCalculatedMaxSpeed(), 0.0f);

		// Remove Frozen
		StateComponent->RemoveTag(FrozenTag);
		TestEqual("Velocidade deve restaurar novamente para 600.0", MovementComponent->GetCalculatedMaxSpeed(), 600.0f);
	});

	It("Should block weapon behavior entry if character has stunned state blocked tag", [this]()
	{
		// Registra o behavior de arma usando a classe concreta USBWeaponBehaviorHitscan e passando a definição
		USBWeaponBehaviorDefinition* Def = NewObject<USBWeaponBehaviorDefinition>();
		Def->BehaviorTag = WeaponTag;
		Def->BlockedTags.AddTag(StunnedTag);
		Def->FireRate = 0.0f;

		USBWeaponBehavior* TestWeapon = NewObject<USBWeaponBehaviorHitscan>(CombatComponent);
		TestWeapon->Initialize(CombatComponent, Def);
		CombatComponent->AddAvailableBehavior(TestWeapon);

		// Ativação sem stun deve passar
		bool bActivated = CombatComponent->RequestWeaponBehavior(WeaponTag);
		TestTrue("Deve ativar arma normalmente", bActivated);
		CombatComponent->StopWeaponBehavior(WeaponTag);

		// Aplica Stun no State Component
		StateComponent->AddTag(StunnedTag);

		// Ativação deve falhar por conta do stun bloqueado
		bool bActivatedBlocked = CombatComponent->RequestWeaponBehavior(WeaponTag);
		TestFalse("Ativacao deve ser bloqueada sob stun", bActivatedBlocked);

		// Remove stun e ativa de novo
		StateComponent->RemoveTag(StunnedTag);
		bool bActivatedAgain = CombatComponent->RequestWeaponBehavior(WeaponTag);
		TestTrue("Ativacao deve voltar a funcionar apos limpar stun", bActivatedAgain);
	});
}
