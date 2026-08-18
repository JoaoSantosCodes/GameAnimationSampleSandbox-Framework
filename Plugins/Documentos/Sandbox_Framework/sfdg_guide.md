# Sandbox Framework Development Guide (SFDG v1.0.0)

Este guia prático descreve os passos operacionais para criar comportamentos, registrar tags e usar os subsistemas do framework seguindo as especificações da **SFPS v1.0.0**.

---

## 1. Como Criar e Registrar um Comportamento de Movimentação

A movimentação é implementada dentro do subdiretório de domínio no plugin agregador **`05_SandboxCharacter`**:

1. **Defina a Classe do Comportamento**:
   Crie a classe C++ herdando de `USBMovementBehavior`.
2. **Defina o Data Asset de Definição (Definition)**:
   Crie um Data Asset herdando de `USBMovementBehaviorDefinition` contendo os parâmetros estáticos (prioridade, regras de interrupção).
3. **Mapear Inputs**:
   Configure o Enhanced Input para emitir a tag de ação (ex: `Input.Action.Sprint`).
4. **Implemente a Lógica usando o Contexto Unificado (`FSBBehaviorContext`)**:
   ```cpp
   bool USBMovementBehaviorSprint::CanActivate_Implementation(const FSBBehaviorContext& Context)
   {
       // O contexto unificado absorve novos tipos de contexto sem quebrar a assinatura da função
       const FSBGameplayContext* Gameplay = Context.GameplayContext;
       if (Gameplay && Gameplay->PlayerState)
       {
           // Validação usando PlayerState
       }
       return true;
   }
   ```

---

## 2. Como Utilizar o Message Router (Gameplay Message Router)

O subsistema de eventos suporta tanto mensagens transientes quanto de estado persistente:

1. **Mensagens Instantâneas**:
   ```cpp
   FSBWeaponFiredMessage Message;
   Message.Instigator = GetOwner();
   USBEventSubsystem::Get(GetWorld())->BroadcastMessage(FSBGameplayTags::Get().Event_Weapon_Fire, Message);
   ```
2. **Mensagens de Estado Persistente (Stateful)**:
   ```cpp
   FSBCharacterStateMessage SprintState;
   SprintState.bIsActive = true;
   USBEventSubsystem::Get(GetWorld())->PublishState(FSBGameplayTags::Get().State_Character_Sprinting, SprintState);
   ```
3. **Limpeza de Cache com `ISBResettable`**:
   Ao destruir ou resetar o ator, chame `ResetState()` para limpar as entradas publicadas correspondentes:
   ```cpp
   void ASBCharacter::ResetState_Implementation()
   {
       // Invalida os caches de estado associados a este Actor no barramento
       if (USBEventSubsystem* Events = GetWorld()->GetSubsystem<USBEventSubsystem>())
       {
           Events->ClearStateCacheForActor(this);
       }
   }
   ```

---

## 3. Diretrizes de Ciclo de Vida, Ticks e Ordem de Execução

- **`ISBInitializable`**: Executa a inicialização ordenada no orquestrador (`Initialize`, `Shutdown`).
- **`ISBTickable` (Tick Groups)**:
  Classes C++ puras que implementam `ISBTickable` devem tickar preferencialmente em `TG_PrePhysics`. Isso garante que as mudanças de velocidade ou forças físicas sejam computadas *antes* da simulação física da Unreal rodar no mesmo frame, prevenindo atrasos ou descompasso visual (defasagem de 1 frame).

---

## 4. Guia Prático de Sincronização via `ISBReplicable`

Para comportamentos transientes que necessitam replicar dados (ex: taxa de munição de uma arma temporária ou status de habilidade):

1. **Implemente `ISBReplicable`**:
   ```cpp
   class USBMovementBehaviorSprint : public USBMovementBehavior, public ISBReplicable
   {
       GENERATED_BODY()
   public:
       // Método de empacotamento de dados para replicação
       virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
       
       // Processamento local de recebimento de estado
       virtual void OnRep_State() override;
   };
   ```
