#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Character/SBCharacter.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SBStatusEffectComponent.h"
#include "DataAssets/SBStatusEffectDefinition.h"
#include "GameplayTagsManager.h"

BEGIN_DEFINE_SPEC(FSBStatusEffectTestsSpec, "Sandbox.StatusEffects", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TestCharacter;
	USBStateComponent* StateComponent;
	USBAttributeComponent* AttributeComponent;
	USBStatusEffectComponent* StatusEffectComponent;

	FGameplayTag BuffTag;
	FGameplayTag DebuffTag;
	FGameplayTag HealthTag;
	FGameplayTag SpeedTag;
END_DEFINE_SPEC(FSBStatusEffectTestsSpec)

void FSBStatusEffectTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		FActorSpawnParameters SpawnParams;
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TestCharacter->SetRole(ROLE_Authority);
		
		// Inicializa Tags
		UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
		BuffTag = TagsManager.AddNativeGameplayTag(TEXT("State.Buff.Regen"));
		DebuffTag = TagsManager.AddNativeGameplayTag(TEXT("State.Debuff.Slowed"));
		HealthTag = TagsManager.AddNativeGameplayTag(TEXT("Attribute.Health"));
		SpeedTag = TagsManager.AddNativeGameplayTag(TEXT("Attribute.Speed"));
		TagsManager.AddNativeGameplayTag(TEXT("State.Character.Invulnerable"));
		TagsManager.AddNativeGameplayTag(TEXT("State.Character.Slowed"));
		TagsManager.AddNativeGameplayTag(TEXT("State.Debuff.Poison"));

		// Instancia e registra componentes
		StateComponent = NewObject<USBStateComponent>(TestCharacter);
		StateComponent->RegisterComponent();

		AttributeComponent = NewObject<USBAttributeComponent>(TestCharacter);
		AttributeComponent->RegisterComponent();

		StatusEffectComponent = NewObject<USBStatusEffectComponent>(TestCharacter);
		StatusEffectComponent->RegisterComponent();

		// Inicialização
		ISBComponentInterface::Execute_OnInitialize(StateComponent);
		ISBComponentInterface::Execute_OnInitialize(AttributeComponent);

		ISBComponentInterface::Execute_OnReady(StateComponent);
		ISBComponentInterface::Execute_OnReady(AttributeComponent);

		// Configura atributos padrão (Vida = 100, Velocidade = 400)
		FSBAttribute HealthAttr;
		HealthAttr.BaseValue = 100.0f;
		HealthAttr.CurrentValue = 100.0f;
		HealthAttr.MaxValue = 100.0f;
		HealthAttr.MinValue = 0.0f;
		AttributeComponent->RegisterAttribute(HealthTag, HealthAttr);

		FSBAttribute SpeedAttr;
		SpeedAttr.BaseValue = 400.0f;
		SpeedAttr.CurrentValue = 400.0f;
		SpeedAttr.MaxValue = 1000.0f;
		SpeedAttr.MinValue = 0.0f;
		AttributeComponent->RegisterAttribute(SpeedTag, SpeedAttr);
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

	It("Should apply and remove permanent status effects with tags and modifiers", [this]()
	{
		// Configura Definição de Buff
		USBStatusEffectDefinition* BuffDef = NewObject<USBStatusEffectDefinition>();
		BuffDef->EffectTag = BuffTag;
		BuffDef->DefaultDuration = 0.0f; // Permanente
		BuffDef->DefaultPeriod = 0.0f;
		BuffDef->GrantedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.Character.Invulnerable")));

		FSBStatusEffectModifier ModEntry;
		ModEntry.AttributeTag = SpeedTag;
		ModEntry.Modifier.Magnitude = 100.0f;
		ModEntry.Modifier.ModifierType = ESBAttributeModifierType::Additive;
		BuffDef->AttributeModifiers.Add(ModEntry);

		// 1. Aplica o Buff
		StatusEffectComponent->ApplyStatusEffect(BuffDef);

		TestTrue("Deve ter o status effect ativo", StatusEffectComponent->HasStatusEffect(BuffTag));
		TestTrue("Deve ter ganho a tag de estado", StateComponent->HasTag(FGameplayTag::RequestGameplayTag(TEXT("State.Character.Invulnerable"))));
		
		// Força o tick dos atributos para recalcular o modificador
		AttributeComponent->TickComponent(0.016f, LEVELTICK_All, nullptr);
		TestEqual("Velocidade deve ter recebido o bônus aditivo (+100)", AttributeComponent->GetAttributeValue(SpeedTag), 500.0f);

		// 2. Remove o Buff manualmente
		StatusEffectComponent->RemoveStatusEffect(BuffTag);

		TestFalse("Não deve mais possuir o status effect", StatusEffectComponent->HasStatusEffect(BuffTag));
		TestFalse("A tag de estado deve ter sido limpa", StateComponent->HasTag(FGameplayTag::RequestGameplayTag(TEXT("State.Character.Invulnerable"))));

		AttributeComponent->TickComponent(0.016f, LEVELTICK_All, nullptr);
		TestEqual("Velocidade deve ter retornado ao padrão (400)", AttributeComponent->GetAttributeValue(SpeedTag), 400.0f);
	});

	It("Should automatically expire temporal status effects", [this]()
	{
		// Configura Definição de Debuff Temporal (Duração 2.0s)
		USBStatusEffectDefinition* DebuffDef = NewObject<USBStatusEffectDefinition>();
		DebuffDef->EffectTag = DebuffTag;
		DebuffDef->DefaultDuration = 2.0f;
		DebuffDef->DefaultPeriod = 0.0f;
		DebuffDef->GrantedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.Character.Slowed")));

		// 1. Aplica
		TestWorld->TimeSeconds = 1.0f;
		StatusEffectComponent->ApplyStatusEffect(DebuffDef);

		TestTrue("Debuff deve estar ativo", StatusEffectComponent->HasStatusEffect(DebuffTag));
		TestTrue("Tag de estado lenta ativa", StateComponent->HasTag(FGameplayTag::RequestGameplayTag(TEXT("State.Character.Slowed"))));
		TestEqual("Tempo restante deve ser aproximadamente 2.0s", StatusEffectComponent->GetEffectRemainingTime(DebuffTag), 2.0f);

		// 2. Tick de tempo antes da expiração (avança 1.0s, novo tempo = 2.0s)
		TestWorld->TimeSeconds = 2.0f;
		StatusEffectComponent->TickComponent(1.0f, LEVELTICK_All, nullptr);

		TestTrue("Debuff ainda deve estar ativo após 1.0s", StatusEffectComponent->HasStatusEffect(DebuffTag));
		TestEqual("Tempo restante deve ser aproximadamente 1.0s", StatusEffectComponent->GetEffectRemainingTime(DebuffTag), 1.0f);

		// 3. Tick de tempo após a expiração (avança 1.5s, novo tempo = 3.5s)
		TestWorld->TimeSeconds = 3.5f;
		StatusEffectComponent->TickComponent(1.5f, LEVELTICK_All, nullptr);

		TestFalse("Debuff deve expirar automaticamente", StatusEffectComponent->HasStatusEffect(DebuffTag));
		TestFalse("Tag lenta deve ter sido removida", StateComponent->HasTag(FGameplayTag::RequestGameplayTag(TEXT("State.Character.Slowed"))));
	});

	It("Should apply periodic damage ticks (DOT)", [this]()
	{
		// Configura Poison DOT (Duração 3.0s, Período 1.0s, Dano -10 de Vida)
		FGameplayTag PoisonTag = FGameplayTag::RequestGameplayTag(TEXT("State.Debuff.Poison"));
		USBStatusEffectDefinition* PoisonDef = NewObject<USBStatusEffectDefinition>();
		PoisonDef->EffectTag = PoisonTag;
		PoisonDef->DefaultDuration = 3.0f;
		PoisonDef->DefaultPeriod = 1.0f;
		PoisonDef->PeriodAttributeTag = HealthTag;
		PoisonDef->PeriodAttributeChange = -10.0f;

		// 1. Aplica Poison
		TestWorld->TimeSeconds = 1.0f;
		StatusEffectComponent->ApplyStatusEffect(PoisonDef);

		TestEqual("Vida inicial deve ser 100", AttributeComponent->GetAttributeValue(HealthTag), 100.0f);

		// 2. Tick de 1.0s (avança tempo para T=2.0s). Deve acionar 1 tick de dano (-10)
		TestWorld->TimeSeconds = 2.0f;
		StatusEffectComponent->TickComponent(1.0f, LEVELTICK_All, nullptr);
		AttributeComponent->TickComponent(1.0f, LEVELTICK_All, nullptr);

		TestEqual("Vida deve cair para 90 após primeiro tick", AttributeComponent->GetAttributeValue(HealthTag), 90.0f);

		// 3. Tick de mais 1.0s (avança tempo para T=3.0s). Deve acionar segundo tick (-10)
		TestWorld->TimeSeconds = 3.0f;
		StatusEffectComponent->TickComponent(1.0f, LEVELTICK_All, nullptr);
		AttributeComponent->TickComponent(1.0f, LEVELTICK_All, nullptr);

		TestEqual("Vida deve cair para 80 após segundo tick", AttributeComponent->GetAttributeValue(HealthTag), 80.0f);
	});
}
