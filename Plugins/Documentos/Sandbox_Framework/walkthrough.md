# Walkthrough: Sandbox Framework Bootstrap (v1.0.0)

Este documento registra a conclusão da fase de **Bootstrap Baseline (v1.0.0)** do **Sandbox Framework**. Estruturamos os diretórios iniciais dos plugins físicos no disco, alinhados com a especificação **SFPS v1.0.0** e o guia de desenvolvimento **SFDG v1.0.0**.

---

## 1. Módulos Inicializados no Disco (7 de 11)

Nesta fase de bootstrap, instanciamos a infraestrutura completa da fundação (4/4) combinada com um subset inicial de gameplay, apresentação e ferramentas (3/7) necessários para a baseline operacional do framework.

Os demais 4 plugins (`06_SandboxCombat`, `07_SandboxInteraction`, `08_SandboxInventory` e `10_SandboxDebug`) serão introduzidos incrementalmente nas próximas fases de desenvolvimento.

### A. Foundation (4/4 Plugins)
- **[01_SandboxCommon](file:///d:/Unreal/V1/Plugins/01_SandboxCommon/)**: Gameplay Tags nativas (`SBGameplayTags`), definição de Atributos (`FSBAttribute`) e seus modificadores dinâmicos correspondentes (`FSBAttributeModifier`), além de logs estruturados e as classes base reutilizáveis (`FSBContext`, `USBBehaviorRegistry`, `USBModifierAggregator`).
- **[02_SandboxInterfaces](file:///d:/Unreal/V1/Plugins/02_SandboxInterfaces/)**: Contratos do ciclo de vida, persistência e rede (`ISBComponentInterface`, `ISBSaveInterface`, `ISBInitializable`, `ISBTickable`, `ISBReplicable`, `ISBResettable`).
- **[03_SandboxAssets](file:///d:/Unreal/V1/Plugins/03_SandboxAssets/)**: `SBAssetManager` integrado e Data Assets de definição (`PawnData`, `ComponentSet`, `AbilitySet`).
- **[04_SandboxCore](file:///d:/Unreal/V1/Plugins/04_SandboxCore/)**: `SBComponentFactory` (resolução topológica de dependências), Message Router priorizado (`USBEventSubsystem`), `SBInputSubsystem` e classes do ciclo do jogo.

### B. Gameplay (1/4 Plugins)
- **[05_SandboxCharacter](file:///d:/Unreal/V1/Plugins/05_SandboxCharacter/)**: Contêiner genérico `ASBCharacter` e componentes injetados (`Attribute`, `State`, `Ability`). Futuramente receberá subcomponentes de Movimentação, Câmera e Animação.

### C. Presentation (1/2 Plugins)
- **[09_SandboxUI](file:///d:/Unreal/V1/Plugins/09_SandboxUI/)**: Gerenciador de camadas de widgets `SBUIManager` e HUD.

### D. Tools (1/1 Plugins)
- **[11_SandboxEditor](file:///d:/Unreal/V1/Plugins/11_SandboxEditor/)**: Módulo exclusivo do editor para validação visual e customização de painéis.

---

## 2. Padrões de Engenharia Aplicados na Baseline

- **Versionamento no Descritor**: Inserção do bloco de versionamento `"SandboxVersion"` nos metadados de todos os arquivos `.uplugin`.
- **Wrapper de Contexto Unificado (`FSBBehaviorContext`)**: Criado para encapsular as referências de gameplay (`FSBGameplayContext`) e infraestrutura (`FSBFrameworkContext`) em um único struct constante repassado aos comportamentos.
- **Modificadores Priorizados**: Inserção do membro `Priority` na estrutura `FSBModifierEntry` para controle estrito de precedências de modificadores físicos.
- **Segurança de Tipos no Contexto**: Configuração de ponteiros fortemente tipados para os subsistemas `USBAssetManager` e `USBEventSubsystem` no contexto de infraestrutura.
- **Agendamento de Testes Automatizados**: A criação da bateria inicial de testes automatizados (Unit/Integration) sob a pasta `Tests/` foi programada para a próxima iteração lógica, acompanhando a codificação dos primeiros comportamentos concretos de movimentação (Sprint/Crouch).

---

## 3. Diretrizes de Compilação

> [!TIP]
> A Unreal Engine 5.8 traz nativamente seu DotNet SDK embutido (*bundled* em `Engine/Binaries/ThirdParty/DotNet/`) para execução do UnrealBuildTool (UBT). Nenhuma instalação externa ou manual do .NET SDK 10 é necessária no Windows para compilar o projeto `V1.sln`, evitando assim potenciais conflitos de runtime do compilador.

---

## 4. Extensão do Personagem - Animação Modular (Fase 8)

Implementamos a fundação de animação baseada em Linked Anim Layers e orientada a tags para o plugin **05_SandboxCharacter**:

- **ISBAnimLayerInterface**: Interface base (`UInterface`) em C++ utilizada para validar se os sub-ABP (sub-grafos) vinculados dinamicamente cumprem o contrato de animação esperado no Sandbox Framework.
- **USBAnimLayerConfigDataAsset**: Estrutura orientada a dados (`UDataAsset`) que mapeia as tags de estado (`FGameplayTag`) para classes de AnimInstance (`TSubclassOf<UAnimInstance>`) de sub-layer e suas respectivas prioridades (`Priority`).
- **USBAnimLayerManagerComponent**: Componente orquestrador de rede que gerencia de forma dinâmica e performática a vinculação e desvinculação de layers na malha:
  - **Hitch-Free Check**: Evita micro-soluços visuais em runtime comparando a lista atual versus a nova lista ordenada de layers. Se os grafos ativos não mudaram, pula a re-vinculação.
  - **StableSort Determinístico**: Resolve colisões de funções de interface compartilhadas ligando as layers em ordem ascendente de prioridade, de modo que a de maior prioridade herde o topo da pilha de execução da UE nativamente.
  - **Coalescimento por Dirty-Flag**: Escuta as alterações de tags do `StateComponent` e agenda um rebuild único por frame no fim do tick (`TG_PrePhysics`), evitando múltiplas vinculações redundantes no mesmo frame.
  - **Guards de Inicialização**: Gerencia links de forma deferida caso o `AnimInstance` do skeletal mesh esteja nulo na inicialização inicial do jogo.

---

## 5. Extensão do Personagem - Sistema de Câmeras (Fase 9)

Implementamos a infraestrutura do sistema de câmera dinâmico e desacoplado para o plugin **05_SandboxCharacter**:

- **FSBCameraContext**: Estrutura contendo referências transientes (Character, SpringArm, CameraComponent) e DeltaTime enviadas a cada frame para atualização dos modos ativos.
- **USBCameraModeDefinition**: Data Asset estático contendo configurações de FOV, Arm Length, Socket Offset e Blend Speed configuradas por designers, incluindo a propriedade `CameraModeClass` para comportamentos especializados.
- **USBCameraMode**: Classe base operacional para modos de câmera que implementa callbacks cruciais (`Enter`, `Update`, `Exit`) e controle de prioridade estática.
- **USBCameraComponent**: Componente orquestrador que gerencia a pilha de câmera local e realiza a interpolação suave (blending) de propriedades:
  - **Coalescimento por Dirty-Flag**: Escuta as alterações de tags do `StateComponent` e agenda uma reconstrução única por frame da pilha no início de seu `TickComponent` (evitando ordenações redundantes).
  - **Ciclo de Atualização (`Update()`) Contínuo**: Todos os modos da pilha rodam `Update()` para manter seu estado dinâmico (timers, camera shake), mas apenas o topo (maior prioridade) fornece as metas para o blending.
  - **Otimização Inteligente para Replays/Spectator**: O componente ignora o tick e processamento de câmera em proxies simulados, executando apenas se o pawn for controlado localmente (`IsLocallyControlled()`) ou for o `ViewTarget` atual do `PlayerController` local daquele cliente.
  - **Ordenação determinística por StableSort**: Reordena a pilha de modos ativos deterministicamente.

---

## 6. Extensão do Atributo & Sistema de Combate (Fase 10)

Implementamos a predição transacional de atributos e a inicialização física do sistema de combate no plugin **06_SandboxCombat**:

- **Predição de Atributos Jitter-Free (`USBAttributeComponent`)**:
  - **Transações por PredictionId**: Implementamos predição de recursos de consumo discreto (Munição, Mana) onde o cliente deduz visualmente o offset em seu HUD local associando a um `PredictionId` sequencial.
  - **Upsert do Array Replicado**: O servidor confirma consumos fazendo o *upsert* (update ou insert) de um ID confirmado em `ConfirmedPredictions` (`TArray<FSBConfirmedPredictionEntry>`), limitando o tamanho do array e eliminando vazamentos de dados na rede.
  - **Sincronização em Mesmo Frame**: OnReps de atributos e confirmações rodam juntos no mesmo frame de rede, limpando offsets locais e garantindo visual 100% livre de flicker ou double-dips.
  - **Timeout Guard**: Varredura contínua no tick que descarta transações mais velhas que 2.0 segundos, executando rollbacks implícitos sob perda severa de conexão.
  - **Consolidação de Escritas (`ModifyAttributeBaseValue`)**: Centralizamos todas as escritas físicas de valores em um único helper para garantir que mapas de cache C++ e arrays de replicação de rede nunca divirjam (corrigindo o bug latente de replicação de regeneração).

- **Módulo de Combate (`06_SandboxCombat`)**:
  - **Descritor `.uplugin` & Build.cs**: Inicializados no disco com as dependências unidirecionais topológicas estritas sobre a base de gameplay (`05_SandboxCharacter`).
  - **USBCombatComponent**: Orquestrador central que gerencia inventário de behaviors de armas, taxa de disparo (cooldowns de Cadência), RPCs de disparo (`ServerRequestFire`) e rollbacks em caso de cheat (`ClientRollbackFire`).
  - **ExclusivityGroup Ejection**: A ativação de armas em slots (ex: Rifle Primary) ejeta de forma automática e frame-perfect outras armas ativas no mesmo grupo de exclusão (ex: Pistol Secondary), realizando o weapon swap de forma data-driven.
  - **USBWeaponBehavior / Hitscan**: Lógica genérica de armas. O `USBWeaponBehaviorHitscan` executa traços físicos autoritativos no servidor (`LineTraceSingleByChannel`) e aplica danos consumindo diretamente da tag de saúde (`Attribute.Character.Health`) do componente de atributo do alvo.

- **Suíte de Testes Automatizados (`SBCombatTests`)**:
  - **Cenário 1**: Valida a predição local de munição, confirmação jitter-free e esvaziamento da fila de transação.
  - **Cenário 2 (Cheat Protection)**: Simula o cliente tentando disparar sem munição, e verifica o disparo do rollback restaurando a munição e limpando o behavior de disparo.
  - **Cenário 3 (Exclusivity Group Ejection)**: Valida a ejeção e transição perfeita de Rifle para Pistola no mesmo frame baseando-se no slot de exclusão.

---

## 7. Validação Completa da Suíte de Testes Automatizados (Fase 11 / v1.2.0)

Concluímos com sucesso a correção, refatoração arquitetural e estabilização de toda a suíte de testes unitários e de rede para os módulos de **Movimentação**, **Câmera**, **Animação** e **Combate**, alcançando **100% de cobertura verde (Exit Code: 0)**.

Seguindo os mais altos padrões de design de testes, removemos todos os desvios de execução de teste (`GIsAutomationTesting` ou classes mock de controller) que alteravam o fluxo do código de produção real, ajustando ao invés disso o ambiente de inicialização dos testes:

### Melhorias Arquiteturais e de Design nos Testes:
- **Remoção de Bypasses no Código de Produção**:
  - Revertemos o desvio de colisão de Crouch no [`SBMovementBehaviorCrouch.cpp`](file:///d:/Unreal/V1/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Movement/Behaviors/SBMovementBehaviorCrouch.cpp). O comportamento agora executa a checagem nativa de produção `CanCrouchInCurrentState()` sem bypasses.
  - Para permitir o Crouch no ambiente headless vazio de testes, configuramos o modo de movimentação do Pawn para `MOVE_Walking` nos arquivos de spec de teste.
  - Removemos o desvio do `GIsAutomationTesting` no override de `IsLocallyControlled()` no [`SBCharacter.cpp`](file:///d:/Unreal/V1/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Character/SBCharacter.cpp). O override agora reflete estritamente a lógica limpa de produção de Lyra: `IsPlayerControlled() && Super::IsLocallyControlled()`.
- **Eliminação de Classes Mock Excedentes**:
  - Excluímos as classes `ASBMockLocalPlayerController` e `ASBMockRemotePlayerController` de [`SBMovementComponent.h`](file:///d:/Unreal/V1/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Public/Components/SBMovementComponent.h). Os testes agora utilizam a classe de produção nativa `APlayerController`.
- **Configuração Realista do Ambiente de Teste**:
  - Para fazer o `IsLocallyControlled()` nativo funcionar perfeitamente em modo headless sem interagir com telas, criamos instâncias de `ULocalPlayer` anexadas a `GEngine` como seu Outer e as atribuímos diretamente ao `Player` de cada Controller local de teste.
  - Para atender às validações nativas do motor (como `APawn::IsPlayerControlled()` que em versões modernas verifica a existência de um PlayerState não-bot), spawnamos instâncias de `APlayerState` no mundo de teste e as vinculamos a `Controller->PlayerState` nos specs de teste.
- **Proteção de API Pública**:
  - O método de utilidade de possessão virtual `Test_Possess(AController*)` foi alterado para `private` em [`SBCharacter.h`](file:///d:/Unreal/V1/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Public/Character/SBCharacter.h).
  - Declaramos `friend class FSBNetworkTestsSpec;` na classe do personagem, garantindo que o atalho de possessão esteja disponível exclusivamente para a suíte de automação sem poluir a API de produção do framework.
- **USBBehaviorRegistry Concrete Class**:
  - A classe `USBBehaviorRegistry` em [`SBCommonTypes.h`](file:///d:/Unreal/V1/Plugins/01_SandboxCommon/Source/SandboxCommon/Public/Types/SBCommonTypes.h) permanece concreta (não-abstrata) uma vez que ela é instanciada diretamente via `NewObject` pela lógica de produção ativa em [`SBMovementComponent.cpp`](file:///d:/Unreal/V1/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Components/SBMovementComponent.cpp).

### Resultados da Suíte Automatizada:
Todas as baterias de testes estão executando e passando de forma 100% estável:
1. **Sandbox.Character.Animation** (2/2 Passando)
2. **Sandbox.Character.Camera** (2/2 Passando)
3. **Sandbox.Character.Movement** (4/4 Passando)
4. **Sandbox.Character.Network** (5/5 Passando)
5. **Sandbox.Combat** (3/3 Passando)

**Status Final: 100% Verde (Zero Falhas, Zero Ensures, Compilação Segura Sequencial com MaxParallelActions=1).**

---

## 8. Extensão de Interação Modular (Fase 12 - SandboxInteraction)

Implementamos o sistema de interações físicas e de rede sob hold-to-interact, autoridade centralizada de locks e proteção contra condições de corrida no plugin **07_SandboxInteraction**:

### A. Componente de Interação (`USBInteractionComponent`):
- **Detecção e Foco por Trace**: Varredura por linha ou varredura de esfera com base em configurações (`InteractionRange`, `bUseLineTrace`, `TraceRadius`), publicando eventos de detecção (`Event.Interaction.Available` e `Event.Interaction.Cleared`) com o payload correspondente (prompt de interação, duração) obtidos a partir dos contratos implementados no alvo.
- **Roteamento de Interface Seguro**: Adicionamos métodos auxiliares robustos que encapsulam o roteamento das chamadas da interface `ISBInteractableInterface` para evitar falhas do vtable do subsistema de reflexão da engine quando invocado em instâncias de teste C++ puras spawnadas dinamicamente. Os helpers tentam um cast estático de C++ para `ISBInteractableInterface*` antes de fazer o fallback via `ProcessEvent` do Unreal.
- **Suporte Completo a Retenção (Hold-to-Interact)**: Lógica com accumulators de DeltaTime local no cliente para hold-to-interact, acompanhado do respectivo acompanhamento visual (`GetHoldProgressPercent()`) e publicação contínua de eventos de progresso (`Event.Interaction.Progress`).
- **Locks no Servidor com Autoridade Estrita**: O servidor valida a distância do jogador (`ServerStartInteract` e `ServerCompleteInteract`) contra trapaças/desconexões, e realiza um lock de exclusividade lógica no ator alvo (`LockInteraction` e `UnlockInteraction`), mitigando cenários de concorrência onde dois jogadores tentam interagir com o mesmo baú/porta ao mesmo tempo (Cenário 5).
- **ClientCancelInteraction Simétrico**: No caso de falhas de rede, cheat ou desvio de distância do hold, o servidor força o cancelamento e restaura o estado do personagem de forma simétrica e limpa.

### B. Resultados da Suíte de Testes Automatizados (`Sandbox.Interaction`):
- **Cenário 1 (Detecção e Foco por Trace)**: Valida que a rotação e posicionamento da mira ativa detectam ou limpam o foco do objeto sob a mira, emitindo os payloads e tags esperados.
- **Cenário 2 (Interação Instantânea/Discreta)**: Valida interações sem duração (hold = 0.0s), as quais disparam imediatamente o evento `Interact` e liberam o lock.
- **Cenário 3 (Interação por Retenção/Hold-to-Interact)**: Valida o acúmulo de progresso de hold ao longo do tempo usando ticks manuais consistentes com a taxa de atualização máxima (`MaxDeltaTime`) da engine, executando a interação física somente no término do hold.
- **Cenário 4 (Interrupção por Distância/Network Safety)**: Valida que teleportar o personagem para longe do alvo do hold durante a interação força a interrupção local e o cancelamento autoritativo pelo servidor.
- **Cenário 5 (Cenário de Corrida de Alvo Compartilhado/Race Condition)**: Valida a disputa pelo mesmo objeto entre Jogador 1 (Servidor) e Jogador 2 (Cliente). O Jogador 2 tem sua requisição rejeitada pelo servidor porque o alvo já está trancado pelo Jogador 1.

**Status Final de Todos os Testes: 100% Verde (21/21 Testes Passando em toda a suíte do Sandbox).**

---

## 9. Sistema de Inventário e Slots Replicados (Fase 13 - SandboxInventory)

Implementamos a infraestrutura completa de inventário autoritativo em rede no plugin **08_SandboxInventory**, integrando-o de forma desacoplada aos demais plugins de gameplay:

### A. Componentes e Estrutura de Itens:
- **Padrão Definition/Instance/Fragment**: Implementamos os itens utilizando `USBItemDefinition` (Primary Data Asset contendo metadados imutáveis e `MaxStackCount`), `USBItemInstance` (UObject contendo estados replicados de quantidade/tags transientes) e `USBItemFragment` (classes instanciadas para especializar o uso de itens de forma polimórfica).
- **Fast Array Replication**: Criamos a estrutura `FSBInventoryList` (derivada de `FFastArraySerializer`) contendo slots `FSBInventoryEntry`. O componente `USBInventoryComponent` registra o array e realiza `ReplicateSubobjects` para todos os clientes conectados.
- **Fila de Ativação e Timeout Guard**: Adicionamos uma fila temporária no tick do cliente que retém a notificação visual do slot enquanto a referência UObject do subobjeto do item não é totalmente resolvida em rede. Aplicamos o **Timeout Guard de 2.0 segundos** para descartar entradas órfãs sob interrupção de rede.
- **Desacoplamento Completo via Message Router**: A conexão entre o Inventário e o Combate (`06_SandboxCombat`) ocorre de forma 100% plana. Ao equipar ou desequipar itens, o inventário publica os eventos `Event.Inventory.ItemEquipped` / `ItemUnequipped` com seus payloads. O `USBCombatComponent` assina os eventos e usa **reflexão dinâmica em runtime** para ler as propriedades do fragmento de equipamento, instanciando e ejetando os `WeaponBehavior` simetricamente sem possuir dependência de compilação com o plugin de inventário.

### B. Resultados da Suíte de Testes Automatizados (`Sandbox.Inventory`):
- **Cenário 1 (Adição e Stacking)**: Valida que itens empilháveis incrementam slots existentes até `MaxStackCount`, dividindo transações subsequentes em novos slots.
- **Cenário 2 (Fila de Ativação e Timeout)**: Verifica que slots com referências nulas enfileiram na ativação e são ejetados com log de aviso após 2 segundos sem resolver.
- **Cenário 3 (Integração de Equipar)**: Valida que equipar um Rifle publica a tag no Message Router, ativando e configurando o `WeaponBehavior` correspondente no `CombatComponent` via reflexão.
- **Cenário 4 (Loot Dispute)**: Testa disputa de anel valendo-se da exclusão mútua de locks de interação, garantindo que o segundo jogador seja rejeitado e o loot não seja duplicado.
- **Cenário 5 (Desequipamento Simétrico)**: Verifica que desequipar o rifle do inventário dispara a remoção e encerramento limpo do behavior correspondente da pilha de combate.

**Status Final de Todos os Testes: 100% Verde (26/26 Testes Passando em toda a suíte do Sandbox).**

---

## 10. Consolidação do USBBehaviorStackComponent (Fase 14 - v1.4.0)

Consolidamos com sucesso o mecanismo de pilha ativa de comportamentos de movimentação (`USBMovementComponent`) e de combate (`USBCombatComponent`) em um único componente genérico comum e robusto, o `USBBehaviorStackComponent`, localizado no plugin core **01_SandboxCommon**.

### A. Componentes Consolidados e Proteção de Reentrância:
- **Base Comum Data-Driven (`USBGameplayBehavior` e `USBGameplayBehaviorDefinition`)**: Extraímos as definições estáticas (`StackPriority`, `ExclusivityGroup`, `RequiredTags`, `BlockedTags`) e a lógica de instância (`Initialize`, `Enter`, `Exit`, `Update`, `CanEnter`, `CanExit`) para classes polimórficas comuns.
- **USBBehaviorStackComponent**: Orquestrador comum que gerencia a pilha ativa (`ActiveBehaviors`) e os comportamentos disponíveis (`AvailableBehaviors`). 
- **Prevenção Simétrica de Reentrância (`FSBStackMutationGuard`)**: Implementamos a mutabilidade segura da pilha em loops de saída em cascata. O guard rastreia a profundidade de mutação (`StackMutationDepth`). Se novas ativações ou ejeções ocorrerem durante a execução dos hooks `Enter`/`Exit`, elas são retidas em `DeferredEntries` e `DeferredExits` e resolvidas deterministicamente de forma sequencial (*flat loop*) quando a profundidade retorna a zero, mitigando crashes por stack overflow.
- **Roteamento de Interfaces no Mundo Headless**: Corrigimos o setup de inicialização de comportamentos em ambientes de testes automatizados headless (onde o sistema de reflexão UObject da Unreal pode falhar ao validar interfaces dinâmicas) substituindo as chamadas de reflexão lentas por casts estáticos de C++ (`Cast<ISBCharacterInterface>` e `Cast<ISBStateComponentInterface>`) com fallbacks seguros para reflexão (`ImplementsInterface` e `Execute_`).
- **Resolução de Conflitos e Shadowing**: Renomeamos as propriedades sombreadas de subclasses (`Definition` -> `MovementDefinition` / `WeaponDefinition`) e corrigimos as diretivas `UFUNCTION` repetidas em overrides virtuais C++ que geravam erros do compilador Unreal Header Tool (UHT).

### B. Adaptação dos Plugins Especializados:
- **USBMovementComponent & USBCombatComponent**: Herdaram diretamente da base comum, eliminando centenas de linhas de código duplicado e herdando as rotinas genéricas de Tick e ordenação.
- **RPC Symmetrical Synchronization (`OnBehaviorEjected`)**: Implementamos o hook virtual de ejeção de rede que propaga flags de skip de replicação (`bSkipServerNotify`/`bSkipClientNotify`) corretamente, permitindo que componentes de movimentação e combate disparem seus respectivos RPCs nativos (`ClientStopBehavior`, `ServerRequestFire`, etc.) sem gerar loops infinitos na rede.

### C. Bateria de Testes Unificada:
Criamos o arquivo de teste [`SBSourceBehaviorStackTests.cpp`](file:///d:/Unreal/V1/Plugins/01_SandboxCommon/Source/SandboxCommon/Private/Tests/SBSourceBehaviorStackTests.cpp) em `01_SandboxCommon` e executamos a suíte de automação completa:
- **Should sort behaviors by descending priority**: Valida a ordenação correta das prioridades na base comum.
- **Should eject conflicting behavior in same ExclusivityGroup**: Testa a ejeção determinística do behavior ativo quando outro behavior do mesmo grupo e de maior prioridade entra na pilha.
- **Should defer reentrant requests correctly in exit cascades**: Estressa a pilha com ativações reentrantes profundas, verificando que o flat loop as adia e executa sequencialmente sem estourar a memória.
- **Suíte Legada Integrada**: Todos os 26 testes de movimentação avançada, rede, câmera, animação, combate, interação e inventário continuam passando perfeitamente.

**Status Final de Todos os Testes: 100% Verde (27/27 Testes Passando com Exit Code: 0).**

---

## 11. Sistema de Persistência e Save Game (Fase 15 - USBSaveSubsystem)

Implementamos a infraestrutura completa de persistência e save/load de jogo no **Sandbox Framework** baseando-se no contrato C++ `ISBSaveInterface`, integrando-o de forma autoritativa no servidor aos componentes de Atributos e Inventário.

### A. Componentes e Estrutura de Salvamento:
- **USBSaveSubsystem (Desacoplamento Base Abstrata)**: Criado o subsistema abstrato base em [`02_SandboxInterfaces`](file:///d:/Unreal/V1/Plugins/02_SandboxInterfaces/Source/SandboxInterfaces/Public/Subsystems/SBSaveSubsystem.h) herdando de `UGameInstanceSubsystem` com a assinatura `UCLASS(Abstract, BlueprintType)`. Isso permite a outros plugins (como `01_SandboxCommon`) resolverem referências do save subsystem estaticamente em compile-time via `GetSubsystem<USBSaveSubsystem>()`, sem dependência circular com o módulo concreto de persistência.
- **USBSaveSubsystemConcrete (Varredura do Mundo)**: Implementada a classe concreta em [`04_SandboxCore`](file:///d:/Unreal/V1/Plugins/04_SandboxCore/Source/SandboxCore/Public/Subsystems/SBSaveSubsystemConcrete.h), a qual varre todos os atores do mundo (`TActorIterator`) e seus componentes associados. Se um ator ou componente implementar `ISBSaveInterface`, seus ganchos polimórficos de serialização (`SaveComponentData` / `LoadComponentData`) são executados.
- **USBSavePayload & FSBSaveObjectData**: O payload genérico utiliza `FObjectAndNameAsStringProxyArchive` com o sinalizador `ArIsSaveGame = true` para filtrar e serializar propriedades `SaveGame` em um buffer de bytes transiente. Para contornar a rejeição do Unreal Header Tool (UHT) a `TMap`s com coleções complexas no campo do valor, envelopamos os dados binários na struct C++ `FSBSaveObjectData`.
- **Persistência de Atributos (`USBAttributeComponent`)**: Salvamento e carregamento autoritativo (`HasAuthority()`). O carregamento de atributos lê a alteração serializada e a propaga chamando `ModifyAttributeBaseValue` para cada atributo de forma individual, garantindo que os rollbacks de predição e o HUD local mantenham a sincronização perfeita sem double-dips ou dessincronização de rede.
- **Persistência de Inventário (`USBInventoryComponent`)**: Salvamento e carregamento autoritativo (`HasAuthority()`). Salva o caminho do data asset do item (`USBItemDefinition`), a quantidade (`StackCount`) e as tags dinâmicas transientes (`DynamicTags`) associadas à instância (ex: `State.Item.Broken`). Ao carregar, o servidor reconstrói as instâncias de itens na rede chamando `ServerAddItem`.

### B. Suíte de Testes Automatizados (`SBSaveTests.cpp`):
A suíte de testes foi alocada no plugin [`08_SandboxInventory`](file:///d:/Unreal/V1/Plugins/08_SandboxInventory/Source/SandboxInventory/Private/Tests/SBSaveTests.cpp) (para resolver todas as inclusões de componentes sem dependências circulares), validando o fluxo completo:
- **World Context Setup**: Instanciamos um `UGameInstance` autônomo e criamos um `FWorldContext` no `GEngine` associado ao `TestWorld`, assegurando que a chamada `GetWorld()` no subsystem resolva perfeitamente no ambiente de testes headless.
- **Prevenção de Colisões Síncronas**: Implementamos a limpeza de atores e componentes em testes sequenciais chamando `Rename` nos atores e componentes antigos antes da chamada de destruição, liberando seus nomes de forma frame-perfect para a nova iteração.
- **Validação de Restauração de Atributos**: Modificamos e persistimos os campos base da struct `FSBAttribute` (as quais marcamos com o especificador `SaveGame` em [`SBCommonTypes.h`](file:///d:/Unreal/V1/Plugins/01_SandboxCommon/Source/SandboxCommon/Public/Types/SBCommonTypes.h) para permitir serialização em baixo nível), comprovando que o carregamento reverte o estado do personagem para os valores gravados.
- **Validação de Restauração de Tags Dinâmicas**: Provamos que itens que contêm tags de estado temporárias (ex: tag `BrokenTag`) no inventário salvam e reconstroem essas tags perfeitamente ao carregar a sessão.

**Status Final de Todos os Testes: 100% Verde (28/28 Testes Passando com Exit Code: 0).**

---

## 12. Sistema de Habilidades Baseado no Behavior Stack (Fase 16 - v1.6.0)

Implementamos o **Sistema de Habilidades** completo herdando diretamente da fundação consolidada do `USBBehaviorStackComponent` e `USBGameplayBehavior`, provendo predição em rede, gerenciamento transacional de recursos e suporte ao Enhanced Input.

### A. Herança Limpa e Validação de Recursos:
- **USBAbility (Gameplay Behavior Especializado)**: Herdado de `USBGameplayBehavior`, ganhando automaticamente toda a proteção de reentrância em cascata, tags exigidas/bloqueadas e grupos de exclusão mútua. Inclui suporte a `CooldownDuration`, `ResourceTag` (ex: `Attribute.Mana`) e `ResourceCost`.
- **USBAbilityComponent (Behavior Stack Replicado)**: Herdado de `USBBehaviorStackComponent`, integrando os RPCs de rede do ciclo de ativação de habilidades. `USBAbility` não é replicado como subobjeto (acoplamento zero de rede), dependendo de tags e cooldowns para sincronizar estado na rede.
- **Enhanced Input Mapping**: Mapeamento dinâmico entre tags de input (`InputTag`) e habilidades (`AbilityTag`), executado em `SetupPlayerInputComponent` através do dispatch `BindInputActions` de forma data-driven.
- **Ordem de Operações Preditiva Segura**:
  - O `RequestBehavior` sobrescrito no cliente local valida e consome o recurso (*predictive consumption*) no `USBAttributeComponent` associado a um `PredictionId` **antes** de chamar a execução da pilha base (`Super::RequestBehavior()`).
  - O servidor associa o `PredictionId` recebido do cliente a uma variável transiente (`CurrentServerPredictionId`) para evitar geração local dessincronizada de IDs e prevenir consumo duplicado de recursos na rede.
  - Se a pilha base rejeitar a ativação (por tags de bloqueio ou exclusão) ou sob falha, o cliente realiza a limpeza da predição local enquanto o servidor executa o **rollback simétrico** reembolsando o valor base do recurso diretamente na autoridade (`SetAttributeBaseValue`).
- **FSBCooldownList (Replicação de Cooldowns)**: Implementado como uma lista de replicação em rede baseada em `FFastArraySerializer`, garantindo sincronização jitter-free de cooldowns ativos sem replicação desnecessária de subobjetos de UObject pesados.

### B. Correção de Acoplamentos de Teste e Robustez de C++:
- **Decoupling dos Testes da Fundação**: Para eliminar o acoplamento circular entre Foundation (`SandboxCore`) e extensões de Gameplay:
  - Os testes de persistência do inventário foram isolados em `08_SandboxInventory` (`SBInventorySaveTests.cpp`).
  - Os testes de persistência de atributos/states core foram isolados no plugin de personagens `05_SandboxCharacter` (`SBSaveTests.cpp`).
  - O plugin `04_SandboxCore` permanece no nível base, com zero acoplamento físico ou lógico com inventário ou personagens.
- **Resolução de Casts em Mundos Headless**: Substituímos as chamadas de interface do Blueprint VM (`ISBCharacterInterface::Execute_GetAttributeComponent`) por robustos casts C++ estáticos (`Cast<ISBCharacterInterface>`), contornando falhas no vtable da Unreal em mundos unitários headless.
- **Registro Dinâmico de Tags nos Testes**: Evitamos poluição estática na lista de tags de produção registrando as tags de input de teste de forma temporária na inicialização (`BeforeEach`) de `SBAbilityTests.cpp` usando o `UGameplayTagsManager`.

### C. Resultados da Suíte de Testes Automatizados (`Sandbox.Character.Abilities`):
- **Cenário 1 (Ativação e Cooldown Replicado)**: Valida que a habilidade ativa, adiciona tags de estado ao personagem, registra o cooldown no array replicado do componente e bloqueia re-ativações até o cooldown expirar.
- **Cenário 2 (Consumo e Rollback Transacional)**: Valida que habilidades consomem mana localmente com `PredictionId`. Sob falta de recurso ou rejeição de ativação por tags de bloqueio, simula o rollback seguro do servidor devolvendo o recurso de forma invisível ao jogador.
- **Cenário 3 (Enhanced Input Mapping)**: Verifica que simulações de triggers de press de input ativam as habilidades correspondentes no componente orquestrador de forma frame-perfect.

**Status Final de Compilação e Suíte de Testes (Fase 16): 100% Sucedido e Verde (31 de 31 testes unitários e de integração concluídos com sucesso no V1Editor).**

---

## 13. Gameplay Debugger e Telemetria (Fase 17 - v1.7.0)

Implementamos o plugin **Gameplay Debugger** (`10_SandboxDebug`) com desacoplamento total das extensões de gameplay usando a interface de reflexão/telemetria `ISBDebugInterface`.

### A. Desacoplamento Arquitetural (Manifesto Garantido):
- **Isolamento de Compilação**: O plugin `10_SandboxDebug` possui dependência única e exclusiva de `02_SandboxInterfaces` (e infraestrutura base `04_SandboxCore`), sem qualquer link de compilação C++ ou inclusão de cabeçalho contra `05_SandboxCharacter`, `06_SandboxCombat`, `07_SandboxInteraction` ou `08_SandboxInventory`.
- **ISBDebugInterface**: Declarada no plugin `02_SandboxInterfaces`. Utiliza a struct leve `FSBDebugLine` para representar metadados estruturados (Label, Value, bIsHeader) evitando o vazamento de ponteiros de objetos internos.
- **Auto-Descrição via Componentes**:
  - `USBBehaviorStackComponent` expõe sua pilha ativa de behaviors, profundidade de mutações e queues de mutação adiada (`DeferredEntries`/`DeferredExits`).
  - `USBAttributeComponent` expõe atributos registrados (Base vs Current), contagem de modificadores ativos e predições locais pendentes.
  - `USBStateComponent` expõe tags de estado ativas e preditas.
  - `USBAbilityComponent` expõe cooldowns ativos em segundos decrescentes e mapeamento de inputs.
  - `USBInventoryComponent` expõe o grid de slots de itens de forma síncrona aos dados replicados.
  - `USBCombatComponent` expõe armas disponíveis e ativas.
  - `ASBTestInteractableActor` expõe a duração de interação, contagem de acessos e o ator que possui o lock atual.

### B. Integração ao GDT (Gameplay Debugger Subsystem):
- **Coleta Autoritativa (`CollectData`)**: Varre em tempo de execução os componentes do ator inspecionado na mira do jogador (tanto personagens quanto atores interativos como baús). Filtra e invoca dinamicamente `ISBDebugInterface::Execute_GetDebugDescription` para quem assina o contrato.
- **Replicação Eficiente via DataPacks**: Utiliza o pipeline nativo `SetDataPackReplication` para replicar de forma otimizada os dados compilados no servidor para renderização local.
- **Apresentação Formatada (`DrawData`)**: Utiliza o markup nativo do GDT canvas para desenhar títulos estruturados em ciano (`{cyan}`) e pares de telemetria alinhados em branco (`{white}`) sob fundo escuro.
- **Compilação Condicional**: Todo o ciclo do módulo e arquivos C++ associados são condicionados via `#if WITH_GAMEPLAY_DEBUGGER` prevenindo vazamento de stubs em Shipping builds.

**Status Final de Compilação e Suíte de Testes (Fase 17): 100% Sucedido e Verde (31 de 31 testes unitários e de integração concluídos com sucesso no V1Editor).**

---

## 14. Interface Dinâmica e Desacoplamento de UI (Fase 18 - v1.8.0)

Implementamos a infraestrutura para **UI Dinâmica** (`09_SandboxUI`), conectando widgets visuais ao barramento de eventos assíncronos (`USBEventSubsystem`) sob acoplamento estático zero, prevenção de vazamento de escopo e testes de idempotência e limpeza automatizados.

### A. Estrutura de Eventos Core (`04_SandboxCore`):
- **Event Payloads Centralizados**: Criado o arquivo [`SBEventPayloads.h`](file:///D:/Unreal/V1/Plugins/04_SandboxCore/Source/SandboxCore/Public/Subsystems/SBEventPayloads.h) declarando as classes de payload derivadas de `UObject` para permitir Garbage Collection e compatibilidade nativa com Blueprints (UMG):
  - `USBPawnEventPayload`
  - `USBAttributeChangedPayload` (com `AttributeTag`, `BaseValue` e `CurrentValue`)
  - `USBInteractionAvailableEventPayload` e `USBInteractionProgressEventPayload` (ambos contendo `TargetPawn` para filtragem)
  - `USBInventoryEventPayload` (contendo `TargetPawn` e `ItemInstance` genérico como `UObject*` para acoplamento C++ zero)
  - `USBCooldownEventPayload`
- **Idempotência no Event Subsystem**: Atualizado o método `SubscribeToEvent` do [`SBEventSubsystem.cpp`](file:///D:/Unreal/V1/Plugins/04_SandboxCore/Source/SandboxCore/Private/Subsystems/SBEventSubsystem.cpp) para buscar delegates existentes antes de registrar a inscrição, prevenindo assinaturas duplicadas acidentais na mesma tag.

### B. Emissão de Eventos e Throttling:
- **Atributos & Habilidades (`05_SandboxCharacter`)**:
  - `SBAttributeComponent` assina seu delegate dinâmico `OnAttributeChanged` e publica telemetria na tag `Event.Attribute.Changed`.
  - `SBAbilityComponent` emite `Event.Ability.CooldownStarted` e gerencia a expiração de cooldowns ativos em seu `TickComponent` para disparar `Event.Ability.CooldownEnded` no frame exato.
- **Throttling a 60 Hz em Interações (`07_SandboxInteraction`)**:
  - Em `SBInteractionComponent`, implementamos um acumulador de delta de tempo no Tick para limitar os disparos de `Event.Interaction.Progress` a uma taxa máxima de **60 Hz** (intervalo `>= 0.01667s`), reduzindo re-renders excessivos em widgets Slate/UMG.
- **Compatibilidade Canônica em Inventários (`08_SandboxInventory`)**:
  - Preservamos os quatro eventos canônicos (`ItemAdded`/`ItemRemoved`/`ItemEquipped`/`ItemUnequipped`) no `SBInventoryComponent`, permitindo que grids de UI assinem as modificações de forma individualizada.
  - Registramos nativamente todas as novas tags de eventos de inventário no `StartupModule` de `SandboxInventoryModule.cpp` para consistência e prevenção de falhas de tags no carregamento autônomo.
  - **Correção de Use-After-Free**: Corrigido bug crítico de acesso de memória (Access Violation) no `ServerRemoveItem()` reordenando a publicação de atualizações de slot antes da exclusão física dos elementos no array de entries.

### C. Ciclo de Vida do Widget e Filtro de Escopo Local (`09_SandboxUI`):
- **Auto-Unsubscribe síncrono**: `USBUserWidget` gerencia um array transiente de `FSBWidgetEventSubscription` (armazenando tag + delegate de blueprint) e executa automaticamente desinscrições cirúrgicas e seguras no `NativeDestruct()`.
- **Filtro de Escopo Local (Anti-Spill)**: Implementamos o método helper `GetOwningPlayerPawn()` no `USBUserWidget`. Os widgets visuais do barramento utilizam esse helper para comparar se o `TargetPawn` do payload do evento corresponde ao Pawn controlado localmente, impedindo o vazamento de dados de interface entre clientes locais em Listen Server ou split-screen.
- **Subsystem Manager (`USBUIManager`)**: Herdado de `ULocalPlayerSubsystem` para garantir o ciclo de vida e acoplamento nativo por jogador do HUD e das camadas de widgets (HUD, Menu, Popup, Notification).
- **Fallback no HUD**: `SBHUD` implementa fallback seguro para instanciar a classe de HUD base configurada em `MainHUDWidgetClass` quando as propriedades do Pawn estão indisponíveis no editor.

### D. Resultados da Suíte de Testes Automatizados (`Sandbox.UI.WidgetEvents`):
Criamos a suíte de testes de UI [`SBUITests.cpp`](file:///D:/Unreal/V1/Plugins/09_SandboxUI/Source/SandboxUI/Private/Tests/SBUITests.cpp) cobrindo os seguintes cenários de conformidade:
- **should be idempotent and auto-unsubscribe cleanly**: Valida que assinar o mesmo delegate duas vezes dispara apenas 1 evento no barramento (idempotência), e que a destruição do widget remove todas as escutas ativas.
- **should filter events based on TargetPawn matching widget's possessed pawn**: Valida que o widget ignora eventos cujo `TargetPawn` não corresponde ao seu Pawn controlado localmente (usando o suporte a `bMockOwningPawn` em ambiente de testes unitários).
- **Suíte Legada e Nova Suíte Integrada**: Todos os 32 testes do Sandbox Framework rodam com sucesso absoluto.

### E. Classes de Suporte C++ (Backing Classes) para Widgets UMG:
Para permitir auditoria de código robusta e simplificar o trabalho do desenvolvedor no editor (evitando programação visual em gráficos espaguete de Blueprints), implementamos a lógica de controle completa em C++ no plugin `09_SandboxUI`:
*   **[`USBStatusHUDWidget`](file:///D:/Unreal/V1/Plugins/09_SandboxUI/Source/SandboxUI/Public/Widgets/SBStatusHUDWidget.h)**:
    *   Vincula dinamicamente componentes de barra de progresso `PB_Health` e `PB_Mana` via especificação `meta = (BindWidget)`.
    *   Assina `Event.Attribute.Changed` e executa um guard contra payloads nulos ou casts inválidos.
    *   Filtra pelo Pawn possuído localmente e calcula a proporção exata de preenchimento (`CurrentValue / MaxValue`) atualizando a porcentagem na tela síncronamente.
*   **[`USBInteractionPromptWidget`](file:///D:/Unreal/V1/Plugins/09_SandboxUI/Source/SandboxUI/Public/Widgets/SBInteractionPromptWidget.h)**:
    *   Vincula `TXT_Prompt` (bloco de texto) e `PB_HoldProgress` (barra de progresso).
    *   Inscreve-se nos eventos `Available`, `Cleared` e `Progress`.
    *   Gerencia os estados de visibilidade (*HitTestInvisible* vs *Collapsed*) e atualiza o progresso do hold de forma reativa a partir do payload `ProgressPercent`.
*   **[`USBAbilityBarWidget`](file:///D:/Unreal/V1/Plugins/09_SandboxUI/Source/SandboxUI/Public/Widgets/SBAbilityBarWidget.h)**:
    *   Vincula `IMG_CooldownMask` e `TXT_CooldownTime`.
    *   Inscreve-se em `CooldownStarted` e `CooldownEnded`.
    *   **Ticking de Cooldown Cosmético**: Implementa interpolação estritamente local (client-side) em `NativeTick` a partir da duração capturada inicialmente, reduzindo qualquer sobrecarga de tráfego de rede ou queries repetitivas ao servidor.
*   **[`USBInventoryGridWidget`](file:///D:/Unreal/V1/Plugins/09_SandboxUI/Source/SandboxUI/Public/Widgets/SBInventoryGridWidget.h)**:
    *   Escuta `Event.Inventory.SlotUpdated` e repassa a notificação para a Blueprint via evento implementável `BP_OnSlotUpdated(UObject* ItemInstance)`. O Blueprint do designer faz o cast dinâmico seguro de `ItemInstance` para `USBItemInstance` no UMG para popular imagens e textos de slot de forma visual.

**Status Final de Compilação e Suíte de Testes (Fase 18): 100% Sucedido e Verde (32 de 32 testes concluídos com sucesso no V1Editor - EXIT CODE: 0).**

---

## 🚀 Integração e Replicação no GameAnimationSample

Em 14 de Agosto de 2026, estendemos a infraestrutura C++ do Sandbox para o projeto **GameAnimationSample** (`D:\Unreal\GameAnimationSample`), realizando a portabilidade completa de forma estável e rastreável:
- **Módulo de Código Nativo**: Criado o módulo do jogo principal C++ `GameAnimationSample` com seus alvos de compilação `Target.cs` e regras de build.
- **Portabilidade de Plugins**: Habilitamos todos os 11 plugins do Sandbox e suas dependências associadas no arquivo `.uproject` do projeto de animações.
- **Build de Compilação Completa**: O projeto compilou com sucesso absoluto na linha de comando via UBT com 241 passos de compilação C++.
- **Garantia Verde nos Testes**: Executamos a suíte de testes unitários do Sandbox dentro do novo ambiente integrado, retornando **EXIT CODE: 0** com todos os 32 testes de automação passando.
- **Repositório GitHub**: Publicado no GitHub sob a conta `JoaoSantosCodes` no repositório [GameAnimationSampleSandbox-Framework](https://github.com/JoaoSantosCodes/GameAnimationSampleSandbox-Framework) com `.gitignore` configurado para omitir assets pesados da Epic Games e manter o repositório leve (apenas código, plugins de lógica e configurações).

---

## 🔒 Fase 20: Segurança de Rede (RPC Rate-Limiting & Server Validations)

Em 14 de Agosto de 2026, implementamos a segurança e blindagem de rede autoritativa do Sandbox Framework contra hacks e exploits no cliente:
*   **RPC Rate-Limiter (`04_SandboxCore`)**:
    *   Criamos a estrutura leve C++ [`FSBRPCRateLimiter`](file:///D:/Unreal/GameAnimationSample/Plugins/04_SandboxCore/Source/SandboxCore/Public/Subsystems/SBRPCRateLimiter.h) para registrar o tempo e contador de requisições de RPC.
    *   Integrada nos métodos de validação RPC de habilidades, locomoção, combate e interação, limitando de forma segura requisições excessivas (antispam/flooding).
*   **Validação de Habilidades (`05_SandboxCharacter`)**:
    *   [`USBAbilityComponent::ServerRequestBehavior_Validate`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Components/SBAbilityComponent.cpp#L290-L302): Valida se a habilidade requisitada está na lista de comportamentos disponíveis concedidos via `PawnData`, bloqueando injeções arbitrárias de habilidades não desbloqueadas.
*   **Validação Física de Proximidade (`07_SandboxInteraction`)**:
    *   [`USBInteractionComponent::ServerStartInteract_Validate`](file:///D:/Unreal/GameAnimationSample/Plugins/07_SandboxInteraction/Source/SandboxInteraction/Private/Components/SBInteractionComponent.cpp#L284-L318) e `ServerCompleteInteract_Validate`: Executa verificações geométricas 3D autoritativas de alcance no servidor, garantindo que o Target esteja dentro do raio de interação física (`InteractionRange + 150.f` de tolerância de lag).
*   **Novos Testes Automatizados**:
    *   Expandimos a suíte de testes de interação em [`SBInteractionTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/07_SandboxInteraction/Source/SandboxInteraction/Private/Tests/SBInteractionTests.cpp) com o **Cenário 6: Validação de Segurança de Rede (Anti-Cheat & Rate-Limiting)**, cobrindo:
        *   Rejeição imediata de alvos nulos ou fora de alcance físico.
        *   Throttling preciso de chamadas pelo rate-limiter, travando requisições que estouram o limite na janela de 1 segundo.
*   **Status de Testes e Build**:
    *   O projeto compilou em 27 segundos via UBT.
    *   A suíte de testes foi executada e retornou **Sucesso Absoluto (33/33 testes verdes - EXIT CODE: 0)**.

---

## ⚡ Fase 21: Compensação de Lag (Network Rewind / Backtracking)

Em 14 de Agosto de 2026, implementamos a compensação de lag autoritativa do lado do servidor para disparos hitscan de armas de fogo:
*   **USBLagCompensationSubsystem (`04_SandboxCore`)**:
    *   Criamos o subsistema de mundo [`USBLagCompensationSubsystem`](file:///D:/Unreal/GameAnimationSample/Plugins/04_SandboxCore/Source/SandboxCore/Public/Subsystems/SBLagCompensationSubsystem.h) herdando de `FTickableGameObject`.
    *   Registra a localização e rotação de todos os `ACharacter` ativos no mundo a cada frame no `HistoryMap`.
    *   Elimina entradas antigas excedendo `MaxHistoryDuration = 1.0f` para otimização de memória.
    *   Adiciona os métodos `RewindPositions` (computa e interpola a posição do passado slerp/lerp baseada na latência do PING e move temporariamente os colisores via `TeleportPhysics`) e `RestorePositions` (restaura a colisão de volta ao presente síncrono).
*   **Integração de Disparo Hitscan (`06_SandboxCombat`)**:
    *   [`USBWeaponBehaviorHitscan::PerformHitscanTrace`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Weapons/SBWeaponBehaviorHitscan.cpp#L43-L75): Recupera a latência round-trip do jogador (`APlayerState::GetPingInMilliseconds()`), calcula o tempo unilateral (`PingInSeconds * 0.5f`) clampado em até `0.5s` de proteção anti-cheat, rebobina os alvos, executa o Line Trace e restaura os alvos síncronamente.
*   **Novos Testes Automatizados**:
    *   Criamos a suíte de testes [`SBLagCompensationTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Tests/SBLagCompensationTests.cpp) simulando com precisão de tempo um movimento linear: grava posições em T=1.0s (origem) e T=2.0s (100 unidades à frente), rebobina para o tempo intermediário T=1.5s (calculando exatamente a metade do trajeto = 50.0) e restaura para o presente de autoridade.
*   **Status de Testes e Build**:
    *   Projeto compilou em 41 segundos.
    *   Suíte inteira passou com sucesso absoluto (**34/34 specs verdes - EXIT CODE: 0**).

---

## 🧪 Fase 22: Sistema de Status Effects (Buffs / Debuffs / DOTs)

Em 14 de Agosto de 2026, implementamos o sistema de Status Effects genérico, extensível e totalmente replicável em rede:
*   **USBStatusEffectDefinition (`05_SandboxCharacter`)**:
    *   Criamos a classe de definição base [`USBStatusEffectDefinition`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Public/DataAssets/SBStatusEffectDefinition.h) como um `UPrimaryDataAsset`.
    *   Permite especificar tags concedidas temporariamente (`GrantedTags`), modificadores de atributos (`AttributeModifiers`), além de ticks periódicos (`DefaultPeriod`, `PeriodAttributeTag`, `PeriodAttributeChange`) para efeitos como veneno (DOT) ou regeneração (HOT).
*   **USBStatusEffectComponent (`05_SandboxCharacter`)**:
    *   Criamos o componente [`USBStatusEffectComponent`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Public/Components/SBStatusEffectComponent.h) para gerenciar o ciclo de vida e expiração dos efeitos ativos.
    *   Armazena e replica o estado dos efeitos usando [`FSBStatusEffectList`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Public/Components/SBStatusEffectComponent.h#L38-L54) com `FFastArraySerializer` para eficiência em rede.
    *   Implementa fallback sob demanda (`FindComponentByClass`) para referenciar componentes de atributos e tags locais caso o cache de inicialização de testes retorne nulo.
*   **Novos Testes Automatizados**:
    *   Criamos a suíte de testes [`SBStatusEffectTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Tests/SBStatusEffectTests.cpp) cobrindo:
        *   Aplicação e remoção de buffs permanentes com modificador de velocidade aditivo (+100) e tag de invulnerabilidade.
        *   Expiração automática de debuffs lentos temporais (2.0 segundos de duração).
        *   Ticks periódicos de dano por veneno (DOT) subtraindo 10 de vida a cada 1.0 segundo.
*   **Status de Testes e Build**:
    *   Projeto compilou em 38 segundos.
    *   Suíte inteira passou com sucesso absoluto (**37/37 specs verdes - EXIT CODE: 0**).

---

## 🎭 Fase 23: Sincronização Estética de Equipamento (Visual & Sockets)

Em 14 de Agosto de 2026, implementamos a sincronização estética de armas e equipamentos físicos nos sockets do esqueleto de personagens de forma replicada em rede:
*   **SBWeaponBehaviorDefinition (`06_SandboxCombat`)**:
    *   Adicionamos propriedades para classe do ator visual (`WeaponActorClass`), socket ativo (`ActiveSocketName`) e socket do coldre (`HolsterSocketName`) em [`SBWeaponBehaviorDefinition.h`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Public/DataAssets/SBWeaponBehaviorDefinition.h).
    *   Criamos [`SBWeaponBehaviorDefinition.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/DataAssets/SBWeaponBehaviorDefinition.cpp) inicializando os sockets padrão `hand_rSocket` (mão ativa) e `spine_03Socket` (coldre).
*   **USBCombatComponent (`06_SandboxCombat`)**:
    *   Declaramos a estrutura replicada [`FSBSpawnedWeaponEntry`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Public/Components/SBCombatComponent.h#L39-L49) e a lista `SpawnedWeapons` para replicação estável em rede.
    *   Implementamos o método `SetWeaponVisualActive` anexando dinamicamente o actor visual ao socket correspondente do mesh (`CharOwner->GetMesh()`) via `AttachToComponent` com regras de snap.
    *   Injetamos a lógica de spawn no servidor dentro de `OnItemEquipped` e `LoadCombatConfig` (com destruição de atores visuais anteriores e configuração de mobilidade para `Movable` para suportar atores com mobilidade estática por padrão, como `AStaticMeshActor`), e a destruição limpa em `OnItemUnequipped` e `OnShutdown_Implementation`.
*   **Integrado saca/guarda automático (`06_SandboxCombat`)**:
    *   [`SBWeaponBehavior.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Weapons/SBWeaponBehavior.cpp): No método `Enter_Implementation` (ativação do disparo/comportamento), chama `SetWeaponVisualActive(true)` para sacar a arma para a mão do personagem. No método `Exit_Implementation`, chama `SetWeaponVisualActive(false)` para guardar no coldre de volta.
*   **Novos Testes Automatizados**:
    *   Criamos a suíte de testes [`SBWeaponVisualTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Tests/SBWeaponVisualTests.cpp) cobrindo:
        *   Spawn automático do ator da arma e anexação inicial no socket de coldre (`spine_03Socket`).
        *   Saque dinâmico (transposição para `hand_rSocket`) ao disparar.
        *   Retorno ao coldre ao cessar o disparo.
        *   Destruição total do ator no shutdown.
*   **Status de Testes e Build**:
    *   Projeto compilou em 30 segundos.
    *   Suíte inteira passou com sucesso absoluto (**38/38 specs verdes - EXIT CODE: 0**).

---

## 🔒 Fase 24: Anti-Cheat Avançado de Movimento e Dano (v1.9.0)

Em 14 de Agosto de 2026, implementamos validações autoritativas rigorosas no servidor para evitar speedhacks, teleportes indevidos e disparos de arma de fogo através de paredes:
*   **Detecção de Movimentos Anômalos (`05_SandboxCharacter`)**:
    *   [`USBMovementComponent::TickComponent`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Components/SBMovementComponent.cpp): O servidor calcula a cada frame de simulação a distância 2D percorrida pelo personagem desde a última posição validada (`LastValidatedLocation`).
    *   Compara contra a velocidade máxima teórica ajustada pelo `CharacterMovement` mais uma tolerância para ping jitter.
    *   Se a distância exceder a tolerância por frame, ou se ocorrer um teleporte instantâneo acima de `3000.f` unidades, o servidor força o rollback do transform do ator (`TeleportTo`) de volta para a última posição autorizada.
*   **Cálculo Centralizado e Testabilidade (`GetCalculatedMaxSpeed`)**:
    *   Para expor a lógica de caixa preta do cálculo físico e matemático de velocidades combinadas, criamos a função pública [`GetCalculatedMaxSpeed()`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Components/SBMovementComponent.cpp). Ela combina de forma cumulativa e proporcional o valor base do CMC, agachamento físico vs. comportamentos ativados na pilha, modificadores cumulativos do Aggregator (sprint 1.5x) e buffs/debuffs do `Attribute.Speed`.
*   **Sincronização Unificada no Startup (`OnReady`) e Alerta de Desvio**:
    *   A sincronização entre o `MaxWalkSpeed` do CMC e a base do atributo `Attribute.Speed` agora ocorre exatamente uma vez durante a inicialização em `OnReady_Implementation()`, utilizando a API formal de escrita `SetAttributeBaseValue()`.
    *   Adicionamos uma verificação de desvio leve e não-bloqueante a cada 5 segundos em `GetCalculatedMaxSpeed()` para alertar ativamente desenvolvedores caso haja dessincronização dinâmica em runtime.
*   **Bloqueio de Dano por Obstrução Física (Wall-Shot Protection) (`06_SandboxCombat`)**:
    *   [`USBWeaponBehaviorHitscan::PerformHitscanTrace`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Weapons/SBWeaponBehaviorHitscan.cpp): Ao processar um acerto e antes de aplicar o dano de forma autoritativa no servidor, realiza um trace extra de linha de visão a partir da posição física do tórax do atacante (`Character->GetActorLocation() + FVector(0,0,50)`) até o ponto de impacto do disparo (`HitResult.ImpactPoint`).
    *   Se este traço intermediário colidir com qualquer geometria física de colisão estática (como paredes ou barreiras de mapa), o disparo é classificado como exploit (Wall-Clipping) e o dano é rejeitado autoritativamente.
*   **Novos Testes Automatizados**:
    *   Expandimos [`SBAntiCheatTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Tests/SBAntiCheatTests.cpp) cobrindo:
        *   **Matemática de Velocidade Combinada**: Valida via `TestEqual` que `GetCalculatedMaxSpeed()` retorna exatamente `1050.f` sob sprint (1.5x) e atributos de velocidade.
        *   **Detecção de Speedhack & Rollback**: Simula deslocamento linear de 340 unidades em 0.1s lido contra a nova velocidade e força o rollback.
        *   **Wall-Shot Protection**: Spawna um `UBoxComponent` estático bloqueante entre o atacante e o alvo, executa o disparo, e valida que o dano é bloqueado.

---

## ⚡ Fase 25: Otimização de Replicação e Atributos Condicionais (v1.10.0)

Em 15 de Agosto de 2026, reformulamos a replicação de rede do sistema de atributos para economizar largura de banda e impedir o vazamento de dados de telemetria internos:
*   **Divisão Pública e Privada (`05_SandboxCharacter`)**:
    *   [`USBAttributeComponent.h`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Public/Components/SBAttributeComponent.h): Substituímos o array de replicação único por dois canais separados: `PublicAttributes` (replicado para todos) e `PrivateAttributes` (replicado apenas para o controlador proprietário).
    *   [`USBAttributeComponent::GetLifetimeReplicatedProps`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Components/SBAttributeComponent.cpp): Registramos `PrivateAttributes` utilizando a condição de replicação estrita **`COND_OwnerOnly`**, limitando o tráfego de rede das variáveis privadas de interesse individual (Mana, Stamina, Munição de Armas).
    *   **Acesso Controlado de Estado para Testabilidade**: Em vez de expor getters mutáveis na API pública, declaramos a classe de testes `friend class FSBConditionalReplicationTestsSpec;` no componente de atributos. Isso permite que a especificação de testes acesse diretamente os dados internos sob escopo restrito, sem expor métodos de mutação na API pública de produção.
*   **Classificação Baseada em Metadados (`bIsPrivate`)**:
    *   Adicionamos a propriedade booleana `bIsPrivate` diretamente à struct `FSBAttribute` (dentro de [`SBCommonTypes.h`](file:///D:/Unreal/GameAnimationSample/Plugins/01_SandboxCommon/Source/SandboxCommon/Public/Types/SBCommonTypes.h)). Isso elimina o padrão frágil de substrings e comparações de tags baseadas em strings (`Contains()`).
*   **Blindagem contra Duplicidades de Canais**:
    *   Em `UpdateReplicatedAttribute()`, limpamos a entrada correspondente no array oposto quando um atributo tem sua classificação alterada em runtime ou reinicialização, prevenindo a existência simultânea de uma tag nos dois canais.
*   **Nova Suíte de Testes de Escopo de Rede**:
    *   Em [`SBConditionalReplicationTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Tests/SBConditionalReplicationTests.cpp), criamos testes específicos para:
        *   **Classificação Dinâmica**: Confirma a classificação pública/privada de atributos baseada estritamente no booleano `bIsPrivate`.
        *   **Migração Segura**: Valida que mudar a flag `bIsPrivate` de um atributo existente remove-o do canal anterior e o insere no canal atual de forma limpa.
        *   **Simulação de Segregação por Roles**: Cria um Pawn local (Owner Client - `ROLE_AutonomousProxy`) e um Pawn remoto simulado (Simulated Proxy - `ROLE_SimulatedProxy`). Simula a replicação do servidor para ambos e valida que o cliente proprietário recebe o atributo privado (Mana = 50.f) enquanto o proxy simulado recebe nulo/vazio (Mana = 0.f), validando a lógica de merge/leitura condicional local (a garantia de não-transmissão em rede depende do mecanismo nativo COND_OwnerOnly da engine, verificado por declaração de replicação, não por tráfego de rede simulado).
*   **Métricas da Suíte**:
    *   A suíte inteira foi executada e homologada com sucesso absoluto (**44 de 44 testes verdes - EXIT CODE: 0**).

---

## ⚡ Fase 26: Persistência Estética e Restauração de Equipamento no Save/Load (v1.11.0)

Em 15 de Agosto de 2026, implementamos a persistência e restauração do estado de equipamento físico nos ganchos de salvamento:
*   **Persistência Integrada via DynamicTags**:
    *   Em [`SBInventoryComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Private/Components/SBInventoryComponent.cpp): Atualizamos `ServerEquipItem` para adicionar a tag `State.Item.Equipped` ao item `USBItemInstance->DynamicTags`, e `ServerUnequipItem` para removê-la. Isso permite salvar automaticamente o estado equipado dentro do array de tags dinâmicas serializado.
*   **Restauração Diferida (Next-Tick Deferral)**:
    *   Em `LoadComponentData_Implementation`: Ao carregar os itens e recriá-los na mochila, lemos as tags dinâmicas restauradas. Em vez de re-equipar as armas inline (o que causaria race conditions caso outros componentes de combate ou do ator ainda não estivessem carregados/inicializados), agendamos a re-equipagem para o próximo tick físico utilizando o gerenciador de timers do mundo (`SetTimerForNextTick`).
*   **Auto-healing de Conflitos**:
    *   A restauração deferida varre o inventário e executa `ServerEquipItem` em sequência. Se dados corrompidos ou saves legados tentarem carregar mais de um item no mesmo `ExclusivityGroup`, a validação de regras de concorrência ejetará automaticamente as duplicadas, mantendo o estado final limpo.
*   **Novo Cenário de Teste de Persistência**:
    *   Em [`SBInventorySaveTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Private/Tests/SBInventorySaveTests.cpp): Adicionamos o `Cenário 2: Persistência e restauração do estado equipado (Visual/Behavior)`, que cria um item com fragmento equipável, equipa-o no personagem, salva o jogo, destrói e recria o personagem, executa o `LoadGame`, avança um tick do timer manager para disparar a re-equipagem deferida, e valida que a arma e seu evento foram restabelecidos com sucesso.
*   **Métricas Finais da Suíte**:
    *   A suíte foi homologada com sucesso absoluto (**45 de 45 testes verdes - EXIT CODE: 0**).

---

## 🟢 Resoluções de Auditoria e Otimizações Físicas (Fases 20-23) (v1.12.0)

Em 16 de Agosto de 2026, implementamos a resolução dos achados prioritários resultantes da auditoria de código linha-a-linha das Fases 20 a 23:
*   **Otimização do Lag Compensation (`04_SandboxCore` & `05_SandboxCharacter`)**:
    *   Substituímos o uso ineficiente de `TActorIterator` a cada frame em `RecordPositions` por uma lista interna de registro dinâmico `RegisteredCharacters`.
    *   Implementamos `ASBCharacter::BeginPlay` (servidor) e `ASBCharacter::EndPlay` para registrar e desregistrar dinamicamente cada pawn no subsistema.
    *   Atualizamos `USBLagCompensationSubsystem::RewindPositions` para aceitar a localização do atirador (`ShooterLocation`) e o raio de alcance da arma (`MaxRange`). Agora, apenas os personagens dentro deste raio físico de ameaça são rebobinados, poupando overhead de colisão e transformações desnecessárias no servidor.
*   **Correção de Tick Drift nos Status Effects (`05_SandboxCharacter`)**:
    *   Substituímos a redefinição direta e absoluta de timer `LastPeriodTriggerTime = CurrentTime;` por incrementos proporcionais e acumulativos baseados no período (`Entry.LastPeriodTriggerTime += Entry.Period;`) organizados em um loop de catch-up (`while`). Isso garante consistência de ticks aplicados independentemente de flutuações de frame rate ou congelamentos temporários do servidor.
*   **Blindagem de Replicação de Armas Visuais (`06_SandboxCombat`)**:
    *   Forçamos programmaticamente `NewWeaponActor->SetReplicates(true);` logo após instanciar a arma visual na autoridade no `USBCombatComponent::OnItemEquipped()`. Isso evita falhas de replicação cosmética caso o designer esqueça de marcar manualmente a flag de replicação no Blueprint da arma visual.
*   **Status de Testes e Build**:
    *   A suíte inteira de automação passou com sucesso absoluto (**45 de 45 testes verdes - EXIT CODE: 0**).

---

## 🟢 Fase 27: Sistema de Estamina Avançado (v1.13.0)

Em 16 de Agosto de 2026, projetamos, implementamos e validamos o sistema de estamina predito/corrigido integrado com sprint e saltos:
*   **Atributo com Replicação Condicional COND_OwnerOnly (`05_SandboxCharacter`)**:
    *   Configuramos e registramos o atributo `Attribute.Stamina` com `bIsPrivate = true` em `OnReady_Implementation` no [`SBMovementComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Components/SBMovementComponent.cpp#L44-L57). Isso ativa a otimização de banda de rede enviando as atualizações apenas ao jogador proprietário (`COND_OwnerOnly`), bloqueando exploits de leitura de pacotes externos por outros jogadores.
*   **Integração com Sprint e Jump (`05_SandboxCharacter`)**:
    *   Implementamos o consumo predito no cliente e replicado no servidor de `15.f/s` ao sprintar, com interrupção instantânea e bloqueio de reativação caso a estamina seja nula.
    *   Sobrescrevemos o método [`ASBCharacter::Jump()`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Character/SBCharacter.cpp#L148-L161) para deduzir `20.f` instantâneos de estamina e abortar fisicamente o pulo se o saldo for menor que o custo.
*   **Regeneração com Delay e Estado Exhausted (`05_SandboxCharacter`)**:
    *   A regeneração passiva recupera `10.f/s` após `1.5s` livres de consumo.
    *   Ao atingir `0.f`, o estado entra em exaustão e recebe a tag `State.Character.Exhausted`. A locomoção volta ao normal e impede novas corridas/pulos até recuperar acima do limiar de `30.f`.
*   **Validação Automatizada (`05_SandboxCharacter`)**:
    *   Adicionamos uma nova suíte de testes unitários [`SBStaminaTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Tests/SBStaminaTests.cpp) cobrindo:
        1. Consumo do Sprint por tick e atraso/taxa da regeneração passiva.
        2. Consumo de estamina ao pular e bloqueio do pulo em caso de saldo insuficiente.
        3. Entrada na exaustão ao zerar a estamina e saída ao atingir o limiar de `30.f`.
*   **Integração com Interface HUD C++ (`09_SandboxUI`)**:
    *   Estendemos a backing class C++ [`USBStatusHUDWidget`](file:///D:/Unreal/GameAnimationSample/Plugins/09_SandboxUI/Source/SandboxUI/Public/Widgets/SBStatusHUDWidget.h) adicionando o ponteiro `PB_Stamina` para vincular automaticamente a barra de estamina na HUD.
    *   Corrigimos a tag do payload de vida para `Attribute.Health` em [`SBStatusHUDWidget.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/09_SandboxUI/Source/SandboxUI/Private/Widgets/SBStatusHUDWidget.cpp#L28) para estar alinhada com as tags nativas registradas na inicialização.
*   **Status de Testes e Build**:
    *   A suíte inteira de automação passou com sucesso absoluto (**48 de 48 testes verdes - EXIT CODE: 0**).

---

## 🟢 Fase 28: Vinculação de Assets Visuais e Playtests de UI (v1.14.0)

Em 16 de Agosto de 2026, corrigimos e integramos a infraestrutura programática do barramento de UI e estabelecemos o roadmap visual do UMG Designer:
*   **Correção de Herança do Payload de Inventário (`08_SandboxInventory`)**:
    *   Identificamos e corrigimos um bug de coerção silenciosa (Cast returning nullptr) na atualização do grid de UI. Fizemos a classe [`USBInventorySlotUpdatedEventPayload`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Public/Components/SBInventoryComponent.h#L21) herdar de `USBInventoryEventPayload` (declarada no plugin `04_SandboxCore`), permitindo casting polimórfico de eventos de rede de inventário de forma limpa.
*   **Mapeamento de HUD Visual C++ (`09_SandboxUI`)**:
    *   Mapeamos o suporte a `PB_Stamina` e o fix do binding de vida (`Attribute.Health`) na classe backing [`USBStatusHUDWidget`](file:///D:/Unreal/GameAnimationSample/Plugins/09_SandboxUI/Source/SandboxUI/Public/Widgets/SBStatusHUDWidget.h).
*   **Diretrizes de Proteção contra UI Spill em Split-Screen**:
    *   Documentamos a arquitetura baseada em filtros locais (`TargetPawn == GetOwningPlayerPawn()`) para garantir isolamento absoluto de pacotes de dados de interface na tela dividida em sessões com múltiplos jogadores locais.
*   **Status de Testes e Build**:
    *   A suíte inteira de automação passou com sucesso absoluto (**48 de 48 testes verdes - EXIT CODE: 0**).

---

## 🟢 Fase 29: Sistema de Munição e Recarga (v1.15.0)

Em 16 de Agosto de 2026, projetamos, implementamos e homologamos o sistema dinâmico de munições e comportamento de recarga com predição local e validação de autoridade:
*   **Inicialização do Atributo de Munição (`06_SandboxCombat`)**:
	*   No [`SBCombatComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Components/SBCombatComponent.cpp#L114-L139), adicionamos o override de `OnReady_Implementation` para registrar dinamicamente o atributo privado `Attribute.Weapon.Ammo` (`COND_OwnerOnly`) com capacidade padrão de `30.f` caso ainda não esteja instanciado.
*   **Comportamento de Recarga (`06_SandboxCombat`)**:
	*   Criamos a classe [`USBWeaponBehaviorReload`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Public/Weapons/SBWeaponBehaviorReload.h) herdando de `USBGameplayBehavior`.
	*   A recarga aplica o estado transiente `State.Character.Reloading` ao personagem, e bloqueia concorrentemente novos disparos de armas no `CanEnter` de [`USBWeaponBehavior`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Weapons/SBWeaponBehavior.cpp#L30-L36).
	*   O comportamento consome `2.0s` de tempo e redefine o atributo de munição de volta ao seu `MaxValue` de forma autoritativa ao final da ação.
*   **Validação Automatizada (`06_SandboxCombat`)**:
	*   Escrevemos a suíte de testes unitários [`SBReloadTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Tests/SBReloadTests.cpp) cobrindo consumo de munição por disparo, bloqueio por munição zerada, início de recarga, bloqueio de disparos durante recarga e restauração de munição ao valor máximo após tick.
*   **Status de Testes e Build**:
	*   A suíte inteira de automação passou com sucesso absoluto (**50 de 50 testes verdes - EXIT CODE: 0**).

---

## 🟢 Fase 30: Sistema de Cooldowns de Habilidade e Custo de Mana (v1.16.0)

Em 16 de Agosto de 2026, projetamos, implementamos e homologamos o suporte a cooldowns baseados em tags e custo de mana com predição e rollback transacional:
*   **Regeneração Passiva de Mana (`05_SandboxCharacter`)**:
	*   No [`SBAbilityComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Components/SBAbilityComponent.cpp#L496-L518), implementamos regeneração passiva de mana de `5.f/s` com delay de `2.0s` da última ação de consumo, processada de forma autoritativa no servidor dentro do `TickComponent`.
*   **Tags de Cooldown e Controle de Estado (`05_SandboxCharacter`)**:
	*   Adicionamos a propriedade `CooldownTag` à classe [`USBAbility`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Public/Abilities/SBAbility.h#L28-L30).
	*   Na ativação bem-sucedida, a `CooldownTag` correspondente (ex: `State.Cooldown.Ability.Fire`) é adicionada ao `USBStateComponent` do personagem.
	*   A expiração do cooldown no `TickComponent` remove a tag automaticamente.
*   **Rollback de Rede Transacional (`05_SandboxCharacter`)**:
	*   No [`SBAbilityComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Components/SBAbilityComponent.cpp#L351-L373), estendemos `ClientRollbackAbility` para expurgar a entrada pendente na `CooldownsList` e remover a `CooldownTag` do `USBStateComponent` caso o servidor rejeite a ativação local da habilidade.
*   **Validação Automatizada (`05_SandboxCharacter`)**:
	*   Ampliamos a suíte de testes unitários [`SBAbilityTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Tests/SBAbilityTests.cpp) com 3 especificações cobrindo regeneração passiva e delay, aplicação/expiração da tag de cooldown e rollback de rede.
*   **Status de Testes e Build**:
	*   A suíte inteira de automação passou com sucesso absoluto (**53 de 53 testes verdes - EXIT CODE: 0**).

---

## 🟢 Fase 31: Inteligência Artificial Integrada com State Component (v1.17.0)

Em 16 de Agosto de 2026, projetamos, implementamos e homologamos a integração de IAs inimigas ao State Component e a tabela de Agro de combate:
*   **Tabela de Agro de Combate (`06_SandboxCombat`)**:
	*   Implementamos o sistema de Agro com a tabela interna `AgroTable` no [`SBCombatComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Components/SBCombatComponent.cpp) gerenciando e limpando chaves `IsValid(Pawn)` dinamicamente para evitar desvios causados por garbage collection.
*   **Restrições de Movimento e CC no orquestrador (`05_SandboxCharacter`)**:
	*   Atualizamos [`SBMovementComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Components/SBMovementComponent.cpp) interceptando as tags `State.Character.Stunned` e `State.Character.Frozen` para forçar velocidade de locomoção máxima de `0.0f` de forma autoritativa.
*   **Tags de Bloqueio de Comportamento (`06_SandboxCombat`)**:
	*   A infraestrutura C++ herdada de `CanEnter` no [`SBWeaponBehavior.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Weapons/SBWeaponBehavior.cpp) valida automaticamente a presença de tags bloqueadas e impede ativações de habilidades/disparos se o Pawn estiver sob Crowd Control (CC).
*   **Validação Automatizada (`06_SandboxCombat`)**:
	*   Criamos a suíte de testes unitários [`SBAIBehaviorTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Tests/SBAIBehaviorTests.cpp) validando registro e resolução de maior Agro, mitigação de velocidade de locomoção a zero sob CC e restrição programática de ativação de habilidades de armas.
*   **Status de Testes e Build**:
	*   A suíte inteira de automação passou com sucesso absoluto (**56 de 56 testes verdes - EXIT CODE: 0**).