2. **Serialização no Componente**:
   O orquestrador do componente intercepta e despacha as atualizações replicadas:
   ```cpp
   void USBMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
   {
       Super::GetLifetimeReplicatedProps(OutLifetimeProps);
       
       // Repassa a replicação para os comportamentos ativos
       if (USBBehaviorRegistryBase* Registry = GetBehaviorRegistry())
       {
           Registry->ReplicateActiveBehaviors(OutLifetimeProps);
       }
   }
   ```
3. **Validação do Servidor**:
   Sempre execute as mudanças de estado diretamente no Servidor (`Authority`), deixando que a replicação atualize as instâncias nos Clientes correspondentes de forma segura.

---

## 5. Como Criar e Utilizar Objetos Interativos Modularmente (07_SandboxInteraction)

O sistema de interação é baseado em objetos que implementam `ISBInteractableInterface` e no componente `USBInteractionComponent` anexado ao personagem.

### A. Criando um Objeto Interativo Simples (Duração 0 - Instantâneo)
Para objetos de uso instantâneo e uso irrestrito (como coletar munição ou apertar um botão simples), basta implementar as funções essenciais sem preocupar-se com tempo de espera ou exclusão mútua:

```cpp
// SBTestInstantPickup.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/SBInteractableInterface.h"
#include "SBTestInstantPickup.generated.h"

UCLASS()
class ASBTestInstantPickup : public AActor, public ISBInteractableInterface
{
	GENERATED_BODY()

public:
	virtual void Interact_Implementation(AActor* Interactor) override
	{
		// Regra de Ouro: Sempre valide se roda sob autoridade do servidor
		if (!HasAuthority()) return;

		// Lógica do pickup (ex: conceder recurso ao Interactor)
		Destroy();
	}

	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override
	{
		return NSLOCTEXT("Interaction", "PickupPrompt", "Coletar Item");
	}
};
```

### B. Criando um Objeto Interativo com Duração (Hold) e Lock de Concorrência
Para alvos compartilhados que exigem um hold progressivo e impedem que dois jogadores acessem simultaneamente (ex: baú de loot, desarmar dispositivo), armazene o `LockOwner` e valide-o de forma autoritativa:

```cpp
// SBTestLockedChest.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/SBInteractableInterface.h"
#include "SBTestLockedChest.generated.h"

UCLASS()
class ASBTestLockedChest : public AActor, public ISBInteractableInterface
{
	GENERATED_BODY()

protected:
	// Rastreia quem possui o lock no Servidor
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CurrentLockOwner = nullptr;

	UPROPERTY(EditAnywhere)
	float UnlockDuration = 2.0f;

public:
	// 1. Informa a duração necessária para o Hold
	virtual float GetInteractionDuration_Implementation(AActor* Interactor) const override
	{
		return UnlockDuration;
	}

	// 2. Verifica se o objeto já está travado por outro jogador
	virtual bool IsInteractionLocked_Implementation(AActor* Interactor) const override
	{
		return CurrentLockOwner.IsValid() && CurrentLockOwner != Interactor;
	}

	// 3. Adquire o Lock
	virtual void LockInteraction_Implementation(AActor* Interactor) override
	{
		if (HasAuthority() && !CurrentLockOwner.IsValid())
		{
			CurrentLockOwner = Interactor;
		}
	}

	// 4. Libera o Lock (Crucial: Chamar em todos os fluxos de término)
	virtual void UnlockInteraction_Implementation(AActor* Interactor) override
	{
		if (HasAuthority() && CurrentLockOwner == Interactor)
		{
			CurrentLockOwner = nullptr;
		}
	}

	// 5. Execução lógica final sob autoridade do servidor
	virtual void Interact_Implementation(AActor* Interactor) override
	{
		if (!HasAuthority()) return;
		
		// Concede a recompensa
		// ...
	}
};
```

### C. Como a UI deve reagir aos Eventos de Interação
A UI pode registrar-se no `USBEventSubsystem` para escutar e renderizar os prompts contextuais e a barra de progresso:

