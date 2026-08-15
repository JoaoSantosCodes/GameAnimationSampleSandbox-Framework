#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Character/SBCharacter.h"
#include "Components/SBAttributeComponent.h"
#include "GameplayTagsManager.h"

BEGIN_DEFINE_SPEC(FSBConditionalReplicationTestsSpec, "Sandbox.Attributes.ConditionalReplication", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TestCharacter;
	USBAttributeComponent* AttributeComponent;

	FGameplayTag HealthTag;
	FGameplayTag ManaTag;
	FGameplayTag StaminaTag;
	FGameplayTag AmmoTag;
END_DEFINE_SPEC(FSBConditionalReplicationTestsSpec)

void FSBConditionalReplicationTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		FActorSpawnParameters SpawnParams;
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TestCharacter->SetRole(ROLE_Authority);
		
		UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
		HealthTag = TagsManager.AddNativeGameplayTag(TEXT("Attribute.Health"));
		ManaTag = TagsManager.AddNativeGameplayTag(TEXT("Attribute.Mana"));
		StaminaTag = TagsManager.AddNativeGameplayTag(TEXT("Attribute.Stamina"));
		AmmoTag = TagsManager.AddNativeGameplayTag(TEXT("Attribute.Weapon.Ammo"));

		AttributeComponent = NewObject<USBAttributeComponent>(TestCharacter);
		AttributeComponent->RegisterComponent();
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

	It("Should correctly classify public and private attributes", [this]()
	{
		TestFalse("Health attribute should be public", AttributeComponent->IsAttributePrivate(HealthTag));
		TestTrue("Mana attribute should be private", AttributeComponent->IsAttributePrivate(ManaTag));
		TestTrue("Stamina attribute should be private", AttributeComponent->IsAttributePrivate(StaminaTag));
		TestTrue("Weapon Ammo attribute should be private", AttributeComponent->IsAttributePrivate(AmmoTag));
	});

	It("Should direct updates to the correct replication array based on private/public status", [this]()
	{
		FSBAttribute InitialHealth;
		InitialHealth.BaseValue = 100.f;
		InitialHealth.CurrentValue = 100.f;

		FSBAttribute InitialMana;
		InitialMana.BaseValue = 50.f;
		InitialMana.CurrentValue = 50.f;

		// Registra atributos
		AttributeComponent->RegisterAttribute(HealthTag, InitialHealth);
		AttributeComponent->RegisterAttribute(ManaTag, InitialMana);

		// Verifica se o array PublicAttributes contém o Health
		bool bFoundHealthInPublic = false;
		for (const FSBAttributeReplicationEntry& Entry : AttributeComponent->GetPublicAttributes())
		{
			if (Entry.Tag == HealthTag)
			{
				bFoundHealthInPublic = true;
				TestEqual("Health base value matches in PublicAttributes", Entry.Attribute.BaseValue, 100.f);
			}
		}
		TestTrue("Health must be present in PublicAttributes", bFoundHealthInPublic);

		// Verifica se o array PublicAttributes NÃO contém o Mana
		bool bFoundManaInPublic = false;
		for (const FSBAttributeReplicationEntry& Entry : AttributeComponent->GetPublicAttributes())
		{
			if (Entry.Tag == ManaTag)
			{
				bFoundManaInPublic = true;
			}
		}
		TestFalse("Mana must NOT be present in PublicAttributes", bFoundManaInPublic);

		// Verifica se o array PrivateAttributes contém o Mana
		bool bFoundManaInPrivate = false;
		for (const FSBAttributeReplicationEntry& Entry : AttributeComponent->GetPrivateAttributes())
		{
			if (Entry.Tag == ManaTag)
			{
				bFoundManaInPrivate = true;
				TestEqual("Mana base value matches in PrivateAttributes", Entry.Attribute.BaseValue, 50.f);
			}
		}
		TestTrue("Mana must be present in PrivateAttributes", bFoundManaInPrivate);
	});
}
