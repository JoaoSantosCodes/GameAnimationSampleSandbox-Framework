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

	It("Should correctly classify public and private attributes based on FSBAttribute metadata", [this]()
	{
		FSBAttribute PublicAttr;
		PublicAttr.bIsPrivate = false;
		AttributeComponent->RegisterAttribute(HealthTag, PublicAttr);

		FSBAttribute PrivateAttr;
		PrivateAttr.bIsPrivate = true;
		AttributeComponent->RegisterAttribute(ManaTag, PrivateAttr);

		TestFalse("Health attribute should be public", AttributeComponent->IsAttributePrivate(HealthTag));
		TestTrue("Mana attribute should be private", AttributeComponent->IsAttributePrivate(ManaTag));
	});

	It("Should direct updates to the correct replication array and clean opposite channel if it changes", [this]()
	{
		// 1. Registra Health como público inicialmente
		FSBAttribute InitialHealth;
		InitialHealth.BaseValue = 100.f;
		InitialHealth.CurrentValue = 100.f;
		InitialHealth.bIsPrivate = false;
		AttributeComponent->RegisterAttribute(HealthTag, InitialHealth);

		// Verifica presença no array público
		bool bFoundInPublic = false;
		for (const FSBAttributeReplicationEntry& Entry : AttributeComponent->GetPublicAttributes())
		{
			if (Entry.Tag == HealthTag) bFoundInPublic = true;
		}
		TestTrue("Health deve estar inicialmente no array publico", bFoundInPublic);

		// 2. Modifica a flag bIsPrivate diretamente no mapa de atributos do servidor
		FSBAttribute* AttrPtr = AttributeComponent->AttributesMap.Find(HealthTag);
		if (AttrPtr)
		{
			AttrPtr->bIsPrivate = true;
		}
		// Força atualização da replicação
		AttributeComponent->SetAttributeBaseValue(HealthTag, 100.f);

		// Verifica migração para o array privado
		bool bFoundInPrivateAfter = false;
		for (const FSBAttributeReplicationEntry& Entry : AttributeComponent->GetPrivateAttributes())
		{
			if (Entry.Tag == HealthTag) bFoundInPrivateAfter = true;
		}
		TestTrue("Health deve migrar para o array privado", bFoundInPrivateAfter);

		// Verifica que foi removido do array público
		bool bFoundInPublicAfter = false;
		for (const FSBAttributeReplicationEntry& Entry : AttributeComponent->GetPublicAttributes())
		{
			if (Entry.Tag == HealthTag) bFoundInPublicAfter = true;
		}
		TestFalse("Health deve ser removido do array publico ao mudar de canal", bFoundInPublicAfter);
	});

	It("Should simulate network replication segregation between Owner Client and Simulated Proxy Client", [this]()
	{
		// 1. Registra atributos no servidor
		FSBAttribute HealthAttr;
		HealthAttr.BaseValue = 100.f;
		HealthAttr.CurrentValue = 100.f;
		HealthAttr.bIsPrivate = false;
		AttributeComponent->RegisterAttribute(HealthTag, HealthAttr);

		FSBAttribute ManaAttr;
		ManaAttr.BaseValue = 50.f;
		ManaAttr.CurrentValue = 50.f;
		ManaAttr.bIsPrivate = true;
		AttributeComponent->RegisterAttribute(ManaTag, ManaAttr);

		// 2. Instancia o cliente simulado
		ASBCharacter* ClientProxy = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		USBAttributeComponent* ClientAttributes = NewObject<USBAttributeComponent>(ClientProxy);
		ClientAttributes->RegisterComponent();
		ISBComponentInterface::Execute_OnInitialize(ClientAttributes);
		ISBComponentInterface::Execute_OnReady(ClientAttributes);

		// 3. Simula replicação para o Dono (Owner Client - recebe ambos)
		for (const FSBAttributeReplicationEntry& Entry : AttributeComponent->GetPublicAttributes())
		{
			ClientAttributes->PublicAttributes.Add(Entry);
		}
		for (const FSBAttributeReplicationEntry& Entry : AttributeComponent->GetPrivateAttributes())
		{
			ClientAttributes->PrivateAttributes.Add(Entry);
		}
		ClientAttributes->OnRep_ReplicatedAttributes();

		TestEqual("Dono do cliente deve receber atributo publico (Health)", ClientAttributes->GetAttributeValue(HealthTag), 100.f);
		TestEqual("Dono do cliente deve receber atributo privado (Mana)", ClientAttributes->GetAttributeValue(ManaTag), 50.f);

		// 4. Simula replicação para Proxy Simulado (Simulated Proxy - recebe APENAS publicos devido ao COND_OwnerOnly)
		ClientAttributes->PublicAttributes.Empty();
		ClientAttributes->PrivateAttributes.Empty();
		ClientAttributes->AttributesMap.Empty(); // Limpa poluição de estado do passo anterior

		for (const FSBAttributeReplicationEntry& Entry : AttributeComponent->GetPublicAttributes())
		{
			ClientAttributes->PublicAttributes.Add(Entry);
		}
		// PrivateAttributes NÃO são transmitidos pelo net driver para conexões não-dono!
		ClientAttributes->OnRep_ReplicatedAttributes();

		TestEqual("Proxy simulado deve receber atributo publico (Health)", ClientAttributes->GetAttributeValue(HealthTag), 100.f);
		TestEqual("Proxy simulado NÃO deve receber atributo privado (Mana)", ClientAttributes->GetAttributeValue(ManaTag), 0.f);

		if (ClientProxy)
		{
			ClientProxy->Destroy();
		}
	});
}