```cpp
void USBUIPromptWidget::InitializeWidget()
{
	if (USBEventSubsystem* EventSubsystem = GetWorld()->GetSubsystem<USBEventSubsystem>())
	{
		// Inscreve-se na detecção de foco
		EventSubsystem->SubscribeToEvent(
			FGameplayTag::RequestGameplayTag("Event.Interaction.Available"),
			ESBEventPriority::Medium,
			FSBBlueprintEventDelegate::CreateUObject(this, &USBUIPromptWidget::OnInteractionAvailable)
		);

		// Inscreve-se na limpeza de foco ou cancelamento
		EventSubsystem->SubscribeToEvent(
			FGameplayTag::RequestGameplayTag("Event.Interaction.Cleared"),
			ESBEventPriority::Medium,
			FSBBlueprintEventDelegate::CreateUObject(this, &USBUIPromptWidget::OnInteractionCleared)
		);

		// Inscreve-se no avanço do Hold
		EventSubsystem->SubscribeToEvent(
			FGameplayTag::RequestGameplayTag("Event.Interaction.Progress"),
			ESBEventPriority::Medium,
			FSBBlueprintEventDelegate::CreateUObject(this, &USBUIPromptWidget::OnInteractionProgress)
		);
	}
}

void USBUIPromptWidget::OnInteractionAvailable(FGameplayTag EventTag, UObject* Payload)
{
	if (USBInteractionAvailableEventPayload* Data = Cast<USBInteractionAvailableEventPayload>(Payload))
	{
		// Exibe prompt na tela (Data->PromptText)
		// Se Data->Duration > 0.0f, prepara a exibição da barra de progresso
	}
}

void USBUIPromptWidget::OnInteractionProgress(FGameplayTag EventTag, UObject* Payload)
{
	if (USBInteractionProgressEventPayload* Data = Cast<USBInteractionProgressEventPayload>(Payload))
	{
		// Atualiza a barra com Data->ProgressPercent (0.0f a 1.0f)
	}
}
```

### D. Nota de Extensão para Módulos de Inventário (08_SandboxInventory)
> [!IMPORTANT]
> Quando implementarmos a coleta de itens físicos no chão na fase do **08_SandboxInventory**, os Pickups físicos de itens no mundo deverão herdar de/ou implementar `ISBInteractableInterface` de forma a reaproveitar o mesmo fluxo de lock e detecção estipulados no plugin `07_SandboxInteraction`. Isso previne a ocorrência de condições de corrida em que dois jogadores disputam o mesmo loot físico no chão, eliminando duplicações indesejadas de itens.

---

## 6. Como Criar e Registrar Habilidades Baseadas no Behavior Stack (Fase 16)

Habilidades são comportamentos especializados herdando de `USBAbility` (derivado de `USBGameplayBehavior`).

### A. Criando a Classe C++ ou Blueprint
1. Crie uma classe herdando de `USBAbility`.
2. Configure as seguintes propriedades no painel de defaults (ou construtor):
   * **`AbilityTag`**: Tag identificadora exclusiva (ex: `Ability.Teleport`).
   * **`AbilityTags`**: Tags de estado ativadas no personagem enquanto a habilidade executa (ex: `State.Character.Teleporting`).
   * **`ResourceTag`**: Tag do atributo consumido (ex: `Attribute.Mana`).
   * **`ResourceCost`**: Custo numérico para a ativação (ex: `25.0f`).
   * **`CooldownDuration`**: Tempo em segundos antes de permitir reativação (ex: `5.0f`).

### B. Implementando a Lógica de Execução
Sobrescreva os ganchos do ciclo de vida:
```cpp
void UPBAbility_Teleport::Enter_Implementation(const FSBBehaviorContext& Context)
{
	Super::Enter_Implementation(Context);

	// Implemente a execução da habilidade aqui (efeitos visuais, lógica de física)
	
	// Termina a execução e ejeta a habilidade da pilha de ativação
	if (USBAbilityComponent* Comp = Cast<USBAbilityComponent>(Context.GameplayContext->Character->FindComponentByClass<USBAbilityComponent>()))
	{
		Comp->StopBehavior(AbilityTag);
	}
}

void UPBAbility_Teleport::Exit_Implementation(const FSBBehaviorContext& Context)
{
	// Limpe modificadores temporários e pare animações de habilidade
	Super::Exit_Implementation(Context);
}
```

---

## 7. Como Integrar Componentes e Atores ao Save Game System (Fase 15)

Qualquer Ator ou Componente que precise persistir seu estado entre sessões de jogo deve assinar e implementar a interface `ISBSaveInterface`.

### A. Assinando a Interface em C++
1. Adicione a dependência de `"SandboxInterfaces"` no seu `Build.cs`.
2. Adicione a herança de `ISBSaveInterface` na sua classe:
```cpp
#include "Interfaces/SBSaveInterface.h"
// ...
class MYGAME_API UMyComponent : public UActorComponent, public ISBSaveInterface
{
	GENERATED_BODY()
// ...
```

### B. Implementando a Serialização de Propriedades
Utilize `USBSavePayload` para gravar e recuperar as propriedades marcadas com o especificador `SaveGame`:

```cpp
virtual void SaveComponentData_Implementation(USBSavePayload* Payload) override
{
	// Grava o estado atual do objeto em formato binário sob uma chave única
	Payload->SerializeObject(GetPathName(), this);
}

virtual void LoadComponentData_Implementation(USBSavePayload* Payload) override
{
	// Restaura o estado a partir do buffer de bytes persistido
	Payload->DeserializeObject(GetPathName(), this);
}

virtual int32 GetSavePriority_Implementation() const override
{
	// Prioridade de Carregamento: valores maiores executam primeiro
	// Atributos = 100, Inventário = 50, Padrão = 0
	return 0; 
}

---

## 8. Como Integrar Widgets ao Barramento de Eventos (Fase 18)

O subsistema de UI dinâmico (`09_SandboxUI`) gerencia inscrições assíncronas automáticas e segurança em Split-Screen de forma centralizada em C++.

### A. Derivando das Backing Classes C++
Para criar um novo widget reativo (ex: Barra de Habilidades, Vida ou Interação), derive seu WBP de uma das classes base em C++ do plugin:
*   `USBStatusHUDWidget` (para barras de progresso vinculadas a atributos)
*   `USBInteractionPromptWidget` (para prompts de interação 3D e holds progressivos)
*   `USBAbilityBarWidget` (para slots de habilidades com cooldown cosmético)
*   `USBInventoryGridWidget` (para grids que reagem a atualizações de slots)

### B. Vinculando Elementos Visuais (`BindWidget`)
No seu Widget Blueprint, crie componentes com o especificador `meta = (BindWidget)` usando o nome correspondente declarado na classe C++ backing (ex: uma barra de progresso chamada `PB_Health`). A vinculação e a atualização do valor são feitas de forma 100% nativa.

### C. Filtro de Player Local (Anti-Spill) em C++
Para evitar o vazamento de dados de interface entre jogadores na mesma tela em split-screen, implemente o filtro de escopo chamando `GetOwningPlayerPawn()` nos manipuladores de eventos:

```cpp
void USBStatusHUDWidget::OnAttributeChanged(FGameplayTag EventTag, UObject* Payload)
{
	if (!Payload) return;
	USBAttributeChangedPayload* AttrPayload = Cast<USBAttributeChangedPayload>(Payload);
	if (!AttrPayload) return;

	// Impede que este HUD atualize se o evento for de outro Pawn (Split-Screen / Servidor)
	if (AttrPayload->TargetPawn != GetOwningPlayerPawn()) return;

	// Atualiza os percentuais baseados no MaxValue (Fail-closed)
	if (PB_Health && AttrPayload->MaxValue > 0.0f)
	{
		PB_Health->SetPercent(AttrPayload->CurrentValue / AttrPayload->MaxValue);
	}
}
```

### D. Interpolação Cosmética de Cooldowns
Evite re-consultas constantes (polling) ou sincronização por frame via rede para cooldowns de UI. Implemente interpolação cosmética estritamente local (client-side) no tick do widget com base no valor recebido no payload de disparo inicial:

```cpp
void USBAbilityBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsCooldownActive)
	{
		CooldownRemaining -= InDeltaTime; // Regressão puramente local
		if (CooldownRemaining <= 0.0f)
		{
			bIsCooldownActive = false;
			IMG_CooldownMask->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}
```
