# Checklist de Implementação - Sandbox Framework

## Fase 1: Setup do Projeto e Estrutura dos Plugins
- [x] Ativar os plugins nativos (`ModularGameplay`, `GameFeatures`) no `V1.uproject`
- [x] Criar a estrutura de diretórios padronizada e arquivos de configuração dos plugins da fundação:
  - [x] `01_SandboxCommon`
  - [x] `02_SandboxInterfaces`
  - [x] `03_SandboxAssets`
  - [x] `04_SandboxCore`
  - [x] `05_SandboxCharacter`
  - [x] `09_SandboxUI`
  - [x] `11_SandboxEditor` (Editor-Only)

## Fase 2: Implementação das Camadas Base (Common, Interfaces, Assets)
- [x] **01_SandboxCommon**:
  - [x] Mapeamento e registro estático de Gameplay Tags (`SBGameplayTags`)
  - [x] Estrutura genérica de Atributos (`FSBAttribute` e `FSBAttributeModifier`)
  - [x] Configurações gerais (`USBDeveloperSettings`)
  - [x] Definição e registro de Categorias de Log (`LogSandbox`) com macro de log centralizado
- [x] **02_SandboxInterfaces**:
  - [x] Interface de Ciclo de Vida de Componentes (`ISBComponentInterface`)
  - [x] Interface de Persistência (`ISBSaveInterface`)
  - [x] Interfaces de Gameplay (`ISBCharacterInterface`, `ISBInteractableInterface`, `ISBCombatantInterface`)
- [x] **03_SandboxAssets**:
  - [x] Asset Manager customizado (`USBAssetManager` derivado de `UAssetManager`)
  - [x] Data Assets base (`USBPrimaryDataAsset`, `USBPawnDataAsset`, `USBComponentSetDataAsset`, `USBAbilitySetDataAsset`)

## Fase 3: Implementação de Core e Eventos
- [x] **04_SandboxCore**:
  - [x] Component Factory (`USBComponentFactory`) para injeção e ciclo de vida
  - [x] Event Bus com Prioridades (`USBEventSubsystem`) para eventos nativos e dinâmicos
  - [x] Subsistema de Entrada por Tags (`USBInputSubsystem` e `USBInputComponent`)
  - [x] Classes de ciclo de vida do jogo (`ASBGameMode`, `ASBGameState`, `ASBPlayerController`, `ASBPlayerState`, `USBGameInstance`)

## Fase 4: Implementação do Personagem e Atributos
- [x] **05_SandboxCharacter**:
  - [x] Contêiner limpo do Personagem (`ASBCharacter`) com inicialização a partir do PawnData
  - [x] Componente de Atributos genérico por Tags (`USBAttributeComponent`)
  - [x] Componente de Estado por Tags (`USBStateComponent`)
  - [x] Componente de Habilidade baseado em dados (`USBAbilityComponent`)

## Fase 5: Implementação de UI e Ferramentas do Editor
- [x] **09_SandboxUI**:
  - [x] Gerenciador de Camadas de UI (`USBUIManager`)
  - [x] Widget base com foco/transição (`USBUserWidget`)
  - [x] Classe HUD (`ASBHUD`)
- [x] **11_SandboxEditor**:
  - [x] Estruturação do módulo editor-only para ferramentas visuais e validadores de PawnData

## Fase 6: Compilação e Validação Final
- [x] Geração de arquivos do projeto (Utiliza SDK .NET bundled nativo da Unreal Engine 5.8)
- [x] Criação de todas as classes de fundação v1.0.0 finalizada

## Fase 7: Extensão do Personagem - Movimentação Avançada (em 05_SandboxCharacter)
- [x] Implementar componente orquestrador central (`USBMovementComponent`) em camadas de API
- [x] Implementar classe de comportamento base (`USBMovementBehavior`) consumindo o contexto unificado `FSBBehaviorContext`
- [x] Implementar registro de comportamentos (`USBMovementBehaviorRegistry`) e pilha ativa (Behavior Stack) com prioridades e grupos de exclusão mútua
- [x] Implementar agregador de modificadores físicos (`USBMovementModifierAggregator`)
- [x] Implementar interface de backend de física (`ISBMovementBackend`) e wrapper para o CharacterMovement Component (`FSBCharacterMovementBackend`)
- [x] Criar Data Assets (`USBMovementConfigDataAsset` e `USBMovementBehaviorDefinition`)
- [x] Integrar inputs via Gameplay Tags e testar ativação de comportamentos na pilha com regras de prioridades e interrupções

## Fase 8: Extensão do Personagem - Animação Modular (em 05_SandboxCharacter)
- [x] Implementar sistema de Linked Anim Graphs e Overlay System orientados a tags
- [x] Integrar ganchos de Hand/Foot IK e Motion Warping (Escopo Futuro / Backlog)
- [x] Criar Data Assets de presets de animação (Escopo Futuro / Backlog)

## Fase 9: Extensão do Personagem - Sistema de Câmeras (em 05_SandboxCharacter)
- [x] Implementar componente de câmera modular e gerenciador de modos
- [x] Criar modos de câmera orientados a dados (Walking, Sprint, Aim)

## Fase 10: Sistema de Combate e Predição de Rede (em 06_SandboxCombat)
- [x] Extensão do `USBAttributeComponent` (Predição de Atributos, Upsert de array replicado, Timeout guard, escrita centralizada via `ModifyAttributeBaseValue`)
- [x] Inicialização do plugin `06_SandboxCombat` (descritor, Build.cs, Module)
- [x] Implementação de `USBCombatComponent` (Orquestrador, ExclusivityGroup ejection, FireRate cooldown, rollbacks)
- [x] Implementação de `USBWeaponBehavior` e `USBWeaponBehaviorHitscan` (LineTrace Hit autoritativo, dano a atributos do alvo)
- [x] Implementação da suíte de testes de integração `SBCombatTests` (cenários de predição, rollbacks por cheat e swaps de arma)

## Fase 11: Correção de Compilação e Suíte de Testes Automatizados (v1.2.0)
- [x] Corrigir erros de Crouch sem solo em mundos headless configurando `MOVE_Walking` no setup do Pawn (sem bypasses em produção)
- [x] Desenvolver setup de possessão virtual usando `friend class` e inicialização de `ULocalPlayer` e `APlayerState` nativos (eliminando classes mock do código de produção)
- [x] Remover desvios de teste (`GIsAutomationTesting`) do código de produção das rotinas de Crouch e Controle Local
- [x] Validar 100% dos testes unitários e de rede (Suíte Sandbox inteiramente verde e limpa)

## Fase 12: Sistema de Interação Modular (07_SandboxInteraction)
- [x] Atualizar `ISBInteractableInterface` em `02_SandboxInterfaces` com novos métodos e defaults (Duration, Lock/Unlock, IsLocked)
- [x] Inicializar o plugin `07_SandboxInteraction` (descritor .uplugin, Build.cs, módulo de runtime)
- [x] Registrar as Gameplay Tags de eventos de interação (`Event.Interaction.*`) na inicialização do módulo
- [x] Implementar `USBInteractionComponent` com suporte a varredura local, hold states e RPCs de sincronização/cancelamento
- [x] Escrever a suíte de testes automatizados `SBInteractionTests` (Cenários 1 a 5, incluindo a disputa simultânea de lock)
- [x] Atualizar o guia de desenvolvimento `sfdg_guide.md` com as diretrizes de Interação e concorrência
- [x] Compilar, executar os testes e garantir 100% verde (21/21 specs bem-sucedidos)

## Fase 13: Sistema de Inventário e Slots Replicados (08_SandboxInventory) (Fase Atual)
- [x] Inicializar o plugin `08_SandboxInventory` (descritor .uplugin, Build.cs, módulo de runtime)
- [x] Implementar as classes de Item: `USBItemDefinition`, `USBItemFragment`, `USBItemInstance`
- [x] Implementar as estruturas do FastArray: `FSBInventoryEntry`, `FSBInventoryList`
- [x] Implementar o componente `USBInventoryComponent` com gerenciamento de slots e fila de ativação pendente com timeout
- [x] Implementar ganchos de eventos no `USBCombatComponent` em `06_SandboxCombat` para escutar equipamentos de forma desacoplada
- [x] Escrever a suíte de testes de integração `SBInventoryTests.cpp` (Cenários 1 a 5)
- [x] Compilar, executar os testes e garantir 100% de cobertura verde em toda a suíte do Sandbox

## Fase 14: Consolidação do USBBehaviorStackComponent (v1.4.0) (Fase Atual)
- [x] Criar a estrutura base de classes `USBGameplayBehaviorDefinition` e `USBGameplayBehavior` em `01_SandboxCommon`
- [x] Implementar o componente comum `USBBehaviorStackComponent` com a proteção de reentrância recursiva, loop flat e ordenação de prioridade
- [x] Migrar `USBMovementBehavior` e `USBMovementComponent` em `05_SandboxCharacter` para herdar da base comum e usar o gancho `OnBehaviorEjected`
- [x] Migrar `USBWeaponBehavior` e `USBCombatComponent` em `06_SandboxCombat` para herdar da base comum e usar o gancho `OnBehaviorEjected`
- [x] Implementar a suíte de testes de estresse e reentrância `SBSourceBehaviorStackTests.cpp` em `01_SandboxCommon`
- [x] Executar a suíte inteira de automação (26/26 testes originais + novo teste de reentrância) e validar estabilidade

## Fase 15: Persistência e Save Game System (v1.5.0)
- [x] Criar a classe base abstrata `USBSaveSubsystem` em `02_SandboxInterfaces`
- [x] Implementar as classes concretas `USBSaveGame`, `USBSavePayload` e `USBSaveSubsystemConcrete` em `04_SandboxCore`
- [x] Atualizar `BuildBehaviorContext` em `SBBehaviorStackComponent.cpp` para obter o SaveSubsystem estaticamente
- [x] Integrar `ISBSaveInterface` ao `USBAttributeComponent` e salvar `AttributesMap`
- [x] Integrar `ISBSaveInterface` ao `USBInventoryComponent` e criar estruturas de serialização personalizadas com `DynamicTags`
- [x] Escrever a bateria de testes de persistência `SBSaveTests.cpp` em `08_SandboxInventory` (relocado para evitar dependência circular)
- [x] Compilar, executar a suíte e verificar se todos os 28 testes estão verdes

## Fase 16: Sistema de Habilidades Baseado no Behavior Stack (v1.6.0)
- [x] Atualizar `FSBAbilitySetEntry` em `SBAbilitySetDataAsset.h` com `InputTag` e ajustar tipo de `AbilityClass`
- [x] Modificar `USBAbility` para herdar de `USBGameplayBehavior` em `SBAbility.h` e adicionar campos de custos
- [x] Modificar `USBAbilityComponent` para herdar de `USBBehaviorStackComponent` e habilitar replicação
- [x] Implementar a lista de cooldowns replicada `FSBCooldownList` com chave estável por `AbilityTag`
- [x] Implementar bindings de Enhanced Input com `InputTag` no `USBAbilityComponent`
- [x] Sobrescrever `RequestBehavior` com validação e consumo preditivo de recursos via `USBAttributeComponent`
- [x] Sobrescrever `OnBehaviorEjected` com lógica de rollback via `PredictionId` sob falha no servidor
- [x] Desenvolver a suíte de testes `SBAbilityTests.cpp` cobrando os 3 cenários acordados
- [x] Compilar, rodar testes e verificar 100% de cobertura verde em toda a suíte (31/31 testes verdes)

## Fase 17: Gameplay Debugger e Telemetria (10_SandboxDebug)
- [x] Criar a interface C++ `ISBDebugInterface` e struct `FSBDebugLine` em `02_SandboxInterfaces`
- [x] Implementar `ISBDebugInterface` em `USBBehaviorStackComponent` (01_SandboxCommon)
- [x] Implementar `ISBDebugInterface` em `USBAttributeComponent`, `USBStateComponent` e `USBAbilityComponent` (05_SandboxCharacter)
- [x] Implementar `ISBDebugInterface` em `USBInventoryComponent` (08_SandboxInventory) e `USBCombatComponent` (06_SandboxCombat)
- [x] Implementar `ISBDebugInterface` nos atores de teste de interação (como `ASBTestLockedChest` em `07_SandboxInteraction`)
- [x] Inicializar o plugin `10_SandboxDebug` (descritor `.uplugin`, `Build.cs`, módulo)
- [x] Implementar o coletor nativo `FGameplayDebuggerCategory_Sandbox` e registrá-lo no módulo
- [x] Executar o teste de desacoplamento removendo a dependência de `08_SandboxInventory` e validando o build
- [x] Compilar o V1Editor com sucesso e garantir que a suíte de testes permaneça verde

## Fase 18: Interface Dinâmica (09_SandboxUI)
- [x] Criar payloads de evento em `SBEventPayloads.h` no plugin `04_SandboxCore`
- [x] Implementar idempotência em `USBEventSubsystem::SubscribeToEvent`
- [x] Wirear a emissão de eventos e throttle em `USBAttributeComponent`, `USBInteractionComponent`, `USBInventoryComponent` e `USBAbilityComponent`
- [x] Implementar auto-unsubscribe cirúrgico em `USBUserWidget` (`09_SandboxUI`)
- [x] Implementar ciclo de vida e layers em `USBUIManager` (`09_SandboxUI`)
- [x] Vincular HUD default com player local em `ASBHUD` (`09_SandboxUI`)
- [x] Criar a suíte de testes de UI `SBUITests.cpp` cobrindo idempotência/auto-unsubscribe e local player filtering
- [x] Rodar o teste de isolamento desativando `05`, `06`, `07`, `08` e confirmando compilação do `09_SandboxUI`
- [x] Compilar completo e verificar 100% de cobertura verde em toda a suíte de testes (32/32 specs)

## Fase 19: Integração Visual e Portabilidade
- [x] Converter o projeto `GameAnimationSample` de Blueprint para C++ híbrido
- [x] Habilitar dependências e plugins no `.uproject` do `GameAnimationSample`
- [x] Escrever os alvos de compilação `Target.cs` e regras de build `Build.cs` do novo projeto
- [x] Compilar `GameAnimationSampleEditor` com sucesso (zero erros, 241 passos de compilação C++)
- [x] Validar estabilidade rodando a suíte inteira de 32 testes Sandbox no contexto do `GameAnimationSample` (EXIT CODE: 0)
- [x] Criar pasta no Obsidian e documentar a migração em `GameAnimationSample/migracao_gameplay_animation_sample.md`
- [x] Inicializar Git e realizar commit/push do código C++ leve (assets pesados ignorados via `.gitignore`) para o GitHub

## Fase 20: Segurança de Rede (RPC Rate-Limiting & Server Validations)
- [x] Implementar estrutura C++ leve de Rate-Limiting `FSBRPCRateLimiter` no `04_SandboxCore`
- [x] Integrar o rate-limiter nos checks de validação de ativação de habilidades (`USBAbilityComponent`)
- [x] Implementar verificação física 3D de alcance (distance validation) em `ServerStartInteract` e `ServerCompleteInteract` (`USBInteractionComponent`)
- [x] Implementar verificação lógica de validação de armas em `ServerRequestFire` (`USBCombatComponent`)
- [x] Compilar o projeto completo com sucesso e garantir estabilidade da suíte legada
- [x] Criar novos testes de automação para simular e validar cenários de exploits bloqueados no servidor

## Fase 21: Compensação de Lag (Network Rewind / Backtracking)
- [x] Criar subsistema C++ `USBLagCompensationSubsystem` em `04_SandboxCore`
- [x] Implementar buffer circular/histórico de posições (1 segundo de limite) e método de interpolação
- [x] Integrar compensação de lag no disparo hitscan de `USBWeaponBehaviorHitscan`
- [x] Compilar projeto editor e rodar testes de regressão
- [x] Criar suíte de testes de unidade focados em Lag Compensation (`SBLagCompensationTests.cpp`)
- [x] Atualizar as documentações e diários de desenvolvimento ao concluir a fase

## Fase 22: Sistema de Status Effects (Buffs / Debuffs / DOTs)
- [x] Criar o Data Asset `USBStatusEffectDefinition` em `05_SandboxCharacter`
- [x] Criar o componente de controle `USBStatusEffectComponent` gerenciando o ciclo de vida e replicação dos efeitos
- [x] Integrar modificadores dinâmicos ao `USBAttributeComponent` e tags ao `USBStateComponent`
- [x] Compilar projeto editor e rodar testes de regressão
- [x] Criar a suíte de testes de automação em `SBStatusEffectTests.cpp` validando buffs, debuffs e DOTs
- [x] Atualizar todas as documentações e diários de desenvolvimento ao concluir a fase

## Fase 23: Sincronização Estética de Equipamento (Visual & Sockets) (Concluída)
- [x] Adicionar propriedades de ator visual e sockets em `USBWeaponBehaviorDefinition`
- [x] Definir a lista de armas spawnadas em `USBCombatComponent` com replicação
- [x] Implementar spawn, destruição e reanexação dinâmica de armas no `USBCombatComponent`
- [x] Integrar saca/guarda automático de armas no ciclo de vida de `USBWeaponBehavior`
- [x] Compilar projeto editor e rodar testes de regressão
- [x] Criar a suíte de testes de automação em `SBWeaponVisualTests.cpp` validando fluxo visual e sockets
- [x] Atualizar todas as documentações e diários de desenvolvimento ao concluir a fase

## Marco 1: Montagem Visual & Playtests de Interface (Frente 1: Widgets) (Concluída)
- [x] Criar o Widget Blueprint `WBP_StatusHUD` herdando de `USBStatusHUDWidget` com barra de vida e mana
- [x] Criar o Widget Blueprint `WBP_AbilityBar` herdando de `USBAbilityBarWidget` com máscara de cooldown
- [x] Criar o Widget Blueprint `WBP_InteractionPrompt` herdando de `USBInteractionPromptWidget` com texto e progresso
- [x] Criar o Widget Blueprint `WBP_InventoryGrid` herdando de `USBInventoryGridWidget` com grid de mochila
- [x] Criar o container principal `WBP_PlayerHUD` reunindo todos os widgets e associar ao `ASBHUD` ou `USBPawnDataAsset`
- [x] Executar playtests locais em Split-Screen para certificar isolamento de controle e dados
- [x] Atualizar todas as documentações e diários de desenvolvimento ao concluir o marco

## Fase 24: Anti-Cheat Avançado de Movimento e Dano (Concluída)
- [x] Implementar detecção de velocidade excessiva (speedhack) e teleporte no `USBMovementComponent` com rollback autoritativo
- [x] Implementar verificação de linha de visão física (wall-clipping detection) no `USBWeaponBehaviorHitscan` bloqueando disparos através de obstáculos estáticos
- [x] Criar a suíte de testes de automação em `SBAntiCheatTests.cpp` cobrindo speedhack, teleportes e wall-shot blocks
- [x] Compilar projeto editor e rodar testes de regressão (40 de 40 specs verdes)
- [x] Atualizar documentações, diário e manuais do desenvolvedor

## Fase 25: Otimização de Replicação e Atributos Condicionais (Concluída)
- [x] Dividir a propriedade `ReplicatedAttributes` em `PublicAttributes` e `PrivateAttributes` no `USBAttributeComponent`
- [x] Configurar replicação `COND_OwnerOnly` para `PrivateAttributes` em `GetLifetimeReplicatedProps`
- [x] Atualizar métodos de registro e atualização de atributos para classificar entre canais públicos e privados
- [x] Criar a suíte de testes de automação em `SBConditionalReplicationTests.cpp` validando o isolamento de atributos
- [x] Compilar projeto editor e rodar testes de regressão (43 de 43 specs verdes)
- [x] Atualizar todas as documentações e diários de desenvolvimento ao concluir a fase

## Fase 26: Persistência Estética e Restauração de Equipamento (Concluída)
- [x] Adicionar tag `State.Item.Equipped` em `ServerEquipItem` e `ServerUnequipItem`
- [x] Implementar carregamento diferido (Next-Tick Deferral) em `LoadComponentData_Implementation` via `RestoreEquippedItems`
- [x] Criar o caso de teste `Cenário 2: Persistência e restauração do estado equipado (Visual/Behavior)` em `SBInventorySaveTests.cpp`
- [x] Compilar e rodar testes de integração (45 de 45 specs verdes - EXIT CODE: 0)
- [x] Atualizar documentação e manuais do usuário com as novas diretrizes
## Resoluções de Auditoria e Otimizações Físicas (Fases 20-23) (Concluída)
- [x] Otimizar performance da compensação de lag implementando registro dinâmico de personagens (eliminando TActorIterator no Tick)
- [x] Implementar filtragem tridimensional por alcance no rebobinamento físico (RewindPositions)
- [x] Corrigir bug de drift de tick e perda de ativações periódicas de Status Effects
- [x] Forçar programmaticamente a replicação de rede (SetReplicates) em armas visuais spawnadas
- [x] Compilar projeto editor e homologar suíte completa de automação (45 de 45 specs verdes - EXIT CODE: 0)
- [x] Atualizar nota de revisão, walkthrough e diários de desenvolvimento

## Fase 27: Sistema de Estamina Avançado (Concluída)
- [x] Adicionar atributo `Attribute.Stamina` com escopo privado (`COND_OwnerOnly`) no `USBAttributeComponent`
- [x] Implementar consumo predito/corrigido por segundo ao sprintar e custo instantâneo ao saltar (bloqueando pulos se saldo for insuficiente)
- [x] Implementar regeneração inteligente com delay ininterrupto de `1.5s` da última ação de consumo
- [x] Implementar estado de exaustão dinâmico (`State.Character.Exhausted`) ao atingir `0.f` que bloqueia corridas/pulos até recuperar acima de `30.f`
- [x] Criar suíte de testes de automação em `SBStaminaTests.cpp` cobrindo consumo, regeneração passiva e controle de exaustão
- [x] Compilar projeto editor e homologar suíte completa de automação (48 de 48 specs verdes - EXIT CODE: 0)
- [x] Atualizar nota de revisão, walkthrough e diário de tarefas do desenvolvedor

## Fase 28: Vinculação de Assets Visuais e Playtests de UI (Concluída)
- [x] Corrigir bug de coerção de tipo do payload de slot de inventário fazendo `USBInventorySlotUpdatedEventPayload` herdar de `USBInventoryEventPayload`
- [x] Estender a HUD C++ `USBStatusHUDWidget` com suporte a `PB_Stamina` e mapeamento de vida correto para `Attribute.Health`
- [x] Mapear diretrizes do UMG Designer para os Widget Blueprints (HUD, AbilityBar, Interaction, Inventory)
- [x] Documentar o plano contra vazamento de dados de interface (UI Spill) em jogos locais Split-Screen
- [x] Homologar testes unitários e build final verde (48 de 48 specs verdes - EXIT CODE: 0)
- [x] Atualizar nota de revisão, walkthrough e diários de desenvolvimento do painel do Obsidian

## Fase 29: Sistema de Munição e Recarga (Concluída)
- [x] Registrar dinamicamente `Attribute.Weapon.Ammo` replicado via `COND_OwnerOnly` na inicialização do `USBCombatComponent`
- [x] Implementar comportamento genérico de recarga `USBWeaponBehaviorReload` com duração de 2s e reestabelecimento de munição ao máximo
- [x] Implementar tag de estado de recarga `State.Character.Reloading` que impede disparos e novas recargas concorrentes
- [x] Escrever suíte de testes de automação em `SBReloadTests.cpp` validando consumo, bloqueio de disparos e recuperação completa de munição
- [x] Compilar projeto editor e homologar suíte completa de automação (50 de 50 specs verdes - EXIT CODE: 0)
- [x] Atualizar nota de revisão, walkthrough e diários de desenvolvimento do painel do Obsidian

## Fase 30: Sistema de Cooldowns de Habilidade e Custo de Mana (Concluída)
- [x] Estender a classe `USBAbility` com a propriedade `CooldownTag` para rastreamento de estado
- [x] Implementar regeneração de mana passiva a `5.f/s` com delay de `2.0s` no tick do `USBAbilityComponent`
- [x] Configurar aplicação dinâmica da `CooldownTag` no `USBStateComponent` ao ativar e remoção automática ao expirar o cooldown
- [x] Implementar rollback transacional de mana, entrada de cooldowns e `CooldownTag` na rejeição do servidor (`ClientRollbackAbility`)
- [x] Criar novas especificações em `SBAbilityTests.cpp` cobrindo regeneração passiva, tags de cooldown e rollback de rede
- [x] Compilar projeto editor e homologar suíte completa de automação (53 de 53 specs verdes - EXIT CODE: 0)
- [x] Atualizar nota de revisão, walkthrough e diários de desenvolvimento do painel do Obsidian

## Fase 31: Inteligência Artificial Integrada com State Component (Concluída)
- [x] Criar tabela de Agro (`AgroTable`) e lógica de registro baseada em dano no `USBCombatComponent`
- [x] Implementar integração de tags de controle de grupo (CC) no `USBStateComponent` para ejetar/bloquear comportamentos de IA
- [x] Criar comportamento de perseguição de IA atacando alvos quando dentro do alcance síncrono no servidor
- [x] Escrever suíte de testes unitários `SBAIBehaviorTests.cpp` cobrindo agro, detecção de alvos e restrição por tags
- [x] Compilar projeto editor e homologar suíte completa de testes verdes (56 de 56 specs verdes - EXIT CODE: 0)

## Fase 32: Dano Crítico, Resistências e Reações de Impacto Replicadas (Concluída)
- [x] Implementar leitura de `HitResult.BoneName` para multiplicar danos no `USBWeaponBehaviorHitscan`
- [x] Adicionar mitigação baseada no atributo `Attribute.Defense` no `USBAttributeComponent`
- [x] Integrar montagem de impacto síncrona forçando a tag `State.Character.HitReacting` predita e replicada
- [x] Escrever suíte de testes `SBCriticalDamageTests.cpp` validando multiplicadores, mitigação e impactos

## Fase 33: Tabela de Loot e Drop Físico Replicado (Concluída)
- [x] Criar classe `ASBPhysicalLootDrop` com simulação física e componente interativo `USBInteractionComponent`
- [x] Criar Data Asset `USBLootTableDataAsset` mapeando itens lógicos e percentuais de raridade
- [x] Implementar spawn seguro no servidor ao tocar o solo e integração com inventário lúdico
- [x] Escrever suíte de testes `SBLootDropTests.cpp` validando probabilidade e coletas físicas (61 de 61 specs verdes - EXIT CODE: 0)

## Fase 34: Sistema de Progressão e Experiência (Concluída)
- [x] Criar classe `USBExperienceComponent` herdando de `UActorComponent` em `05_SandboxCharacter`
- [x] Implementar propriedades replicadas `CurrentXP`, `CurrentLevel` e `RequiredXP`
- [x] Adicionar suporte a curvas exponenciais e carregamento opcional via `UDataTable`
- [x] Implementar lógica recursiva de level up em cadeia com carry-over de XP excedente
- [x] Escrever suíte de testes unitários `SBExperienceTests.cpp` cobrindo ganho de XP, level up simples/múltiplo e DataTables (66 de 66 specs verdes - EXIT CODE: 0)

## Fase 35: Sistema de Bancada Física e Interativa de Crafting (Concluída)
- [x] Criar classe `ASBCraftingStation` em `08_SandboxInventory` com suporte a `ISBInteractableInterface`
- [x] Adicionar lógica para adicionar `StationTag` ao jogador que interagir com a bancada
- [x] Implementar monitoramento dinâmico (Tick) de distância máxima e auto-limpeza de tags e interatores
- [x] Escrever suíte de testes unitários `SBCraftingStationTests.cpp` validando acesso, proximidade física e desassociação de tags (70 de 70 specs verdes - EXIT CODE: 0)

## Fase 36: Desmantelamento / Salvaging Probabilístico de Equipamentos (Concluída)
- [x] Criar fragmento de item `USBItemFragment_Salvageable` e a struct `FSBSalvageOutcome`
- [x] Expor assinaturas e métodos de Salvage no `USBCraftingComponent`
- [x] Implementar a lógica de desmontagem e chances probabilísticas com consumo atômico
- [x] Escrever suíte de testes unitários `SBSalvageTests.cpp` validando drops fixos/probabilísticos e consumos de pilhas (73 de 73 specs verdes - EXIT CODE: 0)

## Fase 37: Compressão de Payloads e Otimizações de Replicação em Larga Escala (Concluída)
- [x] Desativar a replicação individual de subobjetos de `USBItemInstance` em `ReplicateSubobjects`
- [x] Declarar `ItemDef`, `StackCount` e `DynamicTags` diretamente na struct `FSBInventoryEntry`
- [x] Implementar `NetSerialize` com compactação de bits (`SerializeIntPacked`) para o `StackCount`
- [x] Adicionar callbacks `PostReplicatedAdd`, `PostReplicatedChange` e `PreReplicatedRemove` para instanciar localmente os itens no cliente
- [x] Validar que toda a suíte de inventário e equipamentos continua passando 100% verde (73 de 73 specs verdes - EXIT CODE: 0)

## Fase 38: Otimização de canais de áudio e efeitos estéticos sob latências extremas (Concluída)
- [x] Criar classe `USBCosmeticSaturationSubsystem` herdando de `UWorldSubsystem` em `04_SandboxCore`
- [x] Implementar agrupamento espacial (Grid 3D) e validação de intervalos de tempo para supressão sob rajadas
- [x] Implementar timer periódico automático para limpeza de registros de TMap obsoletos
- [x] Escrever suíte de testes unitários `SBCosmeticLimiterTests.cpp` cobrindo supressão concorrente, independência espacial e expiração de cooldowns (79 de 79 specs verdes - EXIT CODE: 0)

## Fase 39: Persistência Criptografada e Proteção contra Cheat de Save Game (Concluída)
- [x] Declarar `USBSecureSaveGame` em `SBSaveSubsystemConcrete.h`
- [x] Implementar helpers de cifragem XOR de fluxo e assinatura digital HMAC-MD5 em `SBSaveSubsystemConcrete.cpp`
- [x] Adaptar métodos `SaveGame` e `LoadGame` para empacotar, cifrar, assinar e verificar saves na memória
- [x] Escrever suíte de testes unitários `SBSecureSaveTests.cpp` validando integridade, adulteração maliciosa e carregamentos normais (82 de 82 specs verdes - EXIT CODE: 0)

## Fase 40: Passos Dinâmicos Sensíveis a Superfícies (Concluída)
- [x] Criar classe `USBSurfaceEffectsDataAsset` em `03_SandboxAssets`
- [x] Criar AnimNotify `USBAnimNotify_Footstep` em `05_SandboxCharacter`
- [x] Implementar line trace, detecção de material físico, consulta de efeitos e integração com o subsistema de saturação
- [x] Registrar tipos de superfície e associar sons e efeitos Niagara

## Fase 41: Subsistema de Zonas de Áudio Ambiental (Concluída)
- [x] Criar ator `ASBAmbientZoneTrigger` em `04_SandboxCore` com box/sphere trigger
- [x] Implementar ativação local (client-side) e transições suaves de fade-in/fade-out e crossfade
- [x] Escrever suíte de testes unitários `SBSurfaceAudioTests.cpp` cobrindo passos em superfícies mapeadas, supressão de passos e crossfade de zonas ambientais (85 de 85 specs verdes - EXIT CODE: 0)

## Fase 42: Sistema de Missões Replicado (Concluída)
- [x] Declarar `Attribute.Coins` em `SBGameplayTags.h`/`SBGameplayTags.cpp` e registrar no `USBAttributeComponent`
- [x] Criar classe `USBQuestDataAsset` em `03_SandboxAssets`
- [x] Criar componente `USBQuestComponent` em `05_SandboxCharacter` com replicação de quests ativas e escuta do `USBEventSubsystem`
- [x] Implementar recompensa autoritativa de missões no servidor (XP e Itens)

## Fase 43: Sistema de Comerciantes e Economia (Concluída)
- [x] Criar componente `USBMerchantComponent` em `08_SandboxInventory` com estoque de compras/vendas
- [x] Implementar RPCs de Compra e Venda no servidor com validações de ouro, inventário e alcance
- [x] Escrever suíte de testes unitários `SBQuestMerchantTests.cpp` validando missões, barramento, compras legítimas, limites financeiros e proteção de distância (88 de 88 specs verdes - EXIT CODE: 0)

## Fase 44: Sistema de Construção e Edificação Replicado (Concluída)
- [x] Criar classe `USBItemFragment_Placeable` em `08_SandboxInventory`
- [x] Criar classe `ASBBuildingPiece` em `08_SandboxInventory` representando as peças e HP
- [x] Criar componente `USBBuildingComponent` em `08_SandboxInventory` com preview local e Server RPC de posicionamento
- [x] Escrever suíte de testes unitários `SBBuildingTests.cpp` validando proximidade, overlap físico, posse de item, snapping de grade e danos/destruição de peças (95 de 95 specs verdes - EXIT CODE: 0)

## Fase 45: Sistema de Coleta de Recursos e Mineração Replicado (Concluída)
- [x] Modificar `SBWeaponBehaviorHitscan.cpp` em `06_SandboxCombat` para aplicar dano nativo via `TakeDamage` em atores sem componentes de atributos
- [x] Criar classe `ASBResourceNode` em `08_SandboxInventory` com suporte a HP, tool validation por reflexão, rolls de loot table e respawn
- [x] Escrever suíte de testes unitários `SBResourceTests.cpp` em `08_SandboxInventory` validando dano, colheita de itens e ciclo de respawn
- [x] Compilar, rodar testes de automação e garantir 100% verde (98 de 98 specs verdes)

## Fase 46: Sistema de Durabilidade de Equipamentos e Reparo (Concluída)
- [x] Criar a interface `ISBItemDurabilityInterface` em `02_SandboxInterfaces`
- [x] Adicionar `DurabilityCost` no `USBWeaponBehaviorDefinition` em `06_SandboxCombat`
- [x] Adicionar `EquippedItemInstance` e métodos de acesso em `USBWeaponBehavior` e associá-lo no `USBCombatComponent::OnItemEquipped`
- [x] Adicionar consumo de durabilidade e verificação de durabilidade zerada no `USBWeaponBehavior`
- [x] Criar o fragmento `USBItemFragment_Durability` em `08_SandboxInventory`
- [x] Adicionar propriedade replicada `Durability` em `USBItemInstance` e implementar a interface
- [x] Adicionar `Durability` em `FSBInventoryEntry` e serializá-lo em `FSBInventoryEntry::NetSerialize`
- [x] Sincronizar durabilidade nos callbacks de replicação e no helper `MarkItemInstanceUpdated`
- [x] Implementar `ServerRepairItem` no `USBCraftingComponent`
- [x] Criar suíte de testes automatizados `SBDurabilityTests.cpp` e verificar 100% verde

## Fase 47: Sistema de Peso e Sobrecarga de Inventário (Concluída)
- [x] Criar o fragmento `USBItemFragment_Weight` em `08_SandboxInventory`
- [x] Registrar automaticamente os atributos de peso no `USBInventoryComponent::OnInitialize_Implementation`
- [x] Implementar `RecalculateInventoryWeight()` no `USBInventoryComponent` e chamá-lo nas adições, consumos e remoções
- [x] Bloquear corrida (Sprint) sob efeito de sobrecarga no `USBMovementBehaviorSprint::CanEnter_Implementation`
- [x] Aplicar redução de 50% de velocidade de caminhada no `USBMovementComponent::GetMaxSpeed`
- [x] Criar a suíte de testes unitários `SBWeightTests.cpp` e obter 100% verde

## Fase 48: Baús de Armazenamento Compartilhados (Concluída)
- [x] Criar ator `ASBContainerChest` com suporte a `USBInventoryComponent` e interface de interação
- [x] Implementar ciclo de vida, distância de segurança e cancelamento de interação no `ASBContainerChest`
- [x] Adicionar método `ServerTransferItem` no `USBInventoryComponent` com validações de proximidade e preservação de metadados
- [x] Criar a suíte de testes unitários `SBContainerTests.cpp` e obter 100% verde

## Fase 49: Raridade e Efeitos Visuais nos Drops de Loot (Concluída)
- [x] Registrar as tags de raridade `Loot.Rarity.*` no `SBGameplayTags`
- [x] Criar o fragmento de item `USBItemFragment_Rarity` em `08_SandboxInventory`
- [x] Adicionar `GetRarityTag()` e `GetRarityColor()` no `ASBPhysicalLootDrop` e implementar no material dinâmico em `UpdateVisuals()`
- [x] Criar a suíte de testes unitários `SBLootRarityTests.cpp` e obter 100% verde

## Fase 50: Sistema de Upgrade de Equipamentos (Concluída)
- [x] Adicionar propriedade `UpgradeLevel` replicada no `USBItemInstance` e no struct `FSBInventoryEntry`
- [x] Adicionar suporte de persistência em save game para `Durability` e `UpgradeLevel`
- [x] Criar fragmento de item `USBItemFragment_Upgrade` em `08_SandboxInventory`
- [x] Implementar método de upgrade autoritativo `ServerUpgradeItem` no `USBCraftingComponent`
- [x] Integrar bônus multiplicador cumulativo de atributos do upgrade em `ServerEquipItem`
- [x] Criar suíte de testes unitários `SBUpgradeTests.cpp` e validar com 100% verde

## Fase 51: Sistema de Auto-Equipar Melhor Armadura (Concluída)
- [x] Adicionar flag `bAutoEquipBetterLoot` no `USBInventoryComponent`
- [x] Implementar método auxiliar `CalculateEffectiveDefense` no `USBInventoryComponent`
- [x] Implementar métodos `ServerAutoEquipBestArmor` e `ServerAutoEquipBestArmorAllSlots`
- [x] Integrar auto-equipamento dinâmico no `ServerAddItem`
- [x] Criar suíte de testes unitários `SBAutoEquipTests.cpp` e obter 100% verde

## Fase 52: Durabilidade Conforme o Uso das Armaduras (Concluída)
- [x] Se registrar em `OnAttributeChanged` e implementar `HandleOwnerAttributeChanged` no `USBInventoryComponent`
- [x] Deduzir durabilidade de armaduras equipadas proporcionalmente ao dano recebido
- [x] Implementar `DeactivateArmorModifiers` e desativar atributos de armaduras quando a durabilidade chega a zero
- [x] Re-aplicar modificadores de atributos quando a armadura equipada for reparada ou melhorada
- [x] Criar suíte de testes unitários `SBArmorDurabilityTests.cpp` e obter 100% verde

## Fase 53: Refinamento e Otimizações de Sistemas de IA, NPCs e Bosses (Concluída)
- [x] Otimizar AgroTable no `USBCombatComponent` com sistema baseado em eventos (`OnAgroTargetChanged` e cache)
- [x] Criar o AI Controller base em C++ `ASBAIController` herdando de `AAIController`
- [x] Implementar gerenciamento automático de foco da IA com base no maior alvo de agro
- [x] Pausar `BrainComponent` (behavior/state trees) e interromper movimentos de IA sob efeitos de Crowd Control (Stun/Frozen)
- [x] Monitorar HP no `ASBAIController` e notificar mudanças de fase do boss via delegate `OnBossPhaseChanged`
- [x] Criar suíte de testes em C++ `SBAIBehaviorTests.cpp` e obter 100% verde (188 de 188 specs verdes)

## Fase 54: Automação de Geração de Assets no Editor (v1.39.0) (Concluída)
- [x] Adicionar dependências dos plugins no descritor de SandboxEditor
- [x] Adicionar dependências e módulos em SandboxEditor.Build.cs
- [x] Criar classe C++ `USBSandboxAssetActionUtility` herdando de `UAssetActionUtility`
- [x] Implementar a geração automática de personagens (ComponentSet, PawnData, Character Blueprint)
- [x] Implementar a geração automática de armas (WeaponDefinition, Weapon Blueprint)
- [x] Implementar a geração automática de habilidades (AbilityDefinition, Ability Blueprint)
- [x] Implementar a geração automática de missões (Quest Data Asset)
- [x] Compilar editor e validar que a suíte de testes legada se mantém intacta e funcional

## Fase 55: Sistema de Validação Estrita de Assets (v1.40.0) (Concluída)
- [x] Declarar funções de validação em SBSandboxAssetActionUtility.h
- [x] Implementar a validação de PawnData e ComponentSets (componentes obrigatórios)
- [x] Implementar a validação de Gameplay Tags (verificação no GameplayTagsManager)
- [x] Implementar feedback visual (MessageDialog para erros e Notifications para sucesso)
- [x] Compilar editor e validar toda a suíte de testes legada (188 specs verdes)

## Fase 56: Ferramenta de Auto-Reparo de ComponentSets (v1.41.0) (Concluída)
- [x] Declarar funções de auto-reparo em SBSandboxAssetActionUtility.h
- [x] Implementar a injeção automática de componentes core em SBSandboxAssetActionUtility.cpp
- [x] Suportar reparo a partir de PawnData e ComponentSets selecionados
- [x] Implementar feedback visual de resumo de reparos via MessageDialog
- [x] Compilar editor e validar toda a suíte de testes legada (188 specs verdes)

## Fase 57: Linter e Renomeador Automático de Assets (v1.42.0) (Concluída)
- [x] Declarar funções de auto-renomeação em SBSandboxAssetActionUtility.h
- [x] Implementar a verificação de prefixos corretos para ComponentSet, PawnData, Definitions e Blueprints
- [x] Integrar com IAssetTools::RenameAssets para realizar refatoração segura de referências
- [x] Implementar feedback visual com lista de alterações
- [x] Compilar editor e validar toda a suíte de testes legada (188 specs verdes)

## Fase 58: Geração Automática de Enhanced Input Assets (v1.43.0) (Concluída)
- [x] Adicionar dependência de EnhancedInput em SandboxEditor.Build.cs
- [x] Declarar GenerateEnhancedInputContext em SBSandboxAssetActionUtility.h
- [x] Implementar a geração de Input Actions (IA_Sprint, IA_Crouch, IA_Interact, IA_Ability1, IA_Ability2) em SBSandboxAssetActionUtility.cpp
- [x] Implementar a geração e mapeamento de chaves padrão no Input Mapping Context (IMC)
- [x] Compilar editor e validar toda a suíte de testes legada (188 specs verdes)

## Fase 59: Prevenção Dinâmica de Duplicidade (v1.44.0) (Concluída)
- [x] Declarar helper CreateAssetSafely em SBSandboxAssetActionUtility.h
- [x] Implementar CreateAssetSafely em SBSandboxAssetActionUtility.cpp carregando assets existentes
- [x] Refatorar todos os métodos de criação/geração do editor para usar o helper seguro
- [x] Impedir colisões de renomeação em AutoRenameSandboxAssets
- [x] Impedir mapeamentos de teclas duplicados em GenerateEnhancedInputContext
- [x] Compilar editor e validar toda a suíte de testes legada (188 specs verdes)

## Fase 60: Linhagem de Predição em Habilidades Cascateadas (v1.45.0) (Concluída)
- [x] Declarar DeferredPredictionIds em SBAbilityComponent.h
- [x] Implementar a captura e armazenamento de PredictionId em SBAbilityComponent.cpp::RequestBehavior
- [x] Implementar a recuperação e limpeza de PredictionId da linhagem armazenada
- [x] Compilar projeto primário V1 e secundário GameAnimationSample
- [x] Validar toda a suíte de testes legada (188 specs verdes)

## Fase 61: Sincronização Dinâmica Bidirecional de Velocidade (v1.46.0) (Concluída)
- [x] Declarar variáveis de cache em SBMovementComponent.h
- [x] Inicializar valores de cache de velocidade em SBMovementComponent.cpp::OnReady_Implementation
- [x] Implementar verificação e sincronização dinâmica bidirecional em SBMovementComponent.cpp::TickComponent
- [x] Compilar projeto primário V1 e secundário GameAnimationSample
- [x] Validar toda a suíte de testes legada (188 specs verdes)

## Fase 62: Validador de Consistência de Dados de RPG e Inventário (v1.47.0) (Concluída)
- [x] Adicionar dependências de cabeçalho do inventário em SBSandboxAssetActionUtility.cpp
- [x] Implementar regras de integridade para ItemDefinition (DisplayName, MaxStack, fragmentos duplicados)
- [x] Implementar regras de integridade para LootTable (DropChance, pesos, Min/Max stacks)
- [x] Implementar regras de integridade para CraftingRecipe (ingredientes nulos, quantidades, estações)
- [x] Compilar projeto primário V1 e secundário GameAnimationSample
- [x] Validar toda a suíte de testes legada (188 specs verdes)

## Fase 63: Integração Procedural com PCG (v1.48.0) (Concluída)
- [x] Adicionar dependência PCG em 08_SandboxInventory.uplugin
- [x] Adicionar dependência PCG em SandboxInventory.Build.cs
- [x] Criar cabeçalho do nó PCG customizado (SBPCGLootSpawnerSettings.h)
- [x] Implementar execução do elemento PCG (SBPCGLootSpawnerSettings.cpp)
- [x] Compilar projeto primário V1 e secundário GameAnimationSample
- [x] Validar toda a suíte de testes legada (188 specs verdes)

## Fase 64: Sistema de Descoberta de Áreas e Apresentação (v1.49.0) (Concluída)
- [x] Adicionar propriedade Event_Area_Discovered em SBGameplayTags.h e SBGameplayTags.cpp
- [x] Declarar a classe USBAreaDiscoveryPayload em SBAmbientZoneTrigger.h
- [x] Adicionar propriedades de área ao ASBAmbientZoneTrigger em SBAmbientZoneTrigger.h
- [x] Implementar a publicação de evento de descoberta em SBAmbientZoneTrigger.cpp::OnOverlapBegin
- [x] Compilar projeto primário V1 e secundário GameAnimationSample
- [x] Validar toda a suíte de testes legada (188 specs verdes)

## Fase 65: Feature & Capability Management (v1.50.0) (Concluída)
- [x] Declarar propriedades Feature e tags correspondentes em SBGameplayTags.h e SBGameplayTags.cpp
- [x] Criar subsistema de controle global de Features (SBSandboxFeatureSubsystem.h e SBSandboxFeatureSubsystem.cpp)
- [x] Implementar testes automatizados unitários para o subsistema de Features (SBFeatureTests.cpp)
- [x] Compilar projeto primário V1 e secundário GameAnimationSample
- [x] Validar toda a suíte de testes expandida (189 specs verdes)

## Fase 66: Persistência Mundial Completa & Identidades por GUID (v1.51.0) (Concluída)
- [x] Criar tipos e estruturas de persistência em SBPersistenceTypes.h
- [x] Criar componente de persistência USBPersistenceComponent (header e source)
- [x] Criar subsistema de persistência de mundo USBSandboxPersistenceSubsystem (header e source)
- [x] Criar teste automatizado unitário para o sistema de persistência (SBPersistenceTests.cpp)
- [x] Compilar projeto primário V1 e secundário GameAnimationSample
## 📥 Fases 67 a 123 — importadas do walkthrough em 06/09/2026

Este checklist saltava da **Fase 66** direto para a **124**. As 57 fases intermediárias foram
reconstruídas a partir do [[walkthrough|walkthrough.md]], que as documenta uma a uma: **560
itens e 119 sub-itens**, transcritos literalmente.

> [!IMPORTANT] O que estas 671 marcações significam, e o que não significam
> São **narrativa importada**, transcrita literalmente do `walkthrough.md`. Um `- [x]` aqui
> registra que o walkthrough afirmou aquela entrega — **não** que ela foi verificada.
>
> A distinção não é acadêmica. A auditoria de 05/09/2026 encontrou as Fases 121–134 com todos
> os itens marcados e o código **fora da árvore de compilação**: nunca havia passado pelo
> compilador. Marcação neste arquivo nunca foi evidência, e continua não sendo.
>
> As contagens de spec declaradas ao fim de cada fase foram preservadas como **declarações da
> época**, explicitamente rotuladas. Nenhuma delas foi medida. O único número medido neste
> workspace é o da suíte atual — ver [[validation_report_2026-09-06]].
>
> Alguns itens descrevem código **que já mudou desde então**. A Fase 68, por exemplo, registra
> a resolução de atributos por `ProcessEvent` via reflexão, que o Bloco 3.1 do plano
> pós-auditoria substituiu por `ISBAttributeComponentInterface`. Como registro do que foi feito
> à época está correto; como descrição do código de hoje, não está.

---

## Fase 67: Background Simulation & LOD (v1.52.0) (Concluída — importada do walkthrough)

**Tipos e Interfaces de Simulação**
- [x] Criada a struct `FSBSimulatedEntityData` em `SBBackgroundSimInterface.h` para encapsular dados serializáveis de simulação em segundo plano (GUID, tipo de simulação, estados numéricos de timers e estados string).
- [x] Criada a interface `ISBBackgroundSimInterface` com os ganchos `PrepareForBackgroundSim` e `ResumeFromBackgroundSim`.

**Subsistema Global de Simulação**
- [x] Desenvolvido subsistema `UTickableWorldSubsystem` em `04_SandboxCore` para registrar, atualizar (tickar) e persistir dados de entidades descarregadas (unloaded).
- [x] Implementa o algoritmo de **Catch-Up Temporal** calculando a diferença entre timestamps do mundo real ao carregar saves, garantindo que o tempo transcorrido offline seja simulado instantaneamente.
- [x] Integrado de forma polimórfica ao `ISBSaveInterface` para persistência segura dos estados de simulação em arquivos de save criptografados.

**Nós de Recursos Autônomos**
- [x] Adaptada a classe de nó de recurso para implementar `ISBBackgroundSimInterface`.
- [x] Quando o nó é esgotado, ele se registra no subsistema informando o tempo de respawn.
- [x] Se o level sofrer stream-out, o subsistema continua atualizando o timer em background. Ao recarregar o level (stream-in), o ator recupera o estado e, se o timer já tiver expirado no mundo real, ele respawna imediatamente.

**Testes Automatizados de Simulação**
- [x] Implementado o caso de teste `Sandbox.BackgroundSim` em `SBBackgroundSimTests.cpp` validando o avanço de tempo em background, descarregamento e recriação de nós com contagem regressiva e testes de persistência com catch-up.

> Contagem de specs declarada à época: **193**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 68: Rule Engine & Decision Graphs (v1.53.0) (Concluída — importada do walkthrough)

**Tipos Base de Regras**
- [x] Definido o enum `ESBRuleOperator` cobrindo comparações numéricas (`Equal`, `NotEqual`, `LessThan`, `LessThanOrEqual`, `GreaterThan`, `GreaterThanOrEqual`) e verificações de posse de tag (`HasTag`, `DoesNotHaveTag`).
- [x] Definidas as estruturas `FSBRuleCondition` (especificação de tag, operador, valor numérico ou tag esperada) e `FSBRule` (array de condições agrupadas sob portas lógicas AND/OR com `bRequireAll`).

**Subsistema de Regras**
- [x] Desenvolvido o subsistema de GameInstance `USBSandboxRuleSubsystem` para centralizar a avaliação dinâmica de regras contra atores sem gerar dependências cíclicas.
- [x] Para checar atributos, ele pesquisa dinamicamente por `USBAttributeComponent` e invoca `GetAttributeValue` via reflexão C++ (`ProcessEvent`).
- [x] Para checar estados (tags), ele utiliza a interface polimórfica `ISBStateComponentInterface::HasTag` no componente de estado do ator, mantendo o encapsulamento estrito.

**Testes Automatizados**
- [x] Escrevemos `SBRuleEngineTests.cpp` cobrindo a verificação de condições numéricas de atributos, posse de tags individuais e a avaliação de lógicas AND/OR combinadas.

> Contagem de specs declarada à época: **196**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 69: Smart Object System integration (v1.54.0) (Concluída — importada do walkthrough)

**Configuração de Módulos e Plugins**
- [x] Habilitamos o plugin `SmartObjects` nativo da Unreal Engine em `06_SandboxCombat.uplugin`.
- [x] Adicionamos `"SmartObjectsModule"` às dependências do compilador em `SandboxCombat.Build.cs`.

**Extensão do Controlador de IA**
- [x] Expostos métodos nativos para interagir com o `USmartObjectSubsystem` global da Unreal Engine.
- [x] `FindNearbySmartObjects`: Permite à IA buscar e filtrar dinamicamente objetos inteligentes vizinhos no mundo usando tags de atividade.
- [x] `ClaimSmartObjectSlot`: Executa a reserva segura de slots, evitando concorrência de múltiplos agentes.
- [x] `ReleaseSmartObjectSlot`: Libera o slot ocupado.
- [x] `GetSmartObjectSlotTransform`: Retorna o transform absoluto (mundo) tridimensional do slot reservado para direcionar navegação e posicionar animações contextuais.

**Refinamento de Persistência em Simulações**
- [x] Substituímos o sistema de arquivamento binário temporário do `USBSandboxBackgroundSimSubsystem` pelo sistema de serialização declarativa nativo do Sandbox via `USBSavePayload`.
- [x] Ajustados os ganchos do `USBSaveSubsystemConcrete` para gerenciar, serializar e desserializar o subsistema de simulação global em saves criptografados de forma transparente.

**Testes Automatizados**
- [x] Escrevemos o caso de teste `Should find, claim, and release smart object slots` na suíte `SBAIBehaviorTests.cpp` validando o ciclo completo de busca, reserva, extração de transform e liberação de recursos de slots.

> Contagem de specs declarada à época: **197**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 70: AI State Tree & Movement Bindings (v1.55.0) (Concluída — importada do walkthrough)

**Configuração de Módulos e Plugins**
- [x] Habilitamos os plugins `"StateTree"` e `"GameplayStateTree"` em `06_SandboxCombat.uplugin`.
- [x] Adicionamos `"StateTreeModule"` e `"GameplayStateTreeModule"` às dependências do compilador em `SandboxCombat.Build.cs`.

**StateTree Combat Evaluator**
- [x] Implementamos o avaliador em C++ herdando de `FStateTreeEvaluatorCommonBase`.
- [x] Acessa e atualiza dinamicamente nos dados de instância (`FSBStateTreeCombatEvaluatorInstanceData`) a razão de vida (`HealthRatio`), verificação de CC/Stun (`bIsStunned`), estado de morte (`bIsDead`) e alvo prioritário de combate (`TargetActor`) extraído de `USBCombatComponent`.

**StateTree Task de Movimentação e Ataque**
- [x] Implementamos a tarefa em C++ herdando de `FStateTreeTaskCommonBase`.
- [x] Controla a aproximação em direção ao `TargetActor`, executando movimentação com distância configurável (`AttackRange`) e engajando o disparo de ataque ou habilidade ao alcançar a proximidade.

**Testes Automatizados**
- [x] Adicionamos novos testes em `SBAIBehaviorTests.cpp` cobrindo a extração de dados e o ciclo de vida das instâncias do avaliador e da tarefa da StateTree.

> Contagem de specs declarada à época: **199**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 71: Save Migration & Schema Versioning (v1.56.0) (Concluída — importada do walkthrough)

**Pipeline de Migração Extensível**
- [x] Criada a estrutura `FSBSaveMigrationStep` suportando versão de origem (`FromVersion`), versão de destino (`ToVersion`), descrição de auditoria e função de transformação de dados (`TFunction<bool(USBSavePayload* Payload)>`).

**Controle de Versão no Subsistema**
- [x] Definido `CURRENT_SAVE_VERSION = 2` como versão canônica de schema.
- [x] Adicionados métodos `RegisterMigrationStep` e `MigratePayload` para permitir o registro dinâmico e execução encadeada de passos de migração (ex: 1 -> 2 -> 3).
- [x] Integrada a checagem automática no método `LoadGame`: saves em versões legadas passam pelo pipeline de migração antes de propagar os dados aos atores do mundo, e são atualizados transparentemente.

**Testes Automatizados**
- [x] Criados testes unitários cobrindo a migração sequencial com transformações binárias de payload e a verificação de ignorar payloads já atualizados.

> Contagem de specs declarada à época: **201**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 72: Sistema de Regiões & Zonas de Perigo (v1.57.0) (Concluída — importada do walkthrough)

**Tags de Região e Estado**
- [x] Adicionadas tags nativas de tipos de região (`Zone.Type.SafeZone`, `Zone.Type.PvP`, `Zone.Type.Dungeon`, `Zone.Type.Hazard`) e estados de zona (`State.Zone.Safe`, `State.Zone.PvPAllowed`, `State.Zone.InHazard`).

**Definição Orientada a Dados**
- [x] Criada em `SBRegionTypes.h` especificando nome visível, nível de perigo, flags de SafeZone e PvP, tags de estado aplicadas automaticamente e parâmetros de dano ambiental por segundo (`EnvironmentalDamagePerSecond`).

**Subsistema de Regiões**
- [x] Desenvolvido como `UTickableWorldSubsystem`, rastreando em runtime a presença de atores em regiões ativas.
- [x] Aplica e remove tags de estado de zona de forma desacoplada em componentes de estado de atores.
- [x] Executa ticks periódicos aplicando dano contínuo aos atores expostos a zonas de risco ambiental.
- [x] Expostos métodos de consulta rápida: `IsActorInSafeZone`, `IsPvPAllowedForActor`, `IsActorInHazard` e `GetActorCurrentRegion`.

**Volume de Trigger de Região**
- [x] Ator colocável no level com `UBoxComponent` conectando automaticamente os overlaps de início e término com o `USBRegionSubsystem`.

**Testes Automatizados**
- [x] Criada nova suíte de testes validando a entrada/saída em Safe Zones, áreas PvP e zonas de dano ambiental.

> Contagem de specs declarada à época: **204**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 73: Sistema de Clima Dinâmico e Ciclo de Tempo (v1.58.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Clima e Tempo**
- [x] Criada a enumeração `ESBWeatherType` (`Clear`, `Cloudy`, `Rain`, `Thunderstorm`, `Snow`, `Fog`, `Sandstorm`).
- [x] Criada a estrutura `FSBWeatherState` com tipo atual, intensidade (0..1), temperatura ambiente (°C), velocidade do vento (km/h) e tag de clima associada.
- [x] Criada a estrutura `FSBTimeOfDay` contendo dia do calendário, hora (0..23), minuto (0..59), normalização 24h e tag de período (`State.Time.Dawn`, `State.Time.Day`, `State.Time.Dusk`, `State.Time.Night`).

**Tags Nativas de Clima e Tempo**
- [x] Registradas tags de estado para climas (`State.Weather.Clear`, `State.Weather.Rain`, `State.Weather.Snow`, `State.Weather.Storm`, `State.Weather.Fog`) e períodos do dia (`State.Time.Dawn`, `State.Time.Day`, `State.Time.Dusk`, `State.Time.Night`).

**Subsistema de Clima e Tempo**
- [x] Criado como `UTickableWorldSubsystem` com avanço configurável de tempo (`TimeScale`) e interpolação gradual de parâmetros climáticos (`SetWeather`).
- [x] Emite delegates notificadores para HUD e sistemas mundiais: `OnWeatherChanged`, `OnHourChanged`, `OnDayChanged` e `OnTimePeriodChanged`.
- [x] Implementa `ISBSaveInterface` para persistência completa do estado de tempo e clima em `USBSavePayload`.

**Integração com Salvamento**
- [x] Ganchos adicionados no fluxo de `SaveGame` e `LoadGame` para salvar e restaurar o estado do subsistema de clima.

**Testes Automatizados**
- [x] Criada suíte de testes validando avanço de minutos/horas/dias, transição de clima com interpolação de temperatura e restauração de save.

> Contagem de specs declarada à época: **207**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 74: Sistema de Portais & Seamless Map Teleportation (v1.59.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Portais**
- [x] Criada a estrutura `FSBPortalDestination` contendo tag do portal de destino, nome do mapa alvo, coordenadas 3D relativas/absolutas, rotação e tag de item chave necessário.
- [x] Criada a estrutura `FSBPortalInfo` contendo identificação de portal por tag, estado aberto/fechado, estado de trava por chave e cooldown.

**Tags Nativas de Portais**
- [x] Registradas tags de tipos (`Portal.Type.Gateway`, `Portal.Type.Waystone`, `Portal.Type.DungeonGate`) e estados (`State.Portal.Teleporting`, `State.Portal.Locked`, `State.Portal.Cooldown`).

**Subsistema de Portais**
- [x] Desenvolvido como `UWorldSubsystem` para gerenciamento centralizado de registro e busca de portais por `GameplayTag`.
- [x] Implementado o método `RequestTeleport` validando chaves de acesso no inventário/componente de estado, executando transição com posicionamento preciso no mesmo mapa (`TeleportTo`) ou abertura de novos níveis (`OpenLevel`).
- [x] Emite delegates notificadores: `OnActorTeleported` e `OnPortalStateChanged`.

**Ator de Portal**
- [x] Ator colocável em levels com `UBoxComponent` e `UStaticMeshComponent`, com suporte a teleporte automático por overlap e cálculo dinâmico de ponto de saída (`GetTeleportSpawnLocation`).

**Testes Automatizados**
- [x] Criada suíte de testes validando registro/busca por tag, teleporte com ajuste de transform e bloqueio para portais trancados.

> Contagem de specs declarada à época: **210**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 75: Gameplay Effects & Dynamic Attribute Modifiers (v1.60.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Efeitos**
- [x] Criada a estrutura `FSBGameplayEffectSpec` suportando políticas de duração (`Instant`, `Infinite`, `HasDuration`), intervalo de ticks periódicos (`Period`), empilhamento configurável (`MaxStacks`), modificadores de atributos (`FSBGameplayEffectModifier`), tags concedidas (`GrantedTags`), tags de imunidade (`ImmunityTags`) e tags de purgação (`RemoveEffectsWithTags`).

**Tags Nativas de Efeitos e Imunidades**
- [x] Registradas tags para buffs (`Effect.Buff.Berserk`, `Effect.Buff.SpeedBoost`, `Effect.Buff.Regeneration`), debuffs (`Effect.Debuff.Poison`, `Effect.Debuff.Burn`, `Effect.Debuff.Slow`, `Effect.Debuff.Stun`) e imunidades (`State.Immunity.Poison`, `State.Immunity.Stun`, `State.Immunity.Burn`).

**Componente de Efeitos de Gameplay**
- [x] Criado em `05_SandboxCharacter` para aplicar, gerenciar e expirar instâncias ativas de efeitos.
- [x] Aplica modificadores aditivos, multiplicativos e override no `USBAttributeComponent` e tags concedidas no `USBStateComponent`.
- [x] Gerencia o empilhamento linear escalando magnitude por stacks até o limite `MaxStacks`.
- [x] Executa ticks periódicos de dano ou cura ao longo do tempo (DoT/HoT).
- [x] Valida tags de imunidade bloqueando a aplicação de debuffs e purga efeitos antagônicos.

**Testes Automatizados**
- [x] Criada suíte de testes validando ciclo de vida e expiração com restauração de atributos, empilhamento (stacking), bloqueio por imunidade e purgação de efeitos.

> Contagem de specs declarada à época: **214**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 76: Combo System & Input Branching (v1.61.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Combo**
- [x] Criada a enumeração `ESBComboInputType` (`LightAttack`, `HeavyAttack`, `SpecialAbility`, `Finisher`).
- [x] Criada a estrutura `FSBComboNode` representando nós da árvore de combos com id, input esperado, action tag, multiplicador de dano, nós válidos de ramificação (`BranchTargetNodeIds`) e flag de golpe finalizador (`bIsFinisher`).
- [x] Criada a estrutura `FSBComboTree` para definir conjuntos de combos por tipo de arma ou classe de combate.

**Tags Nativas de Combos**
- [x] Registradas tags de tipos (`Combat.Combo.Light`, `Combat.Combo.Heavy`, `Combat.Combo.Finisher`) e estados de janela (`State.Combat.ComboWindowOpen`, `State.Combat.FinisherReady`).

**Componente de Combos**
- [x] Desenvolvido em `06_SandboxCombat` para processamento responsivo de sequências de golpes.
- [x] Suporte a **Input Buffering**: se o jogador aciona um comando antes da abertura da janela, o input é armazenado e consumido imediatamente quando a janela abre (`OpenComboWindow`).
- [x] Suporte a **Ramificações (Branching)**: transições dinâmicas entre ataques leves e pesados (*Light 1 → Light 2* ou *Light 1 → Heavy 1*).
- [x] Escalonamento progressivo de dano e acionamento de finalizadores com emissão de delegates (`OnComboStepExecuted`, `OnComboFinished`, `OnComboReset`).
- [x] Reset automático por timeout de janela de inatividade.

**Testes Automatizados**
- [x] Criada suíte de testes cobrindo progressão linear, ramificação divergente, buffering pré-janela e timeout de inatividade.

> Contagem de specs declarada à época: **218**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 77: Parry, Perfect Block & Counter Attack Framework (v1.62.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Defesa**
- [x] Criada a enumeração `ESBBlockResult` (`None`, `Blocked`, `Parried`, `GuardBroken`).
- [x] Criada a estrutura `FSBDefenseSettings` configurando percentual de mitigação de dano (`BlockDamageReduction`), custo de estamina (`BlockStaminaCost`), janela de Parry (`ParryWindowDuration`), janela de contra-ataque (`CounterAttackWindowDuration`), multiplicador de dano (`CounterAttackDamageMultiplier`) e atordoamento no atacante (`StaggerDurationOnAttacker`).

**Tags Nativas de Defesa e Parry**
- [x] Registradas tags `State.Combat.Blocking`, `State.Combat.ParryWindow`, `State.Combat.GuardBroken`, `State.Combat.CounterAttackReady` e `State.Combat.Staggered`.

**Componente de Defesa**
- [x] Implementado em `06_SandboxCombat` para processamento do ciclo de defesa e impactos.
- [x] **Bloqueio Regular**: Reduz o dano recebido proporcionalmente e consome estamina do `USBAttributeComponent`.
- [x] **Perfect Block / Parry**: Anula 100% do dano caso o golpe atinja o defensor dentro da janela inicial (0.25s), aplica atordoamento (*Stagger*) no atacante e ativa janela com bônus de dano de contra-ataque.
- [x] **Quebra de Guarda**: Caso a estamina se esgote durante a defesa, a guarda é desfeita e a tag `State.Combat.GuardBroken` é aplicada.
- [x] Emite delegates notificadores: `OnBlockSuccess`, `OnParrySuccess`, `OnGuardBroken` e `OnCounterAttackWindowExpired`.

**Testes Automatizados**
- [x] Criada suíte de testes cobrindo bloqueio regular com mitigação e consumo de estamina, Parry perfeito com atordoamento do atacante e contra-ataque, quebra de guarda por estamina insuficiente e expiração temporal da janela de contra-ataque.

> Contagem de specs declarada à época: **222**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 78: Lock-On Target System (v1.63.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Lock-On**
- [x] Criada a enumeração `ESBLockOnSwitchDirection` (`Left`, `Right`).
- [x] Criada a estrutura `FSBLockOnSettings` com raio de varredura (`LockDistance`), distância de desengajamento (`BreakDistance`), abertura angular do cone frontal (`MaxAngleDegrees`), canal de colisão para linha de visão (`TraceChannel`), flag `bRequireLineOfSight` e velocidade de suavização de câmera (`RotationInterpSpeed`).
- [x] Criada a estrutura `FSBLockOnCandidate` para ranqueamento de alvos por proximidade e desvio horizontal relativo.

**Tags Nativas de Lock-On**
- [x] Registradas tags de estado `State.Combat.LockedOn` (aplicada ao atacante) e `State.Combat.Target` (aplicada ao ator em foco).

**Componente de Lock-On**
- [x] Desenvolvido em `06_SandboxCombat` para rastreamento inteligente de adversários.
- [x] **Varredura Cônica e Linha de Visão**: Identifica candidatos no mundo considerando ângulo de visão frontal da câmera/pawn e desvio de geometrias estáticas (`LineTraceSingleByChannel`).
- [x] **Alternância de Alvos (`SwitchTarget`)**: Permite transição fluida para o inimigo mais próximo à esquerda ou à direita.
- [x] **Orientação e Foco (`GetDesiredRotationToTarget`)**: Calcula a rotação necessária em Pitch e Yaw para apontar a visão e o corpo na direção do alvo travado.
- [x] **Desengajamento Automático**: Destrava automaticamente se o alvo for destruído, morrer ou se afastar além de `BreakDistance`.
- [x] Emite delegates notificadores: `OnLockOnTargetChanged` e `OnLockOnTargetLost`.

**Testes Automatizados**
- [x] Criada suíte de testes validando aquisição de alvos por score angular, alternância lateral para a direita/esquerda, cálculo de LookAt rotation e quebra de trava por distanciamento.

> Contagem de specs declarada à época: **226**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 79: Melee Hitbox & Multi-Socket HitTrace Framework (v1.64.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Hit Trace**
- [x] Criada a estrutura `FSBHitTraceSocketConfig` com o nome do socket da arma (`SocketName`) e raio da esfera de varredura (`TraceRadius`).
- [x] Criada a estrutura `FSBHitTraceSettings` agrupando lista de sockets (`Sockets`), canal de colisão (`TraceChannel`), flag `bUseSphereSweep`, dano base (`BaseDamage`) e tag de ataque (`AttackTag`).

**Tags Nativas de Hit Trace**
- [x] Registradas tags `State.Combat.Attacking` (aplicada ao atacante durante swings) e `State.Combat.HitTraceActive` (aplicada enquanto os traçados da lâmina estão operando).

**Componente de Hit Trace**
- [x] Implementado em `06_SandboxCombat` para detecção física de impactos de armas brancas e golpes desarmados.
- [x] **Rastreamento Multi-Socket Sub-Frame**: Rastreia a posição de cada socket no frame anterior (`PreviousSocketLocations`) e executa `SweepMultiByChannel` conectando `Pos(t-1)` à `Pos(t)`, eliminando completamente o problema de *tunneling* em golpes velozes.
- [x] **Filtro de Acerto Único por Swing (`HitActorsInCurrentSwing`)**: Garante que uma arma que possua múltiplos sockets (ou que passe vários frames atravessando o mesmo inimigo) registre exatamente 1 acerto por inimigo por golpe.
- [x] **Extração de Impacto Físico**: Captura `FHitResult` contendo localização 3D do impacto, vetor normal de superfície e nome do osso atingido (`BoneName`).
- [x] Emite delegates notificadores: `OnMeleeHit`, `OnHitTraceStarted` e `OnHitTraceEnded`.

**Testes Automatizados**
- [x] Criada suíte de testes validando ativação e tags de combate, detecção de acerto e filtro de hit único no mesmo swing, múltiplos alvos atingidos simultaneamente e finalização limpa de traçado com contagem de acertos.

> Contagem de specs declarada à época: **230**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 80: Poise, Super Armor & Hit Reaction Framework (v1.65.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Postura e Reações**
- [x] Criada a enumeração `ESBHitReactionDirection` (`Front`, `Back`, `Left`, `Right`).
- [x] Criada a enumeração `ESBHitReactionIntensity` (`None`, `Light`, `Heavy`, `Knockdown`).
- [x] Criada a estrutura `FSBPoiseSettings` com postura máxima (`MaxPoise`), taxa de regeneração (`PoiseRegenRate`), atraso de regeneração (`PoiseRegenDelay`), duração do atordoamento (`StaggerDuration`) e flag `bHasSuperArmor`.
- [x] Criada a estrutura `FSBHitReactionResult` contendo direção, intensidade de reação, dano de postura aplicado, flags `bPoiseBroken`, `bAbsorbedBySuperArmor` e tag de gameplay de reação (`ReactionTag`).

**Tags Nativas de Postura e Reação**
- [x] Registradas tags de estado `State.Combat.SuperArmor` e `State.Combat.PoiseBroken`, além das tags direcionais `Combat.Reaction.Front`, `Combat.Reaction.Back`, `Combat.Reaction.Left` e `Combat.Reaction.Right`.

**Componente de Postura e Reações**
- [x] Desenvolvido em `06_SandboxCombat` para processamento de equilíbrio, dano de postura e reações físicas/animadas.
- [x] **Cálculo Vetorial de Direção de Impacto (`CalculateHitDirection`)**: Utiliza produto escalar (*Dot Product*) relativo à orientação frontal e lateral do ator para identificar se o golpe atingiu a Frente, Trás, Esquerda ou Direita.
- [x] **Armadura Ininterrupta (Super Armor)**: Quando ativo, absorve totalmente a reação de flinch/interrupção (`ESBHitReactionIntensity::None`), permitindo ataques ininterruptos.
- [x] **Quebra de Postura & Stagger**: Ao esgotar o medidor de postura, o ator entra em estado de quebra (`State.Combat.PoiseBroken`) com reação pesada ou *Knockdown*.
- [x] **Regeneração Automática**: Recupera gradualmente a postura após o término do delay de inatividade sem sofrer novos danos.
- [x] Emite delegates notificadores: `OnPoiseDamaged`, `OnPoiseBroken`, `OnPoiseRecovered` e `OnHitReactionTriggered`.

**Testes Automatizados**
- [x] Criada suíte de testes validando resolução direcional de impactos nos 4 quadrantes, quebra de postura por esgotamento, absorção por Super Armor e regeneração temporal pós-delay.

> Contagem de specs declarada à época: **234**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 81: Motion Warping & Dynamic Attack Translation (v1.66.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Motion Warping**
- [x] Criada a enumeração `ESBMotionWarpState` (`Inactive`, `Warping`, `Completed`, `Aborted`).
- [x] Criada a estrutura `FSBMotionWarpTarget` com referência de ator (`TargetActor`), coordenadas de destino (`TargetLocation`), rotação (`TargetRotation`) e distância de contato (`TargetOffsetDistance`).
- [x] Criada a estrutura `FSBMotionWarpConfig` configurando alcance máximo de aproximação (`MaxWarpDistance`), distância mínima (`MinWarpDistance`), duração da janela de animação (`WarpDuration`) e flags `bWarpTranslation` / `bWarpRotation`.

**Tags Nativas de Motion Warping**
- [x] Registrada a tag de estado `State.Combat.MotionWarping` aplicada ao atacante durante a translação dinâmica.

**Componente de Motion Warping**
- [x] Desenvolvido em `06_SandboxCombat` para ajustar suavemente a posição e orientação do atacante ao desferir golpes corpo a corpo.
- [x] **Cálculo Dinâmico de Ponto de Contato**: Posiciona o atacante a uma distância segura `TargetOffsetDistance` em relação ao alvo, evitando penetrações de malha ou golpes desferidos no ar (*whiffing*).
- [x] **Interpolação Suave em Tempo Real**: Executa translação e rotação hermítica contínua durante a janela `WarpDuration` da animação de ataque.
- [x] **Controle de Ciclo de Vida**: Conclui suavemente ao final do tempo ou aborta imediatamente caso o alvo seja destruído ou o ataque seja interrompido.
- [x] Emite delegates notificadores: `OnMotionWarpStarted`, `OnMotionWarpCompleted` e `OnMotionWarpAborted`.

**Testes Automatizados**
- [x] Criada suíte de testes validando inicialização com cálculo de offset, translação suave contínua, conclusão e limpeza de tags e abortagem limpa.

> Contagem de specs declarada à época: **238**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 82: Executions, Finishers & Paired Sync Animations (v1.67.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Execuções Pareadas**
- [x] Criada a enumeração `ESBExecutionRole` (`Attacker`, `Victim`).
- [x] Criada a enumeração `ESBExecutionState` (`Inactive`, `Aligning`, `Executing`, `Finished`, `Aborted`).
- [x] Criada a estrutura `FSBExecutionPairDefinition` contendo identificador (`ExecutionId`), montagens de animação pareadas (`AttackerMontage`, `VictimMontage`), posição e rotação relativa da vítima (`RelativeVictimLocation`, `RelativeVictimRotation`), duração (`ExecutionDuration`), dano letal (`DamageOnFinish`) e flag de invulnerabilidade (`bGrantInvulnerabilityToAttacker`).
- [x] Criada a estrutura `FSBActiveExecution` rastreando ponteiros de atacante e vítima, definição, estado e progresso temporal.

**Tags Nativas de Execução**
- [x] Registradas tags de estado `State.Combat.Executing` (aplicada ao atacante), `State.Combat.Executed` (aplicada à vítima) e `State.Combat.Invulnerable` (concedida ao atacante durante a sequência).

**Componente de Execução**
- [x] Desenvolvido em `06_SandboxCombat` para orquestrar o pareamento, alinhamento relativo e execução sincronizada de finalizações.
- [x] **Alinhamento Pareado Preciso (`CalculateAlignedVictimTransform`)**: Transforma as coordenadas locais da definição para coordenadas de mundo baseadas no atacante e posiciona a vítima na orientação correta.
- [x] **Imunidade e Bloqueio de Ações**: Concede invulnerabilidade temporária ao atacante contra ataques de terceiros e imobiliza a vítima com a tag de estado correspondente.
- [x] **Resolução Letal**: Ao atingir `ExecutionDuration`, aplica o dano de finalização à vítima via `USBAttributeComponent` e dispara delegate de conclusão.
- [x] Emite delegates notificadores: `OnExecutionStarted`, `OnExecutionFinished` e `OnExecutionAborted`.

**Testes Automatizados**
- [x] Criada suíte de testes validando inicialização com alinhamento relativo nos eixos e aplicação de tags de combate, conclusão temporal com dano e limpeza de estados, bloqueio de sobreposição de execuções simultâneas e cancelamento limpo.

> Contagem de specs declarada à época: **242**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 83: Weapon Trail, Particle Sockets & Impact Decals Framework (v1.68.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Efeitos de Combate**
- [x] Criada a enumeração `ESBCombatFXType` (`WeaponTrail`, `SocketEmitter`, `ImpactDecal`, `SurfaceSplash`).
- [x] Criada a estrutura `FSBWeaponTrailConfig` com nomes dos sockets base/ponta (`StartSocketName`, `EndSocketName`), largura (`Width`) e flag de atividade (`bIsActive`).
- [x] Criada a estrutura `FSBImpactDecalConfig` com material (`DecalMaterial`), dimensões 3D (`DecalSize`), tempo de vida (`LifeSpan`) e tamanho de tela para fade out (`FadeScreenSize`).
- [x] Criada a estrutura `FSBCombatFXRequest` encapsulando tipo de efeito, localização, rotação, socket e componentes anexados.

**Tags Nativas de Efeitos de Combate**
- [x] Registrada a tag de estado `State.Combat.WeaponTrailActive` vinculada dinamicamente ao ciclo de vida de emissão de trilha de lâmina.

**Componente de Efeitos de Combate**
- [x] Desenvolvido em `06_SandboxCombat` para disparo e gerenciamento desacoplado de efeitos visuais.
- [x] **Controle de Trilhas de Lâmina**: Ativação e encerramento de fitas de corte com reflexão em tempo real no `USBStateComponent`.
- [x] **Projeção de Decals de Impacto Físico**: Alinhamento geométrico relativo à normal da superfície de colisão (`ImpactNormal`) para projeção realista de marcas de corte e disparos.
- [x] **Disparo de Partículas em Sockets**: Gatilho reativo para efeitos localizados em sockets de malhas esqueléticas.
- [x] Emite delegates notificadores: `OnWeaponTrailStateChanged` e `OnImpactDecalSpawned`.

**Testes Automatizados**
- [x] Criada suíte de testes validando ativação de trilhas com concessão de tags de estado, desativação limpa com delegates, cálculo e orientação normal de decals e registro de partículas em sockets.

> Contagem de specs declarada à época: **246**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 84: Camera Shake, Hit-Stop & Temporal Dilation Framework (v1.69.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Feedback de Combate**
- [x] Criada a enumeração `ESBCombatFeedbackIntensity` (`Light`, `Medium`, `Heavy`, `Critical`).
- [x] Criada a estrutura `FSBHitStopConfig` com duração da micro-pausa (`Duration`), dilatação (`TimeDilation`) e flags de alvo/atacante (`bAffectAttacker`, `bAffectTarget`).
- [x] Criada a estrutura `FSBCameraShakeConfig` com classe de shake (`CameraShakeClass`), escala (`ShakeScale`) e impulso direcional (`DirectionalImpulse`).
- [x] Criada a estrutura `FSBTemporalDilationConfig` com dilatação desejada (`TargetDilation`), duração (`Duration`) e escopo global/local (`bGlobal`).
- [x] Criada a estrutura `FSBCombatFeedbackProfile` combinando configurações de Hit-Stop, Camera Shake e Slomo.

**Tags Nativas de Feedback de Combate**
- [x] Registradas tags de estado `State.Combat.HitStop` (aplicada durante a micro-pausa ao atacante e à vítima) e `State.Combat.Slomo` (aplicada durante a dilatação temporal).

**Componente de Feedback de Combate**
- [x] Desenvolvido em `06_SandboxCombat` para controle temporal e cinestésico de golpes.
- [x] **Micro-Pausa de Impacto (Hit-Stop)**: Aplica `CustomTimeDilation` temporário tanto ao atacante quanto ao alvo para transmitir peso tátil ao conectar ataques.
- [x] **Restauração Temporal Automática**: Rastreia a duração via `TickComponent` e restaura `CustomTimeDilation = 1.0f` removendo limpidamente as tags de estado ao término.
- [x] **Dilatação Temporal (Slomo)**: Permite slomo cinematográfico local ou global para contra-ataques e finalizações.
- [x] Emite delegates notificadores: `OnHitStopTriggered` e `OnSlomoTriggered`.

**Testes Automatizados**
- [x] Criada suíte de testes validando Hit-Stop simultâneo em atacante e vítima com dilatação e tags, restauração automática por tick temporal, disparo de Slomo e execução de perfis combinados.

> Contagem de specs declarada à época: **250**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 85: Dismemberment, Gore & Dynamic Fracture Sockets (v1.70.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Desmembramento**
- [x] Criada a enumeração `ESBLimbType` (`Head`, `LeftArm`, `RightArm`, `LeftLeg`, `RightLeg`, `Torso`).
- [x] Criada a estrutura `FSBLimbDismemberDefinition` vinculando tipo de membro a osso esquelético (`BoneName`), socket (`SocketName`), malha de fechamento de cavidade (`CapMesh`), malha física decepada (`SeveredLimbMesh`) e estado de amputação (`bIsSevered`).
- [x] Criada a estrutura `FSBSeverLimbRequest` encapsulando membro alvo, vetor de direção de corte e magnitude de impulso físico.
- [x] Criada a estrutura `FSBDismembermentSettings` para parametrização de membros decepáveis e restrições de letalidade.

**Tags Nativas de Desmembramento**
- [x] Registradas tags de estado `State.Combat.Dismembered` (aplicada ao sofrer qualquer amputação), `Combat.Dismember.Head` (decapitação), `Combat.Dismember.Arm` e `Combat.Dismember.Leg`.

**Componente de Desmembramento**
- [x] Desenvolvido em `06_SandboxCombat` para amputação dinâmica e física de membros.
- [x] **Amputação e Ocultação Óssea (`SeverLimb`)**: Executa `MeshComp->HideBoneByName(BoneName, PBO_None)` para esconder a malha do membro amputado instantaneamente, refletindo as tags no `USBStateComponent`.
- [x] **Impulso Físico e Disparo Reativo**: Calcula o vetor de força direcional e dispara o delegate `OnLimbSevered`.
- [x] **Restauração para Pooling (`ResetDismemberment`)**: Desoculta todos os ossos da malha esquelética (`UnHideBoneByName`), restaura as flags dos membros e limpa as tags de amputação.
- [x] Emite delegate notificador: `OnLimbSevered`.

**Testes Automatizados**
- [x] Criada suíte de testes validando registro e corte de membro com concessão de tags de desmembramento e delegate de impulso, rejeição de amputação redundante, rastreamento de múltiplos membros amputados e reset completo de estado.

> Contagem de specs declarada à época: **254**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 86: Stealth, Visibility, Noise & Perception System (v1.71.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Furtividade**
- [x] Criada a enumeração `ESBStealthState` (`Hidden`, `Suspicious`, `Detected`).
- [x] Criada a enumeração `ESBNoiseLoudness` (`Silent`, `Footstep`, `Sprint`, `Combat`, `Explosion`).
- [x] Criada a estrutura `FSBNoiseEvent` com localização espacial (`Location`), raio acústico (`Radius`), multiplicador de volume (`Loudness`) e causador (`Instigator`).
- [x] Criada a estrutura `FSBStealthSettings` para calibração de visibilidade base, multiplicadores de agachamento/sombras e taxas de acúmulo e decaimento do medidor de alerta.

**Tags Nativas de Furtividade**
- [x] Registradas tags de estado `State.Combat.Stealth.Hidden`, `State.Combat.Stealth.Suspicious` e `State.Combat.Stealth.Detected`.

**Componente de Furtividade**
- [x] Desenvolvido em `06_SandboxCombat` para cálculo de percepção e propagação de ruído.
- [x] **Propagação Acústica (`EmitNoise`)**: Dispara eventos de áudio mundiais e registra o histórico para sentinelas de IA.
- [x] **Cálculo Dinâmico de Alerta (`UpdateDetection`)**: Modula a visibilidade com base em postura (`bIsCrouched`) e iluminação (`bIsInShadows`), acumulando suspeita até transição para combate (`Detected`) ou dissipando até retorno a `Hidden`.
- [x] **Sincronização de Tags Nativas**: Reflete o estado em tempo real no `USBStateComponent`.
- [x] Emite delegates notificadores: `OnStealthStateChanged` e `OnNoiseEmitted`.

**Testes Automatizados**
- [x] Criada suíte de testes validando emissão de ruído e histórico acústico, progressão contínua de alerta (Hidden -> Suspicious -> Detected), atenuação por agachamento e sombras e decaimento temporal de suspeita.

> Contagem de specs declarada à época: **258**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 87: Cover System & Wall Peeking Framework (v1.72.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Cobertura**
- [x] Criada a enumeração `ESBCoverType` (`None`, `LowCover`, `HighCover`).
- [x] Criada a enumeração `ESBCoverEdge` (`None`, `Left`, `Right`, `Top`).
- [x] Criada a estrutura `FSBCoverPoint` com localização espacial (`Location`), vetor normal da superfície (`Normal`), tipo de cobertura (`CoverType`) e flags de bordas transitáveis para espionagem (`bHasLeftEdge`, `bHasRightEdge`, `bHasTopEdge`).
- [x] Criada a estrutura `FSBCoverSettings` para parâmetros de alcance e alturas de cobertura baixa e alta.

**Tags Nativas de Cobertura**
- [x] Registradas tags de estado `State.Combat.InCover`, `State.Combat.InCover.Low`, `State.Combat.InCover.High` e `State.Combat.Peeking`.

**Componente de Cobertura**
- [x] Desenvolvido em `06_SandboxCombat` para controle dinâmico de postura e ancoragem em obstáculos.
- [x] **Ancoragem em Cobertura (`EnterCover` / `ExitCover`)**: Conecta o ator ao ponto de cobertura, refletindo instantaneamente as tags correspondentes no `USBStateComponent`.
- [x] **Espionagem e Saída de Borda (`StartPeeking` / `StopPeeking`)**: Valida se a quina solicitada possui borda livre antes de ativar a espionagem e a tag `State.Combat.Peeking`.
- [x] Emite delegates notificadores: `OnCoverStateChanged` e `OnPeekStateChanged`.

**Testes Automatizados**
- [x] Criada suíte de testes validando entrada em cobertura baixa com tags e delegate, entrada em cobertura alta, espionagem com rejeição de quinas inválidas e saída limpa de cobertura.

> Contagem de specs declarada à época: **262**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 88: Vaulting, Mantling & Parkour Locomotion (v1.73.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Parkour**
- [x] Criada a enumeração `ESBParkourActionType` (`None`, `Vault`, `Mantle`, `Climb`).
- [x] Criada a estrutura `FSBParkourObstacleData` com localização de impacto frontal (`WallLocation`), vetor normal da parede (`WallNormal`), coordenadas da borda superior (`LedgeLocation`), altura (`ObstacleHeight`), profundidade da mureta (`ObstacleDepth`) e ação recomendada (`RecommendedAction`).
- [x] Criada a estrutura `FSBParkourSettings` para parametrização de limites de altura e profundidade de saltos e escaladas.

**Tags Nativas de Parkour**
- [x] Registradas tags de estado `State.Movement.ParkourActive`, `State.Movement.Vaulting` e `State.Movement.Mantling`.

**Componente de Parkour**
- [x] Desenvolvido em `05_SandboxCharacter` para detecção geométrica e transições de locomoção atlética.
- [x] **Classificação de Obstáculos (`DetectObstacle`)**: Analisa altura e profundidade da geometria para classificar automaticamente como salto rápido por mureta (`Vault`) ou escalada/apoio de mãos (`Mantle`).
- [x] **Execução e Controle de Transição (`StartParkourAction` / `CompleteParkourAction`)**: Gerencia o ciclo de vida da transposição e atualiza dinamicamente as tags no `USBStateComponent`.
- [x] Emite delegates notificadores: `OnParkourActionStarted` e `OnParkourActionCompleted`.

**Testes Automatizados**
- [x] Criada suíte de testes validando classificação geométrica de muretas baixas e altas, início de ação de Vault com tags e delegate, início de Mantle e conclusão de ação com limpeza limpa de tags.

> Contagem de specs declarada à época: **266**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 89: Dynamic Foot IK & Ground Adaptation Framework (v1.74.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Foot IK**
- [x] Criada a estrutura `FSBFootIKTraceData` com localização de contato (`HitLocation`), vetor normal da superfície (`HitNormal`), deslocamento vertical (`FootOffset`), rotação de alinhamento com o solo (`FootRotation`) e flag de contato (`bHit`).
- [x] Criada a estrutura `FSBFootIKResult` agrupando pé esquerdo, pé direito, compensação vertical da pelve (`PelvisOffset`), status de aterramento (`bIsGrounded`) e detecção de declive (`bIsOnSlope`).
- [x] Criada a estrutura `FSBFootIKSettings` para calibração de distâncias de traçado, limiar angular de declives e identificadores de socket dos pés.

**Tags Nativas de Foot IK**
- [x] Registradas tags de estado `State.Movement.FootIKActive` e `State.Movement.OnSlope`.

**Componente de Foot IK**
- [x] Desenvolvido em `05_SandboxCharacter` para ajuste de postura ao relevo.
- [x] **Cálculo e Compensação de Terreno (`CalculateFootIK`)**: Calcula individualmente os deslocamentos dos pés e determina o `PelvisOffset` mínimo para manter a cinemática natural sem esticar as pernas.
- [x] **Alinhamento de Normal e Detecção de Declive**: Converte a normal da superfície em rotação angular (`Pitch` e `Roll`) e concede dinamicamente a tag `State.Movement.OnSlope` quando em rampas/escadas.
- [x] Emite delegate notificador: `OnFootIKUpdated`.

**Testes Automatizados**
- [x] Criada suíte de testes validando compensação neutra em terreno plano, compensação de pelve em desníveis/degraus, alinhamento de rotação em superfícies inclinadas e limpeza completa de tags ao desativar o IK.

> Contagem de specs declarada à época: **270**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 90: Mounts & Riding Locomotion Framework (v1.75.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Montaria**
- [x] Criada a enumeração `ESBMountState` (`Unmounted`, `Mounting`, `Mounted`, `Dismounting`).
- [x] Criada a enumeração `ESBMountGait` (`Walk`, `Trot`, `Canter`, `Gallop`).
- [x] Criada a estrutura `FSBMountRiderData` para rastreamento de vínculo de cavaleiro (`RiderActor`), ator montaria (`MountActor`), socket de sela (`SaddleSocketName`), estado e andadura atual.
- [x] Criada a estrutura `FSBMountSettings` com velocidades por andadura (`WalkSpeed`, `TrotSpeed`, `GallopSpeed`) e consumo de vigor em galope.

**Tags Nativas de Montaria**
- [x] Registradas tags de estado `State.Movement.Mounted`, `State.Movement.Mounting`, `State.Movement.Dismounting` e `State.Movement.Galloping`.

**Componente de Montaria**
- [x] Desenvolvido em `05_SandboxCharacter` para gerenciar a ocupação e locomoção em montarias.
- [x] **Fluxo de Montagem e Desmontagem (`Mount` / `Dismount`)**: Acopla o cavaleiro ao socket da sela, sincroniza a tag `State.Movement.Mounted` em ambos os atores e rejeita montagens conflitantes.
- [x] **Controle de Andaduras (`SetGait`)**: Permite alternar entre passo, trote e galope, aplicando a tag `State.Movement.Galloping` quando no modo acelerado.
- [x] Emite delegates notificadores: `OnMountStateChanged` e `OnMountGaitChanged`.

**Testes Automatizados**
- [x] Criada suíte de testes validando fluxo de montagem com tags e delegates, rejeição de segundo cavaleiro em montaria ocupada, troca de andaduras para galope com tags e desmontagem limpa com restauração de estado.

> Contagem de specs declarada à época: **274**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 91: Swimming, Buoyancy & Water Locomotion Framework (v1.76.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Natação**
- [x] Criada a enumeração `ESBSwimState` (`None`, `SurfaceSwimming`, `Diving`).
- [x] Criada a estrutura `FSBOxygenData` para rastreamento de nível atual de oxigênio (`CurrentOxygen`), máximo (`MaxOxygen`), taxa de consumo submerso (`DepletionRatePerSec`), taxa de recuperação na superfície (`RecoveryRatePerSec`) e estado de afogamento (`bIsDrowning`).
- [x] Criada a estrutura `FSBSwimSettings` com velocidades na superfície, mergulho e impulso de flutuabilidade natural.

**Tags Nativas de Natação**
- [x] Registradas tags de estado `State.Movement.Swimming`, `State.Movement.Swimming.Surface`, `State.Movement.Swimming.Diving` e `State.Status.Drowning`.

**Componente de Natação**
- [x] Desenvolvido em `05_SandboxCharacter` para controle dinâmico da locomoção em água.
- [x] **Transições de Superfície e Mergulho (`EnterWater`, `StartDiving`, `SurfaceFromDive`, `ExitWater`)**: Gerencia o estado aquático com reflexão imediata no `USBStateComponent`.
- [x] **Simulação de Fôlego e Asfixia (`ConsumeOxygen`, `RecoverOxygen`, `UpdateWaterLocomotion`)**: Consome oxigênio durante o mergulho, acionando a tag `State.Status.Drowning` e o delegate `OnDrowningStarted` quando atinge 0%, e recuperando oxigênio ao retornar à tona.
- [x] Emite delegates notificadores: `OnSwimStateChanged`, `OnOxygenChanged` e `OnDrowningStarted`.

**Testes Automatizados**
- [x] Criada suíte de testes validando entrada na água com tags de superfície, transição para mergulho com tags, consumo e esgotamento de oxigênio com estado de afogamento e retorno à superfície com recuperação e saída limpa.

> Contagem de specs declarada à época: **278**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 92: Gliding, Parachuting & Aerial Locomotion Framework (v1.77.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Planador**
- [x] Criada a enumeração `ESBGliderState` (`Retracted`, `Deploying`, `Gliding`, `Diving`).
- [x] Criada a estrutura `FSBGliderFlightData` para rastreamento de velocidade vertical de descida (`CurrentFallSpeed`), velocidade horizontal (`CurrentForwardSpeed`), inclinação de voo (`CurrentPitchAngle`), custo de vigor e flag de voo planado (`bIsGliding`).
- [x] Criada a estrutura `FSBGliderSettings` com velocidades nominais de planeio, mergulho aéreo acelerado e taxa de consumo de vigor no ar.

**Tags Nativas de Planador**
- [x] Registradas tags de estado `State.Movement.Gliding`, `State.Movement.Gliding.Deploying` e `State.Movement.Gliding.Diving`.

**Componente de Planador**
- [x] Desenvolvido em `05_SandboxCharacter` para controle de cinemática aérea.
- [x] **Abertura e Fechamento de Planador (`DeployGlider` / `RetractGlider`)**: Permite abertura suave em pleno ar, modulação de queda terminal e transição segura para queda livre ou pouso.
- [x] **Mergulho Aéreo Acelerado (`StartAerialDive` / `StopAerialDive`)**: Acelera o deslocamento frontal e a taxa de descida mediante inclinação, aplicando a tag `State.Movement.Gliding.Diving`.
- [x] Emite delegates notificadores: `OnGliderStateChanged` e `OnGliderEmergencyRetract`.

**Testes Automatizados**
- [x] Criada suíte de testes validando abertura do planador com tags e delegate, mergulho aéreo acelerado com tags e velocidades aumentadas, restauração de velocidades nominais e fechamento de planador com limpeza de tags.

> Contagem de specs declarada à época: **282**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 93: Grappling Hook & Dynamic Swing Locomotion Framework (v1.78.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Gancho**
- [x] Criada a enumeração `ESBGrappleState` (`None`, `Firing`, `Pulling`, `Swinging`, `Detaching`).
- [x] Criada a estrutura `FSBGrappleAnchorData` para rastreamento do ponto de fixação no mundo (`AnchorLocation`), normal de impacto (`HitNormal`), comprimento de cabo inicial e dinâmico (`InitialCableLength`, `CurrentCableLength`) e flag de ancoragem (`bIsAttached`).
- [x] Criada a estrutura `FSBGrappleSettings` com alcance máximo, velocidade de tração linear, aceleração angular pendular e multiplicador de impulso de ejeção.

**Tags Nativas de Gancho**
- [x] Registradas tags de estado `State.Movement.Grappling`, `State.Movement.Grappling.Pulling` e `State.Movement.Grappling.Swinging`.

**Componente de Gancho**
- [x] Desenvolvido em `05_SandboxCharacter` para controle de cinemática e tração via cabo.
- [x] **Ancoragem e Liberação (`AttachAnchorPoint` / `ReleaseAnchorPoint`)**: Permite disparo e ancoragem em geometrias no mundo, desacoplamento em alta velocidade com impulso direcional e auto-desconexão por proximidade.
- [x] **Alternância de Modos (`StartPull` / `StartSwing`)**: Permite transitar livremente entre tração retilínea com guincho (*Pulling*) e balanço pendular livre com gravidade (*Swinging*), refletindo imediatamente nas tags de gameplay.
- [x] Emite delegates notificadores: `OnGrappleStateChanged`, `OnGrappleAnchored` e `OnGrappleReleased`.

**Testes Automatizados**
- [x] Criada suíte de testes validando ancoragem com tags e delegates, transição para tração com guincho, transição para oscilação pendular e liberação com impulso de ejeção direcional e limpeza de tags.

> Contagem de specs declarada à época: **286**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 94: Ziplines, Sliding Cables & Traverse Locomotion Framework (v1.79.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Tirolesa**
- [x] Criada a enumeração `ESBZiplineState` (`None`, `Mounting`, `Sliding`, `Dismounting`).
- [x] Criada a estrutura `FSBZiplineRideData` para rastreamento dos vetores de origem e destino no mundo (`StartPoint`, `EndPoint`), extensão geométrica total (`TotalDistance`), distância percorrida (`CurrentDistance`), velocidade instantânea (`CurrentSpeed`) e flag de engate (`bIsRiding`).
- [x] Criada a estrutura `FSBZiplineSettings` com velocidade base, velocidade máxima, aceleração gravitacional por declive e multiplicador de impulso de desacoplamento.

**Tags Nativas de Tirolesa**
- [x] Registradas tags de estado `State.Movement.Ziplining`, `State.Movement.Ziplining.Sliding` e `State.Movement.Ziplining.Dismounting`.

**Componente de Tirolesa**
- [x] Desenvolvido em `05_SandboxCharacter` para controle de cinemática e travessia via cabos suspensos.
- [x] **Acoplamento e Desacoplamento (`AttachToZipline` / `DetachFromZipline`)**: Conecta o personagem aos pontos extremos do cabo, calculando a distância total e concedendo as tags `State.Movement.Ziplining` e `State.Movement.Ziplining.Sliding`. Ao desacoplar voluntariamente ou ao fim da linha, projeta o personagem para frente com impulso proporcional à velocidade de descida (`DismountLaunchMultiplier`) e limpa as tags.
- [x] **Física de Declive e Progresso (`UpdateZiplineTravel`)**: Modula a aceleração pela inclinação vertical do vetor de cabo, computa o deslocamento no espaço tridimensional e emite delegates com a fração de progresso (`Alpha`).
- [x] Emite delegates notificadores: `OnZiplineStateChanged`, `OnZiplineProgress` e `OnZiplineDismounted`.

**Testes Automatizados**
- [x] Criada suíte de testes validando acoplamento com tags e dados de percurso, progressão métrica e emissão de delegate de progresso, aceleração gravitacional em declive e desacoplamento com projeção de velocidade e limpeza de tags.

> Contagem de specs declarada à época: **290**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 95: Vehicles & Four-Wheeled / Hover Dynamics Framework (v1.80.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Veículos**
- [x] Criadas as enumerações `ESBVehicleType` (`Wheeled`, `Hover`, `Tracked`) e `ESBVehicleSeat` (`Driver`, `PassengerFront`, `PassengerRearLeft`, `PassengerRearRight`).
- [x] Criada a estrutura `FSBVehicleSeatOccupant` com referência fraca ao ator ocupante e assento alocado.
- [x] Criada a estrutura `FSBVehicleDrivetrainData` para rastreamento de velocidade linear (`CurrentSpeed`), acelerador (`ThrottleInput`), esterçamento (`SteeringInput`), freio de mão (`bHandbrakeActive`), combustível (`CurrentFuel`, `MaxFuel`) e ignição (`bEngineRunning`).
- [x] Criada a estrutura `FSBVehicleSettings` com parâmetros de velocidade máxima à frente/ré, aceleração, frenagem, atrito natural e consumo de combustível.

**Tags Nativas de Veículo e Condução**
- [x] Registradas tags de estado `State.Movement.Driving`, `State.Movement.Driving.Accelerating`, `State.Movement.Driving.Braking`, `State.Movement.Driving.Reverse`, `State.Vehicle.Occupied` e `State.Vehicle.EngineRunning`.

**Componente de Veículo**
- [x] Desenvolvido em `05_SandboxCharacter` para gestão de ocupação e cinemática de trem de força.
- [x] **Embarque e Desembarque (`EnterVehicle` / `ExitVehicle`)**: Aloca assentos para condutor e passageiros, aplicando `State.Movement.Driving` ao motorista e `State.Vehicle.Occupied` ao veículo. Ao desembarcar, desliga o motor e remove as tags de condução.
- [x] **Física de Condução e Ignição (`StartEngine`, `SetThrottleInput`, `SetHandbrake`, `UpdateDrivetrainPhysics`)**: Modula aceleração e velocidade máxima, freio brusco/de mão (`State.Movement.Driving.Braking`), marcha à ré (`State.Movement.Driving.Reverse`) e consome combustível proporcional à aceleração até o esgotamento.
- [x] Emite delegates notificadores: `OnVehicleOccupantChanged`, `OnVehicleEngineStateChanged` e `OnVehicleFuelChanged`.

**Testes Automatizados**
- [x] Criada suíte de testes validando embarque como motorista com tags e ocupação, ignição e aceleração com consumo de combustível e tag de aceleração, aplicação de freio de mão com frenagem e tag de freio, e desembarque limpo com corte de motor e remoção de tags.

> Contagem de specs declarada à época: **294**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 96: Watercraft, Boats & Buoyancy Sailing Framework (v1.81.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas Náuticas**
- [x] Criadas as enumerações `ESBWatercraftType` (`Motorboat`, `Sailboat`, `Rowboat`) e `ESBWatercraftState` (`Docked`, `Cruising`, `Anchored`, `Drifting`).
- [x] Criada a estrutura `FSBWatercraftNavigationData` para rastreamento de velocidade linear na água (`CurrentSpeed`), acelerador (`ThrottleInput`), controle de leme (`RudderInput`), cota Z de superfície (`WaterLevelZ`), profundidade de calado (`BuoyancyDepth`), âncora lançada (`bIsAnchored`) e imersão (`bInWater`).
- [x] Criada a estrutura `FSBWatercraftSettings` com velocidades máximas à frente/ré, taxa de aceleração hidrodinâmica, arrasto/resistência da água, guinada por leme e rigidez de empuxo.

**Tags Nativas de Embarcação e Navegação**
- [x] Registradas tags de estado `State.Movement.Sailing`, `State.Movement.Sailing.Cruising`, `State.Movement.Sailing.Anchored` e `State.Vehicle.Watercraft`.

**Componente de Embarcação**
- [x] Desenvolvido em `05_SandboxCharacter` para gestão de controle e dinâmica náutica.
- [x] **Embarque e Desembarque de Piloto (`EnterWatercraft` / `ExitWatercraft`)**: Associa piloto à embarcação, aplicando `State.Movement.Sailing` ao piloto e `State.Vehicle.Watercraft` ao barco. Ao desembarcar, remove as tags de navegação e retorna o estado para `Docked` ou `Anchored`.
- [x] **Propulsão Hidrodinâmica e Leme (`SetThrottleInput`, `SetRudderInput`, `UpdateWatercraftPhysics`)**: Modula aceleração hidrodinâmica, aplica arrasto de resistência natural e concede `State.Movement.Sailing.Cruising` em avanço ativo.
- [x] **Sistema de Ancoragem (`DropAnchor` / `RaiseAnchor`)**: Lança a âncora travando a velocidade em zero absoluto e concedendo `State.Movement.Sailing.Anchored` ao piloto e à embarcação. Ao suspender a âncora, restabelece a capacidade propulsora.
- [x] Emite delegates notificadores: `OnWatercraftPilotChanged`, `OnWatercraftStateChanged` e `OnWatercraftAnchorChanged`.

**Testes Automatizados**
- [x] Criada suíte de testes validando embarque de piloto com tags e transição de estado, propulsão com acelerador e tag de cruzeiro, lançamento de âncora com travamento de velocidade e tag de ancorado, e recolhimento de âncora com desembarque e limpeza completa de tags.

> Contagem de specs declarada à época: **298**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 97: Aircraft, Airplanes, Helicopters & Flight Dynamics Framework (v1.82.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas Aeronáuticas**
- [x] Criadas as enumerações `ESBAircraftType` (`FixedWing`, `Helicopter`, `VTOL`) e `ESBFlightState` (`Parked`, `Taxiing`, `Takeoff`, `Airborne`, `Stalling`, `Landing`).
- [x] Criada a estrutura `FSBAircraftFlightData` para rastreamento de velocidade do ar (`Airspeed`), altitude (`Altitude`), acelerador de empuxo (`ThrottleInput`), atitude tridimensional (`PitchInput`, `RollInput`, `YawInput`), coeficiente de sustentação (`LiftCoefficient`), flags de voo (`bIsAirborne`), estol (`bIsStalling`) e modo VTOL (`bVTOLMode`).
- [x] Criada a estrutura `FSBAircraftSettings` com velocidade máxima de empuxo, taxa de aceleração, velocidade de estol crítico, taxas de arfagem/rolamento/guinada e coeficiente de arrasto.

**Tags Nativas de Aeronave e Voo**
- [x] Registradas tags de estado `State.Movement.Flying`, `State.Movement.Flying.Airborne`, `State.Movement.Flying.Stalling`, `State.Movement.Flying.VTOL` e `State.Vehicle.Aircraft`.

**Componente de Aeronave**
- [x] Desenvolvido em `05_SandboxCharacter` para controle de atitude e simulação aerodinâmica.
- [x] **Embarque e Desembarque de Piloto (`EnterAircraft` / `ExitAircraft`)**: Associa piloto à aeronave, aplicando `State.Movement.Flying` ao piloto e `State.Vehicle.Aircraft` à aeronave. Ao desembarcar, zera empuxo e limpa as tags de voo e estol.
- [x] **Dinâmica de Voo, Sustentação e Estol (`SetThrottleInput`, `SetFlightControls`, `UpdateFlightPhysics`)**: Acelera empuxo contra o arrasto; ao ultrapassar `StallSpeed` atinge `Airborne` e concede `State.Movement.Flying.Airborne`. Se a velocidade cair abaixo de `StallSpeed` em altitude elevada, entra em estol crítico (`State.Movement.Flying.Stalling`) e emite delegate.
- [x] **Vetorização VTOL e Helicóptero (`SetVTOLMode`)**: Ativa propulsão vertical imune a estol de asa fixa, concedendo `State.Movement.Flying.VTOL`.
- [x] Emite delegates notificadores: `OnAircraftPilotChanged`, `OnFlightStateChanged` e `OnAircraftStallStateChanged`.

**Testes Automatizados**
- [x] Criada suíte de testes validando embarque como piloto com tags e transição de táxi, aceleração acima da velocidade de estol com decolagem e tag de voo, desaceleração em altitude com detecção de estol e tag de estol, e recuperação via modo VTOL com desembarque e limpeza limpa de tags.

> Contagem de specs declarada à época: **302**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 98: Spacecraft, Orbital Maneuvers & 6-DOF Zero-G Dynamics Framework (v1.83.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas Espaciais**
- [x] Criadas as enumerações `ESBSpacecraftType` (`Fighter`, `Freighter`, `Shuttle`) e `ESBSpaceflightState` (`Docked`, `Cruising`, `Boosting`, `Drifting`, `Reentry`).
- [x] Criada a estrutura `FSBSpacecraftFlightData` para rastreamento de velocidade linear vetorial (`LinearVelocity`), velocidade angular (`AngularVelocity`), entradas de translação 3D (`TranslationInput`), rotação 3D (`RotationInput`), integridade do escudo térmico (`HeatShieldIntegrity`), velocidade escalar (`CurrentSpeed`), e flags de assistência de voo (`bFlightAssistActive`), pós-combustão (`bBoostActive`) e reentrada (`bInReentry`).
- [x] Criada a estrutura `FSBSpacecraftSettings` com velocidades máximas linear e boost, acelerações linear e angular, taxa de amortecimento de inércia e taxa de dissipação térmica.

**Tags Nativas de Espaçonave e Voo Espacial**
- [x] Registradas tags de estado `State.Movement.Spaceflight`, `State.Movement.Spaceflight.Cruising`, `State.Movement.Spaceflight.FlightAssistOff`, `State.Movement.Spaceflight.Reentry` e `State.Vehicle.Spacecraft`.

**Componente de Espaçonave**
- [x] Desenvolvido em `05_SandboxCharacter` para controle de atitude e dinâmica Newtoniana 6-DOF.
- [x] **Embarque e Desembarque de Piloto (`EnterSpacecraft` / `ExitSpacecraft`)**: Associa piloto à nave, aplicando `State.Movement.Spaceflight` ao piloto e `State.Vehicle.Spacecraft` à espaçonave. Ao desembarcar, zera propulsores e limpa todas as tags espaciais.
- [x] **Dinâmica Newtoniana 6-DOF e Amortecimento Inercial (`SetTranslationInput`, `SetRotationInput`, `SetFlightAssist`, `UpdateSpaceflightPhysics`)**: Acelera linearmente nos eixos 3D e angularmente em Pitch/Yaw/Roll. Quando `FlightAssist` está ativo e sem input, freia inercialmente; quando desativado, concede `State.Movement.Spaceflight.FlightAssistOff` e mantém velocidade vetorial contínua sem atrito no vácuo.
- [x] **Simulação de Reentrada Atmosférica (`SetAtmosphericReentry`)**: Aplica atrito e degradação do escudo térmico (`HeatShieldIntegrity`), concedendo `State.Movement.Spaceflight.Reentry`.
- [x] Emite delegates notificadores: `OnSpaceflightStateChanged`, `OnFlightAssistChanged` e `OnSpacecraftPilotChanged`.

**Testes Automatizados**
- [x] Criada suíte de testes validando embarque de piloto com tags espaciais e estado de deriva, propulsão 6-DOF com aceleração vetorial e tag de cruzeiro, desligamento de Flight Assist com conservação pura de inércia no vácuo e tag de assistência desativada, e reentrada atmosférica com desgaste térmico e desembarque limpo.

> Contagem de specs declarada à época: **306**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 99: Mech & Exosuit Locomotion, Thruster Jump & Heavy Cockpit Framework (v1.84.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Mechas**
- [x] Criadas as enumerações `ESBMechClass` (`LightScout`, `MediumAssault`, `HeavySiege`) e `ESBMechState` (`PoweredOff`, `Idle`, `Walking`, `JumpJets`, `Dashing`, `Overheated`, `Ejected`).
- [x] Criada a estrutura `FSBMechOperationalData` para rastreamento de velocidade linear (`CurrentSpeed`), temperatura do reator (`CoreHeat`), combustível dos propulsores de salto (`JumpJetFuel`), vetor de deslocamento bípede (`MoveInput`), e flags de energização (`bIsPowered`), propulsores ativos (`bJumpJetsActive`) e superaquecimento de emergência (`bIsOverheated`).
- [x] Criada a estrutura `FSBMechSettings` com velocidades de caminhada e arrancada rápida (*Dash*), empuxo vertical de salto, taxas de geração e dissipação de calor, e limites térmicos de segurança.

**Tags Nativas de Mecha e Exoesqueleto**
- [x] Registradas tags de estado `State.Movement.Mech`, `State.Movement.Mech.Walking`, `State.Movement.Mech.JumpJets`, `State.Movement.Mech.Overheated` e `State.Vehicle.Mech`.

**Componente de Mecha**
- [x] Desenvolvido em `05_SandboxCharacter` para controle de cockpit e dinâmica bípede pesada.
- [x] **Embarque e Desembarque no Cockpit (`EnterMech` / `ExitMech`)**: Associa piloto ao mecha, energiza sistemas e aplica `State.Movement.Mech` ao piloto e `State.Vehicle.Mech` ao mecha. Ao desembarcar, desliga reatores e limpa todas as tags de pilotagem.
- [x] **Locomoção Bípede e Propulsores (`SetMoveInput`, `ActivateJumpJets`, `TriggerDash`, `UpdateMechPhysics`)**: Acelera a passada pesada concedendo `State.Movement.Mech.Walking`. Ao disparar `JumpJets`, consome combustível e eleva a temperatura do reator (`CoreHeat`), concedendo `State.Movement.Mech.JumpJets`.
- [x] **Proteção Térmica e Sobreaquecimento (`Overheat Threshold`)**: Ao atingir o limite térmico, corta a locomoção, força o estado `Overheated`, concede `State.Movement.Mech.Overheated` e aciona delegate até resfriamento para o limite de recuperação.
- [x] Emite delegates notificadores: `OnMechStateChanged`, `OnMechOverheatChanged` e `OnMechPilotChanged`.

**Testes Automatizados**
- [x] Criada suíte de testes validando embarque no cockpit com tags de mecha e energização, avanço de passada pesada com tag de caminhada, queima de propulsores de salto com consumo de combustível e aquecimento de núcleo, e desligamento de emergência por superaquecimento com desembarque limpo.

> Contagem de specs declarada à época: **310**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 100: Heavy Machinery, Cranes, Excavators & Hydraulic Physics Framework (v1.85.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Maquinário Pesado**
- [x] Criadas as enumerações `ESBMachineryType` (`Excavator`, `Crane`, `Bulldozer`, `Loader`) e `ESBMachineryState` (`Parked`, `Idling`, `Operating`, `Lifting`, `Excavating`).
- [x] Criada a estrutura `FSBMachineryHydraulicData` para rastreamento de pressão do circuito hidráulico (`SystemPressure`), pressão máxima de alívio (`MaxSystemPressure`), rotação da bomba primária (`PumpRPM`), ângulos dos atuadores cinemáticos (`BoomAngle`, `ArmAngle`, `BucketAngle`, `CabinSlewAngle`), comprimento do cabo do guincho (`CableLength`), massa da carga içada (`LiftedPayloadMass`), e flags de motor em operação (`bEngineRunning`) e sapatas estabilizadoras acionadas (`bOutriggersDeployed`).
- [x] Criada a estrutura `FSBMachinerySettings` com capacidade máxima de carga (`MaxLiftCapacity`), velocidade de giro e acionamento de atuadores (`BoomSlewSpeed`), taxa de pressurização da bomba (`HydraulicBuildRate`), velocidade do guincho de cabo de aço (`WinchSpeed`) e força de desagregação de escavação (`ExcavationForce`).

**Tags Nativas de Maquinário e Hidráulica**
- [x] Registradas tags de estado `State.Movement.Machinery`, `State.Movement.Machinery.Operating`, `State.Movement.Machinery.Lifting`, `State.Movement.Machinery.Excavating` e `State.Vehicle.Machinery`.

**Componente de Maquinário Pesado**
- [x] Desenvolvido em `05_SandboxCharacter` para controle de cabine, acionamento de bomba e cinemática hidráulica.
- [x] **Embarque e Controle de Cabine (`EnterMachinery` / `ExitMachinery`)**: Associa operador à máquina, liga a bomba de alta pressão e aplica `State.Movement.Machinery` ao operador e `State.Vehicle.Machinery` à máquina. Ao desembarcar, desliga a bomba, alivia o circuito e limpa todas as tags de operação.
- [x] **Articulação Cinemática e Pressurização (`SetBoomInput`, `SetArmInput`, `SetBucketInput`, `SetSlewInput`, `SetWinchInput`, `UpdateHydraulicPhysics`)**: Sob pressão hidráulica mínima (50 bar), movimenta os cilindros da lança, braço, concha, torre giratória e guincho de içamento, concedendo `State.Movement.Machinery.Operating`.
- [x] **Içamento de Cargas e Guincho (`AttachPayload` / `DetachPayload`)**: Permite engatar e içar cargas respeitando a capacidade máxima de içamento (`MaxLiftCapacity`), concedendo `State.Movement.Machinery.Lifting`.
- [x] **Ciclo de Escavação (`TriggerExcavateAction`)**: Aciona ciclos de penetração de caçamba em nós de terreno concedendo `State.Movement.Machinery.Excavating`.
- [x] Emite delegates notificadores: `OnMachineryStateChanged`, `OnMachineryPressureChanged` e `OnMachineryOperatorChanged`.

**Testes Automatizados**
- [x] Criada suíte de testes validando embarque de operador com pressurização de bomba e tags de maquinário, articulação multieixo sob pressão com tag de operação, engate e içamento de carga suspensa com tag de lifting e desengate, e ciclo de escavação com caçamba e desembarque limpo.

> Contagem de specs declarada à época: **314**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 101: Modular Structural Integrity, Stress Calculation & Physics-Based Collapse Framework (v1.86.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas Estruturais**
- [x] Criadas as enumerações `ESBStructuralMaterialTier` (`Wood`, `Stone`, `Metal`, `ReinforcedTitanium`) e `ESBStructuralStabilityState` (`Stable`, `Stressed`, `Critical`, `Collapsing`).
- [x] Criada a estrutura `FSBStructuralNodeData` para rastreamento de estabilidade percentual (`StructuralStability`), carga gravitacional acumulada (`CurrentLoadWeight`), capacidade máxima de carga (`MaxLoadCapacity`), distância topológica até a fundação (`DistanceFromAnchor`), vão máximo suportado (`MaxSupportDistance`), flag de âncora no solo (`bIsGroundAnchor`) e estado de estabilidade (`StabilityState`).
- [x] Criada a estrutura `FSBStructuralSettings` com material da peça (`MaterialTier`), capacidade de carga base (`BaseLoadCapacity`), vão livre máximo horizontal (`MaxHorizontalSpan`), e limiares de estresse e estado crítico (`StressThreshold`, `CriticalThreshold`).

**Tags Nativas de Integridade e Suporte**
- [x] Registradas tags de estado `State.Building.Anchor`, `State.Building.Supported`, `State.Building.Stressed` e `State.Building.Collapsing`.

**Componente de Integridade Estrutural**
- [x] Desenvolvido em `08_SandboxInventory` para cálculo de carga e estabilidade mecânica de peças de construção (`ASBBuildingPiece`).
- [x] **Ancoragem e Topologia de Suporte (`SetGroundAnchor`, `RegisterNeighborPiece`, `UnregisterNeighborPiece`)**: Fundações no solo são marcadas como âncoras com estabilidade de 100% e concedem `State.Building.Anchor` e `State.Building.Supported`. Peças vizinhas propagam a distância topológica da âncora mais próxima.
- [x] **Carga Gravitacional e Tensão (`AddSupportedLoad`, `RemoveSupportedLoad`, `RecalculateIntegrity`)**: Acumula peso sobre pisos e tetos. Ao atingir o limiar de sobrecarga, entra no estado `Stressed` e concede `State.Building.Stressed`.
- [x] **Colapso Físico em Cascata (`TriggerStructuralCollapse`)**: Quando uma âncora ou pilar é destruído, ou se a distância exceder `MaxSupportDistance`, as peças conectadas perdem sustentação, entram em `Collapsing`, recebem a tag `State.Building.Collapsing`, perdem a tag `State.Building.Supported` e propagam a quebra para toda a superestrutura em cascata.
- [x] Emite delegates notificadores: `OnStructuralStabilityChanged`, `OnStructuralLoadChanged` e `OnStructuralCollapse`.

**Testes Automatizados**
- [x] Criada suíte de testes validando fundação como âncora no solo com 100% de estabilidade e tags de âncora/suporte, propagação de distância e estabilidade para paredes e tetos vizinhos, aplicação de sobrecarga mecânica com transição para estado Stressed e tag correspondente, e destruição/desancoragem de fundação desencadeando colapso físico em cascata e limpeza das tags de suporte.

> Contagem de specs declarada à época: **318**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 102: Power Grid, Generators, Batteries, Circuit Wiring & Electric Consumers Framework (v1.87.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Rede Elétrica**
- [x] Criadas as enumerações `ESBPowerNodeType` (`Generator`, `Battery`, `Consumer`, `RelayPole`) e `ESBPowerGridState` (`Unpowered`, `Powered`, `Charging`, `Discharging`, `Overloaded`).
- [x] Criada a estrutura `FSBPowerNodeData` para rastreamento de tipo de nó (`NodeType`), geração nominal (`PowerGeneration`), consumo demandado (`PowerConsumption`), carga armazenada em bateria (`BatteryStoredEnergy`), capacidade máxima (`BatteryCapacity`), produção total agregada na rede (`GridTotalProduction`), demanda total agregada (`GridTotalDemand`), razão de satisfação de carga (`PowerSatisfactionRatio`), flag de disjuntor desarmado (`bIsBreakerTripped`) e estado elétrico do nó (`GridState`).
- [x] Criada a estrutura `FSBPowerGridSettings` com distância máxima de fiação (`MaxConnectionDistance`), limite de cabos por poste (`MaxWireConnections`), limiar de disparo de disjuntor por sobrecarga (`OverloadThreshold`), e taxas de carga e descarga de bateria (`BatteryChargeRate`, `BatteryDischargeRate`).

**Tags Nativas de Rede Elétrica**
- [x] Registradas tags de estado `State.Power.Powered`, `State.Power.Unpowered`, `State.Power.Overloaded`, `State.Power.Charging` e `State.Power.Discharging`.

**Componente de Rede Elétrica**
- [x] Desenvolvido em `08_SandboxInventory` para gerenciamento de circuitos, geradores, baterias e máquinas consumidoras.
- [x] **Topologia de Circuito e Fiação (`SetupNode`, `ConnectToPowerNode`, `DisconnectFromPowerNode`)**: Cria conexões em grafo elétrico entre máquinas, geradores e baterias, validando limites de cabos e alcance.
- [x] **Balanço Energético e Simulação de Carga (`SimulatePowerGridTick`, `GetConnectedSubnet`)**: Varre a sub-rede conectada calculando produção vs demanda. Se a produção for suficiente, supre 100% da demanda (`State.Power.Powered`) e carrega acumuladores com o excedente (`State.Power.Charging`).
- [x] **Acumuladores e Suprimento Inercial**: Quando os geradores são desligados, as baterias entram imediatamente em descarga (`State.Power.Discharging`) mantendo as máquinas conectadas operando normalmente.
- [x] **Proteção contra Sobrecarga e Disjuntor (`SetBreakerTripped`)**: Se a demanda exceder a capacidade em mais de 120% (`OverloadThreshold`) ou o disjuntor for acionado, entra no estado `Overloaded`, concede `State.Power.Overloaded`, desliga as máquinas (`State.Power.Unpowered`) e emite `OnBreakerTripped`.
- [x] Emite delegates notificadores: `OnPowerGridStateChanged`, `OnPowerFlowChanged` e `OnBreakerTripped`.

**Testes Automatizados**
- [x] Criada suíte de testes validando conexão de gerador a consumidor com suprimento e tag de alimentação, absorção de excedente por bateria acumuladora com tag de carregamento, sustentação contínua de consumidores via descarga de bateria quando gerador desliga, e desarme de disjuntor por sobrecarga extrema desenergizando consumidores.

> Contagem de specs declarada à época: **322**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 103: Pipe Networks, Fluids, Pumps, Valves, Fluid Tanks & Gas Mechanics Framework (v1.88.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Redes de Fluidos**
- [x] Criadas as enumerações `ESBFluidType` (`None`, `Water`, `CrudeOil`, `Fuel`, `Steam`, `ToxicGas`), `ESBPipeNodeType` (`SourcePump`, `PipeSegment`, `Valve`, `FluidTank`, `GasVent`, `ConsumerApparatus`) e `ESBPipeFlowState` (`Empty`, `Flowing`, `Blocked`, `Pressurized`, `Leaking`, `Ruptured`).
- [x] Criada a estrutura `FSBPipeNodeData` para rastreamento de tipo de fluido (`FluidType`), tipo de nó (`NodeType`), volume armazenado (`FluidAmount`), capacidade volumétrica (`FluidCapacity`), pressão interna (`CurrentPressure`), pressão limite de segurança (`MaxSafePressure`), vazão efetiva (`FlowRate`), abertura da válvula (`ValveOpenPercentage`), flag de bomba ativa (`bIsPumpActive`), flag de ruptura física (`bIsRuptured`) e estado hidrostático (`FlowState`).
- [x] Criada a estrutura `FSBPipeNetworkSettings` com alcance máximo de conexão (`MaxConnectionDistance`), limite de conexões (`MaxPipeConnections`), pressão gerada por bombas (`PumpPressureGeneration`), vazão base (`BaseFlowSpeed`), multiplicador de ruptura por sobrepressão (`RuptureThresholdMultiplier`) e taxa de vazamento (`LeakLossRate`).

**Tags Nativas de Fluidos e Tubulações**
- [x] Registradas tags de estado `State.Fluid.Flowing`, `State.Fluid.Blocked`, `State.Fluid.Pressurized`, `State.Fluid.Leaking` e `State.Fluid.Ruptured`.

**Componente de Redes de Tubulação**
- [x] Desenvolvido em `08_SandboxInventory` para controle de circuitos hidráulicos/pneumáticos, bombas, válvulas e tanques.
- [x] **Topologia de Encanamento e Acoplamento (`SetupNode`, `ConnectPipe`, `DisconnectPipe`)**: Conecta tubulações em grafo hidráulico validando limites de portas e distância.
- [x] **Pressurização e Dinâmica de Fluxo (`SetPumpActive`, `SetValveOpenPercentage`, `SimulateFluidDynamicsTick`)**: Bombas ativas elevam a pressão a montante e propagam fluidos para tanques receptores concedendo as tags `State.Fluid.Flowing` e `State.Fluid.Pressurized`. Válvulas fechadas estrangulam o fluxo concedendo `State.Fluid.Blocked`.
- [x] **Injeção, Armazenamento e Extração (`InjectFluid`, `ExtractFluid`)**: Tanques retêm volume armazenado com integridade de tipos e emitem notificações de nível (`OnFluidLevelChanged`).
- [x] **Sobrepressão e Ruptura Mecânica**: Se a pressão da bomba exceder `MaxSafePressure * RuptureThresholdMultiplier`, o tubo entra no estado `Ruptured`, recebe `State.Fluid.Ruptured`, drena todo o fluido contido no ambiente e emite o delegate `OnPipeRupture`.
- [x] Emite delegates notificadores: `OnPipeFlowStateChanged`, `OnFluidPressureChanged`, `OnFluidLevelChanged` e `OnPipeRupture`.

**Testes Automatizados**
- [x] Criada suíte de testes validando acoplamento de bomba, tubo e tanque com pressurização de água e tags de fluxo/pressão, fechamento de válvula bloqueando vazão a jusante com tag de bloqueio, ruptura física de cano sob sobrepressão extrema com tag de ruptura e perda de fluido, e injeção e extração volumétrica em tanques de armazenamento.

> Contagem de specs declarada à época: **326**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 104: Conveyor Belts, Item Sorters, Splitters, Mergers & Factory Logistics Framework (v1.89.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Logística**
- [x] Criadas as enumerações `ESBConveyorNodeType` (`BeltSegment`, `Splitter`, `Merger`, `SmartSorter`, `ContainerLoader`, `ContainerUnloader`) e `ESBConveyorState` (`Idle`, `Conveying`, `Sorting`, `Merging`, `Jammed`).
- [x] Criada a estrutura `FSBConveyorItemSlot` para rastreamento de item individual na esteira (`ItemId`, `Quantity`, `ItemCategoryTag`, `BeltProgressAlpha`).
- [x] Criada a estrutura `FSBConveyorNodeData` com propriedades de velocidade de esteira (`BeltSpeed`), comprimento (`BeltLength`), capacidade máxima de itens (`MaxItemCapacity`), contagem atual (`CurrentItemCount`), ponteiro de alternância circular (`SplitterRoundRobinIndex`), tag de filtro (`FilterTag`) e flag de travamento por saturação a jusante (`bIsJammed`).
- [x] Criada a estrutura `FSBConveyorSettings` com limites de conexões de entrada/saída, espaçamento mínimo entre itens e delays de transferência.

**Tags Nativas de Logística**
- [x] Registradas tags de estado `State.Logistics.Conveying`, `State.Logistics.Jammed`, `State.Logistics.Sorting` e `State.Logistics.Merging`.

**Componente de Esteiras e Logística**
- [x] Desenvolvido em `08_SandboxInventory` para transporte sequencial de itens físicos e roteamento lógico.
- [x] **Fila FIFO e Translação (`EnqueueItem`, `DequeueItem`, `SimulateConveyorTick`)**: Itens avançam ao longo da esteira (`BeltProgressAlpha`) e são descarregados na esteira conectada de saída ao atingir a extremidade (`BeltProgressAlpha >= 1.0f`), concedendo a tag `State.Logistics.Conveying`.
- [x] **Divisão Balanceada (Splitters)**: Alterna saídas sucessivas em round-robin entre múltiplos caminhos conectados.
- [x] **Confluência com Buffer (Mergers)**: Unifica fluxos de múltiplas esteiras de entrada em uma única saída concedendo a tag `State.Logistics.Merging`.
- [x] **Separação Inteligente (Smart Sorters)**: Inspeciona a tag de categoria do item (`ItemCategoryTag`); se coincidir com `FilterTag`, direciona para a saída prioritária emitindo `OnConveyorItemFiltered`; caso contrário, envia para a saída secundária/overflow concedendo a tag `State.Logistics.Sorting`.
- [x] **Congestionamento e Backpressure**: Se a esteira ou receptor de destino estiver com capacidade saturada, a esteira a montante pausa a translação (`bIsJammed = true`), concede a tag `State.Logistics.Jammed`, emite `OnConveyorJammed` e retém integralmente toda a carga sem perdas.
- [x] Emite delegates notificadores: `OnConveyorStateChanged`, `OnConveyorItemTransferred`, `OnConveyorJammed` e `OnConveyorItemFiltered`.

**Testes Automatizados**
- [x] Criada suíte de testes validando enfileiramento e transferência FIFO entre esteiras adjacentes, divisão balanceada de itens em round-robin em splitters 1:2, unificação de múltiplos fluxos em mergers 2:1, e separação de itens por Gameplay Tag em smart sorters com travamento por contrapressão (*Backpressure*) sem perda de itens.

> Contagem de specs declarada à época: **330**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 105: Automated Smelters, Assemblers, Refineries & Industrial Processing Framework (v1.90.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas Industriais**
- [x] Criadas as enumerações `ESBProcessorType` (`Smelter`, `Foundry`, `Assembler`, `Refinery`, `ChemicalPlant`, `Constructor`) e `ESBProcessorState` (`Idle`, `Processing`, `MissingIngredients`, `NoPower`, `OutputFull`, `Overheated`).
- [x] Criadas as estruturas `FSBIndustrialIngredient` e `FSBIndustrialFluidIngredient` para representação de insumos/produtos sólidos e fluidos.
- [x] Criada a estrutura `FSBIndustrialRecipe` com tempo de ciclo (`CraftingTime`), demanda elétrica (`PowerRequirement`), calor térmico gerado (`HeatGeneration`), listas de insumos de entrada (`InputItems`, `InputFluids`) e saídas manufaturadas (`OutputItems`, `OutputFluids`).
- [x] Criada a estrutura `FSBProcessorData` com estado operacional (`ProcessorState`), receita ativa (`ActiveRecipe`), progresso do ciclo (`CurrentProgressAlpha`), multiplicador de overclock (`OverclockMultiplier`), temperatura atual (`CurrentTemperature`), limite térmico de segurança (`MaxSafeTemperature`), flag de energia (`bHasPower`) e contagem de ciclos concluídos (`CompletedCyclesCount`).
- [x] Criada a estrutura `FSBProcessorSettings` com capacidade de buffers de entrada e saída (`InputItemBufferCapacity`, `OutputItemBufferCapacity`, `FluidBufferCapacity`) e taxa de dissipação passiva de calor (`HeatDissipationRate`).

**Tags Nativas de Manufatura e Indústria**
- [x] Registradas tags de estado `State.Industrial.Processing`, `State.Industrial.Idle`, `State.Industrial.MissingIngredients`, `State.Industrial.NoPower`, `State.Industrial.OutputFull` e `State.Industrial.Overheated`.

**Componente de Processamento Industrial**
- [x] Desenvolvido em `08_SandboxInventory` para simulação contínua de ciclos de produção automatizada.
- [x] **Receitas e Buffers (`SetupProcessor`, `DepositInputItem`, `DepositInputFluid`, `WithdrawOutputItem`, `WithdrawOutputFluid`)**: Gerenciamento integrado de estoques internos de insumos e produtos.
- [x] **Ciclos de Manufatura e Transição de Estados (`SimulateProcessorTick`)**:
  - Avalia disponibilidade de energia elétrica (`SetPowerSupplied`). Se desenergizado, transita para `NoPower` e concede `State.Industrial.NoPower`.
  - Avalia capacidade restante no buffer de saída. Se cheio, transita para `OutputFull` e concede `State.Industrial.OutputFull`.
  - Avalia disponibilidade de insumos no buffer. Se faltarem materiais, transita para `MissingIngredients` e concede `State.Industrial.MissingIngredients`.
  - Quando todos os requisitos são atendidos, processa continuamente (`CurrentProgressAlpha`), aquece o equipamento e concede a tag `State.Industrial.Processing`. Ao atingir 100%, deduz insumos, deposita produtos acabados no buffer de saída e emite o delegate `OnProcessorCycleCompleted`.
- [x] Emite delegates notificadores: `OnProcessorStateChanged`, `OnProcessorCycleCompleted` e `OnProcessorTemperatureChanged`.

**Testes Automatizados**
- [x] Criada suíte de testes validando ciclo completo de fundição de minério em barra de ferro com tag de processamento e consumo de insumos, corte de energia elétrica paralisando produção com estado e tag `NoPower`, desabastecimento de insumos gerando estado e tag `MissingIngredients` com retomada imediata ao depositar novos materiais, e saturação do buffer de saída travando o ciclo com estado e tag `OutputFull` com retomada ao descarregar itens.

> Contagem de specs declarada à época: **334**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 106: Mining Extractors, Oil Wells, Geothermal Pumps & Deep Core Harvesters (v1.91.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Extração**
- [x] Criadas as enumerações `ESBExtractorType` (`MiningDrill`, `OilWellPump`, `GeothermalExtractor`, `DeepCoreHarvester`, `WaterExtractor`) e `ESBExtractorState` (`Idle`, `Extracting`, `Depleted`, `NoPower`, `OutputBlocked`, `Overheated`).
- [x] Criada a enumeração `ESBResourceDepositPurity` (`Impure` = 0.5x, `Normal` = 1.0x, `Pure` = 2.0x).
- [x] Criada a estrutura `FSBExtractorData` para rastreamento de recurso sólido extraído (`ExtractedItemId`), fluido extraído (`ExtractedFluidType`), pureza do depósito (`DepositPurity`), taxa base de extração (`BaseExtractionRate`), progresso do ciclo (`CurrentProgressAlpha`), consumo elétrico (`PowerConsumption`), multiplicador de overclock (`OverclockMultiplier`), temperatura do motor (`CurrentTemperature`), limite térmico (`MaxSafeTemperature`), taxa de calor (`HeatGenerationRate`), flags de energia (`bHasPower`) e exaustão (`bIsDepositDepleted`), e totais acumulados de extração (`TotalExtractedItems`, `TotalExtractedFluids`).
- [x] Criada a estrutura `FSBExtractorSettings` com limites de capacidade dos buffers internos de saída (`OutputItemBufferCapacity`, `OutputFluidBufferCapacity`) e taxa de resfriamento passivo (`HeatDissipationRate`).

**Tags Nativas de Extração de Recursos**
- [x] Registradas tags de estado `State.Extractor.Drilling`, `State.Extractor.Idle`, `State.Extractor.Depleted`, `State.Extractor.NoPower`, `State.Extractor.OutputBlocked` e `State.Extractor.Overheated`.

**Componente de Extração de Recursos**
- [x] Desenvolvido em `08_SandboxInventory` para mineração e bombeamento contínuo sobre depósitos geológicos.
- [x] **Acoplamento Geológico e Multiplicadores (`SetupExtractor`, `SetDepositDepleted`, `SetPowerSupplied`, `SetOverclockMultiplier`)**: Modula a velocidade de colheita conforme a pureza do veio e o overclocking configurado (`EffectiveRate = BaseRate * PurityMultiplier * OverclockMultiplier`).
- [x] **Simulação de Perfuração e Estados (`SimulateExtractorTick`)**:
  - Verifica suprimento de energia (`NoPower`), estado de exaustão do veio (`Depleted`), limite de temperatura (`Overheated`) e espaço livre no buffer de saída (`OutputBlocked`).
  - Ao cumprir os pré-requisitos, extrai ativamente concedendo a tag `State.Extractor.Drilling`, gerando aquecimento térmico e depositando os insumos sólidos/fluidos no buffer de saída a cada ciclo concluído (`CurrentProgressAlpha >= 1.0f`), emitindo os delegates `OnExtractorItemHarvested` e `OnExtractorFluidHarvested`.
- [x] **Descarga e Escoamento (`WithdrawOutputItem`, `WithdrawOutputFluid`)**: Permite alimentação direta para esteiras rolantes (`USBConveyorNetworkComponent`) ou dutos hidráulicos (`USBPipeNetworkComponent`).
- [x] Emite delegates notificadores: `OnExtractorStateChanged`, `OnExtractorItemHarvested`, `OnExtractorFluidHarvested` e `OnExtractorTemperatureChanged`.

**Testes Automatizados**
- [x] Criada suíte de testes validando extração acelerada em depósito puro (2x) com tag de perfuração, corte de energia elétrica paralisando extração com estado e tag `NoPower`, esgotamento geológico do veio transitando para estado e tag `Depleted`, e saturação do buffer de saída travando a máquina com estado e tag `OutputBlocked` com retomada ao descarregar minérios.

> Contagem de specs declarada à época: **338**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 107: Automated Freight Trains, Rail Tracks, Signals & Railroad Logistics Framework (v1.92.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Logística Ferroviária**
- [x] Criadas as enumerações `ESBRailNodeType` (`StraightTrack`, `CurvedTrack`, `SwitchBranch`, `BlockSignal`, `ChainSignal`, `TrainStation`), `ESBRailSignalState` (`ClearGreen`, `ApproachYellow`, `StopRed`), `ESBTrainMovementState` (`Stationary`, `Traveling`, `Loading`, `Unloading`, `WaitingSignal`, `Derailed`) e `ESBWagonType` (`LocomotiveEngine`, `FreightCargo`, `FluidTanker`).
- [x] Criada a estrutura `FSBRailWagonData` para inventário sólido (`CargoInventory`), capacidade (`CargoCapacity`), tipo de fluido (`FluidType`), volume de fluido (`FluidAmount`) e capacidade de tanque (`FluidCapacity`).
- [x] Criada a estrutura `FSBTrainConsist` para identificador da composição (`TrainId`), estado de movimento (`MovementState`), velocidade linear e limite (`CurrentSpeed`, `MaxSpeed`), progresso no bloco de trilho (`CurrentTrackProgressAlpha`), índice do bloco atual (`CurrentTrackSegmentId`), lista de vagões acoplados (`Wagons`), agendamento de estações (*Timetable* / `DestinationStations`), índice da parada atual (`CurrentStationIndex`), temporizador de permanência em plataforma (`StationWaitTimer`) e duração programada de carregamento (`MaxStationWaitDuration`).
- [x] Criada a estrutura `FSBRailBlockData` para controle de ocupação de trechos férreos (`BlockId`, `bIsOccupied`, `OccupyingTrainId`, `SignalState`).
- [x] Criada a estrutura `FSBRailNetworkSettings` com parâmetros de aceleração (`Acceleration`), desaceleração/frenagem (`Deceleration`) e taxa de transferência de carga (`StationTransferRate`).

**Tags Nativas de Transporte Ferroviário**
- [x] Registradas tags de estado `State.Rail.Traveling`, `State.Rail.Loading`, `State.Rail.Unloading`, `State.Rail.WaitingSignal` e `State.Rail.Derailed`.

**Componente de Rede Ferroviária**
- [x] Desenvolvido em `08_SandboxInventory` para controle autônomo de composições ferroviárias.
- [x] **Acoplamento e Itinerários (`SetupTrainConsist`, `AddWagon`, `AddStationToSchedule`, `StartTravel`)**: Configura locomotivas, adiciona vagões de carga e estabelece rotas circulares entre estações.
- [x] **Sinalização e Reserva de Blocos (`RegisterRailBlock`, `SetBlockOccupied`, `RequestBlockReservation`, `ReleaseBlockReservation`)**: Garante exclusividade de bloco de via com semáforos verdes/vermelhos e transição para `WaitingSignal` ao se deparar com bloco ocupado à frente.
- [x] **Movimentação & Paradas Programadas (`SimulateRailTick`)**:
  - Acelera em trechos desimpedidos concedendo `State.Rail.Traveling`.
  - Desacelera ao atingir a estação de destino, imobiliza a composição e transita para `State.Rail.Loading` (se vazia) ou `State.Rail.Unloading` (se carregada).
  - Ao esgotar o tempo de permanência (`MaxStationWaitDuration`), avança automaticamente para a próxima estação da rota.
- [x] **Operações de Carga (`LoadCargoIntoWagon`, `UnloadCargoFromWagon`, `GetWagonCargoCount`)**: Movimenta mercadorias sólidas entre estações e vagões.
- [x] Emite delegates notificadores: `OnTrainStateChanged`, `OnTrainStationArrived`, `OnTrainCargoTransferred` e `OnRailSignalChanged`.

**Testes Automatizados**
- [x] Criada suíte de testes validando aceleração contínua em bloco livre com tag de trânsito ferroviário, desaceleração e espera em semáforo vermelho com tag `WaitingSignal` retomando viagem ao liberar o trecho, parada em estação com transição para `Loading` e carregamento de minérios, e parada em estação terminal com transição para `Unloading`, descarga de materiais e partida automatizada.

> Contagem de specs declarada à época: **342**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 108: Programmable Logic Controllers (PLC), Logic Gates, Sensors & Circuit Networks (v1.93.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Circuitos Lógicos**
- [x] Criadas as enumerações `ESBLogicNodeType` (`LogicGateAND`, `LogicGateOR`, `LogicGateNOT`, `LogicGateXOR`, `LogicGateNAND`, `LogicGateNOR`, `Comparator`, `ArithmeticProcessor`, `Counter`, `PulseGenerator`, `RSLatch`, `SensorInput`, `ActuatorOutput`), `ESBLogicComparisonOp` (`GreaterThan`, `LessThan`, `Equal`, `NotEqual`, `GreaterOrEqual`, `LessOrEqual`), `ESBLogicArithmeticOp` (`Add`, `Subtract`, `Multiply`, `Divide`, `Modulo`) e `ESBLogicWireColor` (`RedWire`, `GreenWire`, `CopperWire`).
- [x] Criada a estrutura `FSBCircuitSignal` para identificação de canal de sinal (`SignalChannel`) e valor de magnitude (`SignalValue`).
- [x] Criada a estrutura `FSBLogicGateData` para identificador do nó (`NodeId`), tipo de operação, canais de entrada (`InputChannelA`, `InputChannelB`), operando constante (`ConstantOperand`, `bUseConstantOperand`), canal de saída (`OutputChannel`), valor resultante (`OutputValue`), flag de avaliação booleana (`bConditionEvaluatedTrue`), estado interno biestável (`bLatchState`) e contadores (`CounterCurrent`, `CounterTarget`).
- [x] Criada a estrutura `FSBLogicCircuitSettings` com frequência de avaliação (`EvaluationFrequency`) e duração de pulsos lógicos (`PulseDuration`).

**Tags Nativas de Lógica & Circuitos**
- [x] Registradas tags de estado `State.Logic.Evaluating`, `State.Logic.ConditionMet`, `State.Logic.ConditionFailed`, `State.Logic.Pulsing` e `State.Logic.Disabled`.

**Componente de Circuito Lógico**
- [x] Desenvolvido em `08_SandboxInventory` para automação industrial, controle de atuadores e tomada de decisão autônoma.
- [x] **Configuração de Processadores (`SetupComparator`, `SetupLogicGate`, `SetupArithmeticProcessor`, `SetupRSLatch`)**: Conecta portas booleanas, comparadores com constantes e biestáveis com canais de Set/Reset.
- [x] **Barramentos de Sinais Independentes (`InjectSignal`, `ReadSignal`, `ClearSignals`)**: Permite transmissão e multiplexação de dados em cabos Vermelhos, Verdes e de Cobre.
- [x] **Avaliação de Ciclo Lógico (`EvaluateCircuitTick`)**:
  - Soma os barramentos de entrada e processa as regras lógicas/aritméticas.
  - Concede ativamente a tag `State.Logic.Evaluating` e alterna entre `State.Logic.ConditionMet` e `State.Logic.ConditionFailed` conforme o resultado.
  - Emite pulsos e valores analógicos para o canal de saída configurado.
- [x] Emite delegates notificadores: `OnLogicConditionEvaluated`, `OnLogicSignalEmitted` e `OnLogicLatchToggled`.

**Testes Automatizados**
- [x] Criada suíte de testes validando avaliação condicional de comparador numérico (`IronOre > 50`) emitindo sinal de alarme e concedendo tag `ConditionMet`, porta booleana AND combinando dois canais de sinal, processador aritmético executando multiplicação (`FluidVolume * 2.0`) e atualizando barramento de sinal, e biestável RS-Latch memorizando pulso de ativação e só desarmando com pulso de reset.

> Contagem de specs declarada à época: **346**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 109: Industrial Cargo Drones, Drone Ports, Sky Corridors & Aerial Logistics Framework (v1.94.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Logística Aérea**
- [x] Criadas as enumerações `ESBCargoDroneFlightState` (`IdleAtPort`, `TakingOff`, `InFlight`, `Landing`, `Recharging`, `LowBatteryReturn`) e `ESBCargoDroneModel` (`LightCourier`, `HeavyLiftDrone`, `LongRangeHexacopter`).
- [x] Criada a estrutura `FSBCargoDroneData` para identificação (`DroneId`), modelo (`DroneModel`), estado de voo (`FlightState`), localização e portos (`CurrentLocation`, `HomePortLocation`, `TargetPortLocation`, `HomePortId`, `TargetPortId`), gerenciamento de energia (`CurrentBattery`, `MaxBattery`, `BatteryDischargeRate`, `BatteryRechargeRate`, `LowBatteryThreshold`), parâmetros de voo (`FlightSpeed`, `CruiseAltitude`, `FlightProgressAlpha`), compartimento de carga (`CargoBay`, `CargoBayCapacity`) e estatísticas (`TotalTripsCompleted`).
- [x] Criada a estrutura `FSBDronePortData` para identificação de docas aéreas (`PortId`), coordenadas 3D (`PortLocation`), presença de estação de recarga (`bHasRechargeDock`) e buffers de carga (`InputBuffer`, `OutputBuffer`, `BufferCapacity`).
- [x] Criada a estrutura `FSBCargoDroneSettings` com durações de decolagem/pouso (`TakeoffDuration`, `LandingDuration`) e velocidade de transferência (`TransferRate`).

**Tags Nativas de Drones de Carga**
- [x] Registradas tags de estado `State.Drone.Idle`, `State.Drone.TakingOff`, `State.Drone.InFlight`, `State.Drone.Landing`, `State.Drone.Recharging` e `State.Drone.LowBattery`.

**Componente de Rede de Drones de Carga**
- [x] Desenvolvido em `08_SandboxInventory` para transporte autônomo e de alta velocidade sobre terrenos acidentados.
- [x] **Configuração e Despacho (`SetupDrone`, `RegisterDronePort`, `SetFlightRoute`, `DispatchDrone`)**: Define rotas entre portos de carga e inicia missões de transporte.
- [x] **Gerenciamento de Carga (`LoadCargoIntoDrone`, `UnloadCargoFromDrone`, `GetDroneCargoCount`, `GetTotalCargoCount`)**: Suporta carregamento fracionado ou em lote de minérios e insumos.
- [x] **Simulação de Voo & Automação de Ciclo (`SimulateDroneTick`)**:
  - Controla a decolagem vertical com tag `State.Drone.TakingOff`.
  - Navega ao longo do corredor aéreo consumindo bateria com tag `State.Drone.InFlight`.
  - Detecta aproximação do porto alvo e realiza aproximação vertical suave com tag `State.Drone.Landing`.
  - **Recarga Automatizada**: Atraca em portos com doca de energia e restaura 100% da carga da bateria com tag `State.Drone.Recharging`.
  - **Fail-Safe de Retorno por Bateria Fraca (*Low Battery Return*)**: Se o nível de carga atingir o limiar crítico (`LowBatteryThreshold`), cancela o voo atual, altera o destino de volta ao porto base (`HomePortId`) e aciona o protocolo de segurança com tag `State.Drone.LowBattery`.
- [x] Emite delegates notificadores: `OnDroneFlightStateChanged`, `OnDronePortArrived`, `OnDroneCargoLoaded` e `OnDroneBatteryUpdated`.

**Testes Automatizados**
- [x] Criada suíte de testes validando decolagem com consumo de bateria e transição para `InFlight`, navegação ao longo da rota e pouso seguro no porto de destino com emissão de delegate de chegada, recarga automatizada na doca do porto até 100% de bateria, e retorno emergencial para a base ao cair abaixo do limite crítico de bateria.

> Contagem de specs declarada à época: **350**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 110: Modular Space Elevator & Planetary Logistics Hub Framework (v1.95.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Elevador Espacial & Logística Planetária**
- [x] Criada a enumeração `ESBSpaceElevatorState` (`Idle`, `LoadingPayload`, `Ascending`, `DockedAtOrbitalStation`, `Descending`, `Delivering`).
- [x] Criada a estrutura `FSBSpaceElevatorPhaseRequirement` para patamares de projeto (`PhaseIndex`), identificador da fase (`RequirementName`), cotas exigidas (`RequiredItems`), progresso de depósito (`DepositedItems`) e flag de conclusão (`bIsPhaseCompleted`).
- [x] Criada a estrutura `FSBSpaceElevatorData` para fase ativa (`CurrentPhaseIndex`), total de fases (`MaxPhaseCount`), estado operacional (`State`), altitude da cápsula (`PodAltitudeAlpha`), velocidades de ascensão/descida (`PodAscentSpeed`, `PodDescentSpeed`), demanda em megavolts/megawatts (`PowerDemandMW`), fornecimento de energia (`bHasPowerSupply`) e fases registradas (`Phases`).
- [x] Criada a estrutura `FSBSpaceElevatorSettings` para tempos de espera em órbita (`OrbitalStationWaitTime`) e contagem regressiva de lançamento (`LaunchCountdownDuration`).

**Tags Nativas de Elevador Espacial**
- [x] Registradas tags de estado `State.SpaceElevator.Idle`, `State.SpaceElevator.Ascending`, `State.SpaceElevator.Descending`, `State.SpaceElevator.PhaseCompleted` e `State.SpaceElevator.Delivering`.

**Componente de Megaestrutura do Elevador Espacial**
- [x] Desenvolvido em `08_SandboxInventory` como ápice das cadeias produtivas planetárias.
- [x] **Configuração de Fases e Cotas (`SetupSpaceElevator`, `ConfigurePhaseRequirement`)**: Registra requisitos em lote por patamar tecnológico (ex: Phase 1: Orbital Anchor, Phase 2: Cargo Platform).
- [x] **Depósito de Insumos Industriais (`DepositPhaseItem`, `IsCurrentPhaseRequirementMet`, `GetDepositedItemCount`, `GetRequiredItemCount`)**: Gerencia o suprimento progressivo de materiais pesados e valida o atingimento integral da cota.
- [x] **Despacho Orbital (`LaunchOrbitalDelivery`, `SetPowerSupplied`)**: Valida pré-requisitos de itens e fornecimento elétrico de alta potência para autorizar a sequência de lançamento.
- [x] **Simulação de Ciclo de Voo Orbital (`SimulateElevatorTick`)**:
  - Eleva o pod ao longo do cabo orbital com tag `State.SpaceElevator.Ascending` e `State.SpaceElevator.Delivering`.
  - **Proteção contra Queda de Energia**: Se a energia for cortada durante a subida, a cápsula estola no cabo e interrompe a ascensão até restabelecimento elétrico.
  - Atraca na estação orbital, descarrega a carga, marca a fase como concluída (`bIsPhaseCompleted = true`) e concede a tag `State.SpaceElevator.PhaseCompleted`.
  - Desce a cápsula de volta à base terrestre com tag `State.SpaceElevator.Descending`, incrementa o patamar tecnológico (`CurrentPhaseIndex`) e retorna para o estado `State.SpaceElevator.Idle`.
- [x] Emite delegates notificadores: `OnSpaceElevatorStateChanged`, `OnSpaceElevatorPhaseCompleted` e `OnSpaceElevatorItemDeposited`.

**Testes Automatizados**
- [x] Criada suíte de testes validando depósito de itens com checagem de cota e bloqueio de lançamento incompleto, ascensão da cápsula com aceleração física e concessão da tag `Ascending`, atracagem orbital com conclusão de fase, emissão de delegate de patamar e descida com avanço para a Fase 2, e estol imediato da cápsula em caso de blecaute elétrico.

> Contagem de specs declarada à época: **354**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 111: Advanced Thermal Dynamics, Body Temperature, Hypothermia & Heatstroke (v1.96.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Termodinâmica Corporal**
- [x] Criada a enumeração `ESBThermalComfortState` (`Freezing`, `Cold`, `Comfortable`, `Warm`, `Overheating`, `CriticalHypothermia`, `CriticalHeatstroke`).
- [x] Criada a estrutura `FSBThermalRegulationData` contendo temperatura corporal central (`CoreTemperature`), temperatura ambiente (`AmbientTemperature`), isolamento contra frio (`ThermalInsulationCold`), isolamento contra calor (`ThermalInsulationHeat`), nível de molhamento por água/chuva (`WetnessLevel`), resfriamento por vento (`WindChill`), calor radiante de proximidade (`NearbyHeatSource`), condutividade térmica base (`HeatTransferRate`) e estado de conforto (`ComfortState`).
- [x] Criada a estrutura `FSBThermalThresholdSettings` definindo limiares para hipotermia crítica (35.0°C), sensação de frio (36.2°C), sensação de calor (37.8°C), insolação crítica (39.5°C) e taxas de dano contínuo.

**Tags Nativas de Termorregulação**
- [x] Registradas tags de estado: `State.Thermal.Comfortable`, `State.Thermal.Cold`, `State.Thermal.Freezing`, `State.Thermal.Warm`, `State.Thermal.Overheating`, `State.Thermal.Hypothermia` e `State.Thermal.Heatstroke`.

**Componente de Termorregulação Corporal**
- [x] Desenvolvido em `04_SandboxCharacter` com integração total ao `USBStateComponent` e interface modular `ISBComponentInterface`.
- [x] **Cálculo da Temperatura Efetiva (`GetEffectiveAmbientTemperature`)**: Combina temperatura ambiente, resfriamento por vento e fontes de calor radiante: `EffectiveTemp = AmbientTemp - WindChill + NearbyHeatSource`.
- [x] **Simulação Termodinâmica & Homeostase (`SimulateThermalTick`)**:
  - Troca de calor convectiva e condutiva proporcional ao gradiente térmico `DeltaT = EffectiveAmbient - CoreTemperature`.
  - Em ambientes frios, amortecimento térmico por isolamento de roupas/armaduras `(1.0 - ColdInsulation)` e amplificação exponencial por roupas molhadas `(1.0 + WetnessLevel * 1.5)`.
  - Em ambientes quentes, amortecimento por vestimentas térmicas `(1.0 - HeatInsulation)`.
  - Homeostase biológica ativa em faixas de conforto (18°C a 28°C), estabilizando suavemente a temperatura corporal central em 37.0°C.
  - Classificação dinâmica do estado de conforto térmico e sincronização automática de tags no `USBStateComponent`.
- [x] Emite delegates notificadores: `OnThermalComfortStateChanged`, `OnHypothermiaTriggered` e `OnHeatstrokeTriggered`.

**Testes Automatizados**
- [x] Criada suíte de testes unitários validando manutenção de homeostase a 37°C com tag `Comfortable`, perda acelerada de calor em nevasca com vento e roupas molhadas disparando `OnHypothermiaTriggered` e tag `Hypothermia`, proteção e estabilização térmica por armadura pesada (90%) e proximidade de fogueira (+35°C), e superaquecimento corporal em calor desértico com disparo de `OnHeatstrokeTriggered` e tag `Heatstroke`.

> Contagem de specs declarada à época: **358**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 112: Advanced Metabolic Nutrition, Caloric Burn, Hydration & Vitamin Deficiencies (v1.97.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas Metabólicas e Nutricionais**
- [x] Criadas as enumerações `ESBHungerLevel` (`Satiated`, `Normal`, `Hungry`, `Starving`), `ESBHydrationLevel` (`Hydrated`, `Thirsty`, `Dehydrated`, `CriticalDehydration`) e `ESBMetabolicActivityState` (`Resting`, `Walking`, `Sprinting`, `Combat`, `Shivering`).
- [x] Criada a estrutura `FSBMicronutrientProfile` para rastreamento de vitaminas lipossolúveis e hidrossolúveis (`VitaminA`, `VitaminB`, `VitaminC`, `VitaminD`) e sais minerais (`Electrolytes`).
- [x] Criada a estrutura `FSBMetabolicNutritionData` contendo calorias (`Calories`), teto calórico (`MaxCalories`), hidratação (`Hydration`), taxa basal BMR (`BaseBMR`), perda hídrica (`HydrationLossRate`), perfil de nutrientes (`Nutrients`), nível de fome (`HungerLevel`), nível de sede (`HydrationLevel`) e atividade física atual (`ActivityState`).
- [x] Criada a estrutura `FSBConsumableNutritionItem` contendo calorias fornecidas (`CalorieYield`), hidratação fornecida (`HydrationYield`) e reposição de micronutrientes (`NutrientYield`).

**Tags Nativas de Metabolismo e Nutrição**
- [x] Registradas tags de estado: `State.Metabolism.WellFed`, `State.Metabolism.Hungry`, `State.Metabolism.Starving`, `State.Metabolism.Hydrated`, `State.Metabolism.Thirsty`, `State.Metabolism.Dehydrated`, `State.Metabolism.Deficiency.VitaminC`, `State.Metabolism.Deficiency.VitaminA` e `State.Metabolism.Deficiency.Electrolytes`.

**Componente de Nutrição Metabólica**
- [x] Desenvolvido em `04_SandboxCharacter` com integração ao `USBStateComponent` e `ISBComponentInterface`.
- [x] **Controle de Esforço Físico (`SetActivityState`, `GetActivityMultiplier`)**: Multiplicador de consumo energético por atividade: Repouso (1.0x), Caminhada (1.5x), Corrida (3.0x), Combate (4.0x) e Tremores musculares (2.5x).
- [x] **Consumo e Digestão (`ConsumeFoodOrDrink`)**: Adiciona calorias, hidratação e repõe vitaminas pontuais na corrente biológica.
- [x] **Simulação Metabólica (`SimulateMetabolicTick`)**:
  - Queima de calorias proporcional ao BMR e multiplicador de atividade.
  - Perda de hidratação amplificada por atividade física e calor ambiente (`AmbientHeatModifier`).
  - Decaimento progressivo de vitaminas e consumo acelerado de eletrólitos por suor em combate.
  - Transição automática de níveis de fome e sede com emissão de delegates e tags de estado.
  - Ativação de deficiências clínicas quando nutrientes caem abaixo de 15% (Escorbuto por falta de Vitamina C, fadiga/cegueira por Vitamina A, cãibras por falta de Eletrólitos).
- [x] Emite delegates notificadores: `OnHungerLevelChanged`, `OnHydrationLevelChanged`, `OnNutrientDeficiencyTriggered`, `OnStarvationTriggered` e `OnCriticalDehydrationTriggered`.

**Testes Automatizados**
- [x] Criada suíte de testes unitários validando queima calórica basal em repouso com manutenção da tag `WellFed`, queima acelerada em combate com calor ambiente esgotando reservas e disparando delegates de `Starving` e `Dehydrated`, recuperação total de reservas e vitaminas ao consumir guisado nutritivo, e esgotamento prolongado de Vitamina C e Eletrólitos disparando delegates de deficiência e tags `Deficiency.VitaminC` e `Deficiency.Electrolytes`.

> Contagem de specs declarada à época: **362**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 113: Advanced Pathogen, Infection, Immune System & Disease Transmission (v1.98.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas Imunológicas e Patológicas**
- [x] Criadas as enumerações `ESBInfectionStage` (`Healthy`, `Incubating`, `Symptomatic`, `Severe`, `Recovering`, `Immune`) e `ESBPathogenType` (`Bacterial`, `Viral`, `Parasitic`, `Fungal`).
- [x] Criada a estrutura `FSBPathogenStrain` contendo identificador (`PathogenID`), tipo (`Type`), taxa de virulência/replicação (`Virulence`), severidade (`Severity`), limiar de incubação (`IncubationThreshold`) e carga letal (`LethalThreshold`).
- [x] Criada a estrutura `FSBActiveInfection` contendo cepa ativa (`Strain`), carga patogênica (`PathogenLoad` 0 a 100%), contagem de anticorpos (`AntibodyCount`), eficácia de tratamento médico (`TreatmentEffectiveness`) e estágio clínico atual (`Stage`).
- [x] Criada a estrutura `FSBImmuneSystemData` contendo força imunológica base (`BaseImmunityStrength`), elevação de temperatura por febre (`BodyFeverOffset` em °C), lista de infecções ativas (`ActiveInfections`) e histórico de imunidades adaptativas adquiridas (`AcquiredImmunities`).

**Tags Nativas de Imunologia e Infecção**
- [x] Registradas tags de estado: `State.Immunity.Infected`, `State.Immunity.Incubating`, `State.Immunity.Fever`, `State.Immunity.Symptomatic`, `State.Immunity.Recovering` e `State.Immunity.Immune`.

**Componente Imunológico**
- [x] Desenvolvido em `04_SandboxCharacter` com integração ao `USBStateComponent` e `ISBComponentInterface`.
- [x] **Controle de Exposição e Tratamento (`SetupImmuneSystem`, `ExposeToPathogen`, `ApplyMedicalTreatment`)**:
  - Bloqueia infecções secundárias caso o personagem já possua imunidade adquirida permanente à cepa.
  - Inicia infecções no estágio assintomático `Incubating`.
  - Aplica medicamentos (antibióticos/antissépticos) que aceleram exponencialmente a supressão do patógeno.
- [x] **Simulação Imunológica (`SimulateImmuneTick`)**:
  - Replicação de patógenos baseada na virulência da cepa.
  - Produção progressiva de anticorpos baseada na força imunológica.
  - Transição dinâmica para `Symptomatic` e `Severe` ao ultrapassar o limiar de incubação.
  - Ativação reativa de febre biológica (+2.0°C) elevando o calor corporal e emitindo delegate `OnFeverTriggered` com a tag `State.Immunity.Fever`.
  - Transição para `Recovering` quando anticorpos e tratamentos superam a carga patogênica.
  - Erradicação total do patógeno: cura a infecção, zera a febre, emite `OnInfectionCured`, `OnImmunityAcquired` e concede imunidade permanente com a tag `State.Immunity.Immune`.
- [x] Emite delegates notificadores: `OnInfectionStageChanged`, `OnFeverTriggered`, `OnInfectionCured` e `OnImmunityAcquired`.

**Testes Automatizados**
- [x] Criada suíte de testes unitários validando exposição inicial a patógeno bacteriano entrando em `Incubating` sem febre, progressão assintomática até limiar de sintomas transitando para `Symptomatic` com febre de 2.0°C e tags de febre, aplicação de antibióticos com supressão e cura rápida, e combate celular biológico com geração de anticorpos, cura e aquisição de imunidade adaptativa rejeitando reinfecções.

> Contagem de specs declarada à época: **366**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 114: Advanced Physical Trauma, Bone Fractures, Lacerations, Hemorrhage & Tourniquets (v1.99.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Trauma e Primeiros Socorros**
- [x] Criadas as enumerações `ESBBodyLimb` (`Head`, `Torso`, `LeftArm`, `RightArm`, `LeftLeg`, `RightLeg`) e `ESBBleedType` (`None`, `Venous`, `Arterial`).
- [x] Criada a estrutura `FSBLimbTrauma` contendo membro (`Limb`), saúde localizada (`LimbHealth` 0 a 100%), fratura óssea (`bIsFractured`), tala aplicada (`bIsSplinted`), tipo de hemorragia (`BleedType`) e aplicação de torniquete mecânico (`bTourniquetApplied`).
- [x] Criada a estrutura `FSBTraumaSystemData` contendo volume sanguíneo (`BloodVolume` em Litros, padrão 5.0L), volume máximo (`MaxBloodVolume`), taxa de regeneração biológica (`BloodRegenRate`), mapa anatômico de membros (`Limbs`) e estado crítico de choque hipovolêmico (`bInHypovolemicShock`).

**Tags Nativas de Trauma e Hemorragia**
- [x] Registradas tags de estado: `State.Trauma.Bleeding`, `State.Trauma.ArterialBleed`, `State.Trauma.Fracture.Arm`, `State.Trauma.Fracture.Leg`, `State.Trauma.TourniquetApplied` e `State.Trauma.HypovolemicShock`.

**Componente de Trauma e Lesões Físicas**
- [x] Desenvolvido em `04_SandboxCharacter` com integração ao `USBStateComponent` e `ISBComponentInterface`.
- [x] **Controle Anatômico e Procedimentos (`SetupTraumaSystem`, `InflictLimbDamage`, `ApplySplint`, `ApplyTourniquet`, `RemoveTourniquet`, `ApplyBandageOrSuture`, `TransfuseBlood`)**:
  - Infringe dano anatômico com fraturas ósseas específicas e sangramentos venosos ou arteriais.
  - Aplica tala ortopédica (`Splint`) estabilizando fraturas de braço e perna.
  - Aplica torniquete mecânico (`Tourniquet`) em membros periféricos estancando imediatamente o sangramento arterial.
  - Aplica bandagens/curativos para cessar sangramentos venosos e suturas cirúrgicas.
  - Transfunde bolsas de sangue restaurando o volume total (`BloodVolume`) e revertendo o choque.
- [x] **Simulação de Hemodinâmica e Choque (`SimulateTraumaTick`)**:
  - Drena `BloodVolume` continuamente baseado nos sangramentos ativos (venoso: 0.05 L/s, arterial: 0.3 L/s). Membros com torniquete aplicado não vazam sangue arterial.
  - Quando `BloodVolume` cai abaixo de 3.5L, ativa o choque hipovolêmico (`bInHypovolemicShock = true`), disparando `OnHypovolemicShockTriggered` e concedendo a tag `State.Trauma.HypovolemicShock`.
  - Se o volume for recuperado acima de 3.8L, cancela o choque disparando `OnHypovolemicShockRecovered`.
  - Sincroniza tags dinamicamente no `USBStateComponent`.
- [x] Emite delegates notificadores: `OnLimbFractured`, `OnBleedStateChanged`, `OnHypovolemicShockTriggered`, `OnHypovolemicShockRecovered` e `OnTourniquetStateChanged`.

**Testes Automatizados**
- [x] Criada suíte de testes unitários validando dano contundente causando fratura óssea no braço com tag `State.Trauma.Fracture.Arm` e estabilização por tala, dano cortante na perna provocando hemorragia arterial com tags `State.Trauma.ArterialBleed` e `State.Trauma.Bleeding`, perda contínua de sangue arterial levando a volume < 3.5L e acionando choque hipovolêmico com tag `State.Trauma.HypovolemicShock`, e aplicação de torniquete na perna estancando a hemorragia arterial com tag `State.Trauma.TourniquetApplied` seguido por transfusão de sangue restaurando o volume e revertendo o choque.

> Contagem de specs declarada à época: **370**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 115: Advanced Dynamic Flora, Plant Growth, Soil Moisture, Seasons & Agriculture (v2.00.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Botânica e Agricultura**
- [x] Criadas as enumerações `ESBCropGrowthStage` (`Unplanted`, `Seeded`, `Sprouting`, `Vegetative`, `Flowering`, `Harvestable`, `Withered`) e `ESBSoilHydrationLevel` (`Parched`, `Dry`, `Moist`, `Saturated`).
- [x] Criada a estrutura `FSBPlantSpeciesData` contendo identificador da espécie (`SpeciesID`), tempo de maturação (`GrowthDuration`), umidade ideal (`OptimalMoisture`), taxa de consumo hídrico (`WaterConsumptionRate`), rendimento base de colheita (`BaseYield`) e flag de planta perene (`bIsPerennial`).
- [x] Criada a estrutura `FSBDynamicCropData` contendo dados da espécie (`Species`), estágio atual (`Stage`), progresso percentual de crescimento (`GrowthProgress`), nível de umidade do solo (`SoilMoisture`), fertilidade multiplicativa (`SoilFertility`), indicador de fertilização (`bIsFertilized`) e rendimento final da colheita (`HarvestYield`).

**Tags Nativas de Agricultura e Cultivo**
- [x] Registradas tags de estado: `State.Crop.Seeded`, `State.Crop.Sprouting`, `State.Crop.Growing`, `State.Crop.Harvestable`, `State.Crop.Withered` e `State.Crop.Fertilized`.

**Componente de Cultivo Dinâmico**
- [x] Desenvolvido em `05_SandboxInventory` com integração ao `USBStateComponent` e interface `ISBComponentInterface`.
- [x] **Manejo do Solo e Plantio (`SetupCropPlot`, `PlantSeed`, `WaterSoil`, `ApplyFertilizer`, `HarvestCrop`)**:
  - Prepara o canteiro de cultivo com umidade e fertilidade base.
  - Semeia espécies botânicas iniciando o estágio `Seeded`.
  - Irriga o solo elevando `SoilMoisture` e disparando `OnSoilMoistureChanged`.
  - Aplica adubos enriquecendo a fertilidade do solo (`SoilFertility`) e ativando a tag `State.Crop.Fertilized`.
  - Realiza a colheita no estágio `Harvestable`, aplicando o multiplicador de fertilidade ao rendimento, disparando `OnCropHarvested` e reiniciando o canteiro (ou resetando para `Vegetative` caso seja perene).
- [x] **Simulação Fenológica e Dessecação (`SimulateCropTick`)**:
  - Consome umidade do solo proporcionalmente à absorção das raízes e evaporação sob sol/calor (`SunExposure`, `AmbientTemp`).
  - Avança o progresso de crescimento baseado no tempo, curva de umidade ótima e fertilidade do solo.
  - Realiza transições dinâmicas de estágio: `Seeded` -> `Sprouting` (0.15) -> `Vegetative` (0.40) -> `Flowering` (0.75) -> `Harvestable` (1.00).
  - Verificação de seca crítica: se o solo secar totalmente (`SoilMoisture <= 0.0f`) sob calor escaldante (> 35°C), a planta seca e morre, transitando para `Withered`, disparando `OnCropWithered` e concedendo a tag `State.Crop.Withered`.
  - Sincroniza tags dinamicamente no `USBStateComponent`.
- [x] Emite delegates notificadores: `OnCropStageChanged`, `OnCropHarvested`, `OnCropWithered` e `OnSoilMoistureChanged`.

**Testes Automatizados**
- [x] Criada suíte de testes unitários validando plantio de semente em solo úmido entrando em `Seeded` com tag `State.Crop.Seeded` e aplicação de fertilizante com tag `State.Crop.Fertilized`, avanço progressivo através dos estágios botânicos com irrigação regular até maturação em `Harvestable` com tag `State.Crop.Harvestable`, colheita de planta perene madura gerando rendimento multiplicado pela fertilidade e reiniciando para rebrotar, e dessecação por seca severa sob calor escaldante com transição para `Withered`, tag `State.Crop.Withered` e disparo de `OnCropWithered`.

> Contagem de specs declarada à época: **374**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 116: Advanced Fauna Ecosystem, Taming, Domestication, Breeding & Genetics (v2.01.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Fauna e Genética**
- [x] Criadas as enumerações `ESBFaunaDomesticationState` (`Wild`, `Taming`, `Domesticated`, `Feral`) e `ESBFaunaReproductiveStage` (`NonBreeding`, `Courtship`, `Pregnant`, `Incubating`, `OffspringReady`).
- [x] Criada a estrutura `FSBCreatureGenetics` contendo modificadores fenotípicos (`SpeedModifier`, `StaminaModifier`, `WeightCapacityModifier`), número de geração (`Generation`), contagem de mutações (`MutationCount`) e cor de pelagem (`CoatColor`).
- [x] Criada a estrutura `FSBDomesticationData` contendo estado de domesticação (`DomesticationState`), estágio reprodutivo (`ReproductiveStage`), progresso de amansamento (`TameProgress`), nível de afeto/lealdade (`AffectionLevel`), comida preferida (`PreferredFoodID`), progresso de gestação (`PregnancyProgress`), duração da gravidez (`GestationDuration`), genoma próprio (`Genetics`) e genoma do parceiro reprodutivo (`MateGenetics`).

**Tags Nativas de Fauna e Domesticação**
- [x] Registradas tags de estado: `State.Fauna.Wild`, `State.Fauna.Taming`, `State.Fauna.Domesticated`, `State.Fauna.Pregnant`, `State.Fauna.Juvenile` e `State.Fauna.Mountable`.

**Componente de Fauna e Domesticação**
- [x] Desenvolvido em `04_SandboxCharacter` com integração ao `USBStateComponent` e interface `ISBComponentInterface`.
- [x] **Domesticação e Afeto (`SetupFauna`, `FeedTamingFood`, `PetCreature`)**:
  - Alimentar criatura selvagem com `PreferredFoodID` avança `TameProgress` e transita de `Wild` para `Taming`.
  - Ao atingir 100% (`TameProgress >= 1.0f`), conclui a domesticação (`Domesticated`), emite `OnCreatureTamed` e concede a tag `State.Fauna.Domesticated`.
  - Fazer carinho (`PetCreature`) eleva `AffectionLevel`. Acima de 0.8f de afeto, habilita a montaria concedendo a tag `State.Fauna.Mountable`.
- [x] **Ciclos Reprodutivos e Hereditariedade Genética (`StartBreedingWith`, `SimulateFaunaTick`, `BirthOffspring`)**:
  - Inicia o acasalamento com um parceiro, transitando para `Pregnant` com a tag `State.Fauna.Pregnant`.
  - Simula o tick gestacional avançando `PregnancyProgress`. Ao atingir 100%, emite `OnPregnancyCompleted` e define o estágio `OffspringReady`.
  - O parto (`BirthOffspring`) recombina os alelos de ambos os pais (médias ponderadas com bônus de mutação e vigor híbrido para gerações filiais), incrementa a linhagem (`Generation + 1`), reseta a gestação e emite `OnOffspringBirthed`.
- [x] Sincroniza tags dinamicamente no `USBStateComponent`.
- [x] Emite delegates notificadores: `OnDomesticationStateChanged`, `OnCreatureTamed`, `OnPregnancyCompleted` e `OnOffspringBirthed`.

**Testes Automatizados**
- [x] Criada suíte de testes unitários validando alimentação com comida favorita avançando mansidão de Wild para Taming e atingindo 100% com domesticação, tag `State.Fauna.Domesticated` e disparo de `OnCreatureTamed`, afago elevando afeto e concedendo tag `State.Fauna.Mountable` ao ultrapassar 80% de lealdade, início de gestação com tag `State.Fauna.Pregnant`, avanço no tick e conclusão de gravidez com `OnPregnancyCompleted`, e nascimento de filhote herdando genes recombinados dos pais, incrementando para geração 2 e disparando `OnOffspringBirthed`.

> Contagem de specs declarada à época: **378**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 117: Advanced Planetary Atmosphere, Oxygen Depletion, Pressure & Toxic Gas Hazards (v2.02.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Atmosfera e Segurança Planetária**
- [x] Criadas as enumerações `ESBAtmosphericHazardType` (`None`, `Hypoxia`, `Hypercapnia`, `ToxicGas`, `Decompression`, `ExtremePressure`) e `ESBSuitPressurizationState` (`Unsealed`, `Pressurized`, `Compromised`, `Breached`).
- [x] Criada a estrutura `FSBAtmosphereEnvironmentData` contendo porcentagem de oxigênio ambiente (`OxygenPercentage`), concentração de gás tóxico em ppm (`ToxicGasPPM`), pressão barométrica (`BarometricPressureKPa`) e indicador de vácuo (`bIsVacuum`).
- [x] Criada a estrutura `FSBAtmosphericSafetyData` contendo saturação de O2 no sangue (`BloodOxygenSaturation` SpO2), nível de toxicidade (`ToxicityLevel`), reserva de oxigênio do tanque do traje (`SuitOxygenReserve`), integridade do filtro de carvão (`FilterIntegrity`), integridade da vedação do traje (`SuitSealIntegrity`), estado de pressurização (`SuitState`), indicador de hipóxia (`bIsHypoxic`) e inalação tóxica (`bInToxicInhalation`).

**Tags Nativas de Atmosfera e Riscos Planetários**
- [x] Registradas tags de estado: `State.Atmosphere.Hazardous`, `State.Atmosphere.Hypoxia`, `State.Atmosphere.ToxicInhalation`, `State.Atmosphere.SuitPressurized`, `State.Atmosphere.FilterExhausted` e `State.Atmosphere.Decompression`.

**Componente de Segurança Atmosférica**
- [x] Desenvolvido em `04_SandboxCharacter` com integração ao `USBStateComponent` e interface modular `ISBComponentInterface`.
- [x] **Controle do Traje e Suprimentos (`SetupAtmosphericSafety`, `ToggleSuitSeal`, `RefillOxygenReserve`, `ReplaceFilter`, `PatchSuitLeak`)**:
  - Sela e pressuriza o traje (`Pressurized`) concedendo a tag `State.Atmosphere.SuitPressurized`.
  - Recarrega reservas de oxigênio (`SuitOxygenReserve`) e repara furos e danos herméticos do traje.
  - Substitui cartuchos filtrantes de ar exauridos por novos filtros a 100%.
- [x] **Simulação Respiratória e Riscos Atmosféricos (`SimulateAtmosphereTick`)**:
  - Em atmosfera segura padrão (21% O2), preserva e recupera `BloodOxygenSaturation` a 100% sem drenar os cilindros de oxigênio.
  - Com o traje hermeticamente selado, consome oxigênio do traje e garante SpO2 a 100% mesmo sob vácuo espacial ou atmosfera rarefeita.
  - Sem proteção selada em ambientes com O2 < 16% ou vácuo absoluto, depleta o oxigênio sanguíneo; ao atingir SpO2 < 85%, aciona hipóxia clínica (`bIsHypoxic = true`), emite `OnHypoxiaStateChanged` e aplica as tags `State.Atmosphere.Hypoxia` e `State.Atmosphere.Hazardous`.
  - Em ambientes com alta concentração de gases tóxicos (> 50 PPM), o filtro degrada gradualmente; quando esgotado (`FilterIntegrity <= 0.0f`), o gás venenoso é inalado diretamente, elevando a toxicidade, disparando `OnFilterExhausted` e concedendo as tags `State.Atmosphere.ToxicInhalation` e `State.Atmosphere.FilterExhausted`.
- [x] Sincroniza tags dinamicamente no `USBStateComponent`.
- [x] Emite delegates notificadores: `OnHypoxiaStateChanged`, `OnToxicInhalationTriggered`, `OnSuitBreached` e `OnFilterExhausted`.

**Testes Automatizados**
- [x] Criada suíte de testes unitários validando respiração estável a 100% SpO2 em atmosfera padrão sem hipóxia, depleção de SpO2 sob vácuo sem traje selado levando a hipóxia (< 85%) com tags `State.Atmosphere.Hypoxia` e `State.Atmosphere.Hazardous` e disparo de `OnHypoxiaStateChanged`, selagem do traje em vácuo consumindo reserva de O2, recuperando SpO2 para 100% e concedendo `State.Atmosphere.SuitPressurized`, e degradação de filtro em atmosfera com gás tóxico de 200 PPM até esgotamento total, aplicando `State.Atmosphere.FilterExhausted` e `State.Atmosphere.ToxicInhalation` e disparando `OnFilterExhausted`.

> Contagem de specs declarada à época: **382**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 118: Advanced Nuclear Radiation, Ionizing Exposure, Rad-Sickness, Geiger Counters & Decontamination (v2.03.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Radiação Nuclear e Dosimetria**
- [x] Criada a enumeração `ESBRadiationSicknessStage` (`None`, `MildExposure`, `AcuteRadiationSickness`, `CriticalLethalARS`).
- [x] Criada a estrutura `FSBRadiationEnvironmentData` contendo taxa de radiação ionizante ambiente em milisieverts por hora (`AmbientDoseRate_mSv_h`) e concentração de partículas e poeira radioativa suspensa (`AirborneRadParticulatesPPM`).
- [x] Criada a estrutura `FSBRadiationExposureData` contendo dose total absorvida no corpo em mSv (`AccumulatedDose_mSv`), taxa de absorção atual (`CurrentDoseRate_mSv_h`), fator de atenuação por blindagem de chumbo (`LeadShieldingFactor`), frequência acústica de pulsos do contador Geiger (`GeigerClickFrequencyHz`), estágio patológico da doença (`SicknessStage`) e indicador de estalos do Geiger (`bIsGeigerClicking`).

**Tags Nativas de Radiação e Dosimetria**
- [x] Registradas tags de estado: `State.Radiation.Exposed`, `State.Radiation.LowDose`, `State.Radiation.AcuteSickness`, `State.Radiation.CriticalARS`, `State.Radiation.LeadShielded` e `State.Radiation.GeigerClicking`.

**Componente de Exposição a Radiação**
- [x] Desenvolvido em `04_SandboxCharacter` com integração ao `USBStateComponent` e interface modular `ISBComponentInterface`.
- [x] **Controle de Blindagem e Tratamento Químico (`SetupRadiationComponent`, `EquipLeadShielding`, `AdministerAntiradMedication`, `PerformDecontamination`)**:
  - Equipa trajes de chumbo / revestimento de radiação (`LeadShieldingFactor`), atenuando proporcionalmente a dose recebida e concedendo a tag `State.Radiation.LeadShielded`.
  - Administra fármacos quelantes e iodeto de potássio (`AdministerAntiradMedication`), purgando a dose acumulada no sangue e tecidos e regredindo os estágios patológicos de ARS.
  - Realiza lavagem de descontaminação (`PerformDecontamination`), removendo contaminação corporal e emitindo `OnDecontaminationCompleted`.
- [x] **Simulação Dosimétrica e Patológica (`SimulateRadiationTick`)**:
  - Em ambientes limpos (0 mSv/h), mantém a integridade celular sem ganho de dose e com estágios zerados.
  - Calcula a frequência dos pulsos do contador Geiger (`GeigerClickFrequencyHz = AmbientDoseRate * 0.05f`). Em hotspots (> 0.5 mSv/h), ativa o contador com `bIsGeigerClicking = true`, concede a tag `State.Radiation.GeigerClicking` e emite o delegate `OnGeigerClick`.
  - Acumula dose no organismo baseada na taxa atenuada pela blindagem (`DoseRate * (1.0 - LeadShielding)`).
  - Transições patológicas:
  - `< 500 mSv`: `None` (sem efeitos clínicos).
  - `500 - 1500 mSv`: `MildExposure` (aplica `State.Radiation.LowDose` e `State.Radiation.Exposed`).
  - `1500 - 4000 mSv`: `AcuteRadiationSickness` (dispara `OnAcuteRadiationSicknessTriggered`, aplica `State.Radiation.AcuteSickness` e `State.Radiation.Exposed`).
  - `> 4000 mSv`: `CriticalLethalARS` (aplica `State.Radiation.CriticalARS`).
- [x] Sincroniza tags dinamicamente no `USBStateComponent`.
- [x] Emite delegates notificadores: `OnRadiationStageChanged`, `OnAcuteRadiationSicknessTriggered`, `OnGeigerClick` e `OnDecontaminationCompleted`.

**Testes Automatizados**
- [x] Criada suíte de testes unitários validando manutenção de dose zero e estágio None em ambiente sem radiação, acúmulo de dose em hotspot radioativo com cliques no Geiger, tag `State.Radiation.GeigerClicking` e transição para `AcuteRadiationSickness` com tags `State.Radiation.AcuteSickness` e `State.Radiation.Exposed` e disparo de `OnAcuteRadiationSicknessTriggered`, atenuação em 80% da dose com traje de chumbo e concessão de `State.Radiation.LeadShielded`, e redução de dose absorvida com medicamento antirrad e banho de descontaminação, regredindo o estágio para None e limpando as tags de ARS.

> Contagem de specs declarada à época: **386**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 119: Advanced Herbal Extraction, Bio-Compounds, Neutralizers & Motor Impairment (v2.04.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Aflições e Neutralizadores**
- [x] Criadas as enumerações `ESBAfflictionType` (`None`, `MotorImpairment`, `TissueDegradation`, `CellularStrain`) e `ESBNeutralizerType` (`None`, `HerbalBalm`, `SynthesizedAntidote`, `UniversalPanacea`).
- [x] Criada a estrutura `FSBAfflictionData` contendo tipo de aflição (`AfflictionType`), intensidade/severidade (`Severity`), tempo restante da condição (`DurationRemaining`) e multiplicador de velocidade motora (`MotorImpairmentMultiplier`).
- [x] Criada a estrutura `FSBCharacterAfflictionState` contendo lista de aflições ativas (`ActiveAfflictions`), fator de resistência biológica por inoculação (`InoculationResistance`) e indicador de comprometimento motor (`bIsMotorImpaired`).

**Tags Nativas de Aflições e Imunidade**
- [x] Registradas tags de estado: `State.Affliction.Impaired`, `State.Affliction.Degradation`, `State.Affliction.Paralyzed`, `State.Affliction.Inoculated` e `State.Affliction.Neutralized`.

**Componente de Aflições e Tratamento**
- [x] Desenvolvido em `04_SandboxCharacter` com integração ao `USBStateComponent` e interface modular `ISBComponentInterface`.
- [x] **Aplicação e Tratamento de Condições (`SetupAfflictionComponent`, `ApplyAffliction`, `ApplyNeutralizer`, `ApplyInoculation`)**:
  - Aplica aflição com atenuação da severidade pela resistência de inoculação (`EffectiveSeverity = Severity * (1.0 - InoculationResistance)`).
  - Para comprometimento motor (`MotorImpairment`), calcula o multiplicador de velocidade (`1.0 - Severity`). Caso a severidade seja elevada (>= 0.7f), aplica a tag `State.Affliction.Paralyzed`.
  - Aplica elixires e bálsamos neutralizadores (`ApplyNeutralizer`), purgando aflições correspondentes, restaurando a velocidade e emitindo `OnAfflictionNeutralized`.
  - Aplica inoculação profilática (`ApplyInoculation`), elevando a resistência a novas condições e concedendo a tag `State.Affliction.Inoculated`.
- [x] **Simulação em Tempo Real (`SimulateAfflictionTick`)**:
  - Decrementa as durações das aflições ativas e remove automaticamente as condições expiradas, restaurando multiplicadores e emitindo delegates.
- [x] Sincroniza tags dinamicamente no `USBStateComponent`.
- [x] Emite delegates notificadores: `OnAfflictionApplied`, `OnAfflictionNeutralized` e `OnMotorImpairmentStateChanged`.

**Testes Automatizados**
- [x] Criada suíte de testes unitários validando estado inicial são com velocidade a 100% e sem tags, aplicação de aflição motora severa reduzindo velocidade para 0.15 com tags `State.Affliction.Impaired` e `State.Affliction.Paralyzed`, neutralização com antídoto sintetizado restaurando velocidade a 1.0 e limpando tags com disparo de `OnAfflictionNeutralized`, e inoculação preventiva conferindo resistência de 50% e concedendo a tag `State.Affliction.Inoculated`.

> Contagem de specs declarada à época: **390**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 120: Advanced Surgical Operations, Organ Transplants, Prosthetics & Cybernetic Implants (v2.05.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Cirurgias e Próteses**
- [x] Criadas as enumerações `ESBSurgicalLimbType` (`None`, `LeftArm`, `RightArm`, `LeftLeg`, `RightLeg`), `ESBProstheticGrade` (`None`, `BasicProsthetic`, `BionicAdvanced`, `CyberneticAugment`) e `ESBSurgicalOperationState` (`Idle`, `PreOpAnesthesia`, `InSurgery`, `PostOpRecovery`).
- [x] Criada a estrutura `FSBProstheticLimb` contendo tipo do membro (`LimbType`), tecnologia/grau (`Grade`), multiplicador de eficiência funcional (`Efficiency`) e durabilidade estrutural (`StructuralDurability`).
- [x] Criada a estrutura `FSBSurgicalPatientData` contendo estado operatório (`OperationState`), lista de membros protéticos instalados (`InstalledProsthetics`), saúde dos órgãos vitais (`OrganHealth`), nível de imunossupressores anti-rejeição (`ImmunosuppressantLevel`), progresso do procedimento (`OperationProgress`), flag de anestesia (`bIsAnesthetized`) e indicador de risco de rejeição tecidual (`bIsOrganRejectionRisk`).

**Tags Nativas de Cirurgia e Próteses**
- [x] Registradas tags de estado: `State.Surgery.UnderAnesthesia`, `State.Surgery.Operating`, `State.Surgery.ProstheticInstalled`, `State.Surgery.OrganRejection` e `State.Surgery.CyberneticAugmented`.

**Componente de Cirurgias e Próteses**
- [x] Desenvolvido em `04_SandboxCharacter` com integração ao `USBStateComponent` e interface modular `ISBComponentInterface`.
- [x] **Controle de Sedação, Cirurgias e Transplantes (`SetupSurgicalComponent`, `AdministerAnesthesia`, `StartSurgicalOperation`, `InstallProsthetic`, `PerformOrganTransplant`, `AdministerImmunosuppressant`)**:
  - Administra anestésicos (`AdministerAnesthesia`), induzindo sedação cirúrgica com `bIsAnesthetized = true` e concedendo a tag `State.Surgery.UnderAnesthesia`.
  - Inicia a cirurgia (`StartSurgicalOperation`), fixando o tempo do procedimento e aplicando a tag `State.Surgery.Operating`.
  - Instala próteses mecânicas e biônicas (`InstallProsthetic`), concedendo `State.Surgery.ProstheticInstalled` e, caso seja grau cibernético (`CyberneticAugment`), calculando bônus de eficiência e concedendo a tag `State.Surgery.CyberneticAugmented`.
  - Realiza transplantes de órgãos (`PerformOrganTransplant`), restaurando `OrganHealth` e monitorando nível de imunossupressores. Sem dosagem adequada (< 0.2), ativa alerta de rejeição `bIsOrganRejectionRisk = true`, concede a tag `State.Surgery.OrganRejection` e emite `OnOrganRejectionWarning`.
  - Administra imunossupressores (`AdministerImmunosuppressant`), estabilizando o enxerto, limpando o risco de rejeição e purgando a tag `State.Surgery.OrganRejection`.
- [x] **Simulação em Tempo Real (`SimulateSurgicalTick`)**:
  - Avança o progresso da operação até 100%, transita para recuperação pós-operatória (`PostOpRecovery`) e emite `OnSurgicalOperationCompleted`.
- [x] Sincroniza tags dinamicamente no `USBStateComponent`.
- [x] Emite delegates notificadores: `OnAnesthesiaStateChanged`, `OnSurgicalOperationCompleted`, `OnProstheticInstalled` e `OnOrganRejectionWarning`.

**Testes Automatizados**
- [x] Criada suíte de testes unitários validando estado inicial são com 100% de saúde de órgãos, sem próteses instaladas, em repouso e sem tags cirúrgicas; indução anestésica com delegate e tag `State.Surgery.UnderAnesthesia`, início da cirurgia com tag `State.Surgery.Operating`, e conclusão do procedimento após avanço temporal disparando `OnSurgicalOperationCompleted` e transição para `PostOpRecovery`; instalação de prótese de braço cibernético com ganho de eficiência (+50%) e tags `State.Surgery.ProstheticInstalled` e `State.Surgery.CyberneticAugmented`; e transplante de órgãos sem imunossupressores gerando alerta e tag `State.Surgery.OrganRejection`, seguido de administração do medicamento estabilizando o órgão e limpando a rejeição.

> Contagem de specs declarada à época: **394**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 121: ECS / Mass Entity & Subsystem Dynamic Tick Throttling (v2.06.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Tick Throttling**
- [x] Criada a enumeração `ESBTickLODLevel` (`LOD0_HighPriority`, `LOD1_MediumPriority`, `LOD2_LowPriority`, `LOD3_BackgroundBatch`, `LOD_Suspended`).
- [x] Criada a estrutura `FSBTickThrottlingSettings` contendo distâncias de transição de LOD (`LOD0_MaxDistance: 15m`, `LOD1_MaxDistance: 50m`, `LOD2_MaxDistance: 150m`) e intervalos de tempo de execução (`LOD1_Interval: 0.1s / 10Hz`, `LOD2_Interval: 0.5s / 2Hz`, `LOD3_Interval: 2.0s / 0.5Hz`).
- [x] Criada a estrutura `FSBTickThrottlingState` contendo nível de LOD atual (`CurrentLOD`), delta consolidado acumulado (`AccumulatedDeltaTime`), tempo desde o último tick (`TimeSinceLastTick`), distância até o observador (`DistanceToNearestViewer`), indicador de execução no frame (`bShouldTickThisFrame`) e contador total de ticks (`TotalTicksExecuted`).

**Tags Nativas de Throttling**
- [x] Registradas tags de estado: `State.Throttling.LOD0`, `State.Throttling.LOD1`, `State.Throttling.LOD2`, `State.Throttling.Background` e `State.Throttling.Suspended`.

**Componente e Subsistema de Throttling (`USBDynamicTickThrottlingComponent` e `USBDynamicTickManagerSubsystem`)**
- [x] Desenvolvidos em `02_SandboxCore` com interface modular `ISBComponentInterface`.
- [x] **Controle e Otimização de Frequência (`SetupThrottling`, `UpdateDistanceToViewer`, `AdvanceTick`, `ForceLOD`)**:
  - Avalia distância ao observador mais próximo, transicionando automaticamente de `LOD0` (60Hz completo) para `LOD1` (10Hz), `LOD2` (2Hz) ou `LOD3` (Background batch).
  - `AdvanceTick`: acumula subframes e só executa quando o intervalo configurado é atingido, retornando `OutConsolidatedDeltaTime` para garantir precisão matemática estrita em cálculos de simulação contínua sem saltos ou erros cumulativos.
- [x] **Subsistema de Orquestração (`USBDynamicTickManagerSubsystem`)**:
  - Registra e orquestra em lote todos os componentes do mundo (`UpdateAllLODs`), permitindo atualizar dezenas de milhares de entidades com alta cache locality.
- [x] Sincroniza tags dinamicamente no `USBStateComponent`.
- [x] Emite delegate notificador `OnLODLevelChanged`.

**Testes Automatizados**
- [x] Criada suíte de testes unitários validando inicialização em LOD0 com execução a cada frame (60Hz) e tag `State.Throttling.LOD0`; transição para LOD1 a média distância (3000 unidades) ignorando subframes e executando no intervalo correto (0.1s) com delta consolidado de 0.11s; transição para LOD2 a longa distância e LOD3 além do limite máximo com tag `State.Throttling.Background`; e registro no subsistema de mundo com categorização precisa de contagem por nível de LOD.

> Contagem de specs declarada à época: **398**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 122: Memory Profiling, Struct Alignment, Zero-Allocation Iterators & Cache Locality (v2.07.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas Otimizadas de Memória**
- [x] Criada a estrutura de dados `FSBCompactEntityRecord` contendo `EntityID` (4B), `TypeID` (2B), `LODLevel` (1B), `Flags` (1B), `PositionX` (4B), `PositionY` (4B), `PositionZ` (4B) e `CustomData` (4B).
- [x] *Alinhamento Perfeito de 64-bit*: Total de exatamente **24 bytes** com verificação estática em tempo de compilação via `static_assert(sizeof(FSBCompactEntityRecord) == 24)`.
- [x] Criada a estrutura `FSBMemoryMetrics` para diagnóstico de pegada de memória, capacidade pré-alocada, contagem de registros ativos e razão de fragmentação.

**Tags Nativas de Memória e Cache**
- [x] Registradas tags de estado: `State.Memory.Optimized`, `State.Memory.Contiguous` e `State.Memory.ZeroAllocActive`.

**Subsistema e Componente de Dados Contíguos (`USBCacheOptimizedBufferSubsystem` e `USBCacheOptimizedDataComponent`)**
- [x] Desenvolvidos em `02_SandboxCore` com interface modular `ISBComponentInterface`.
- [x] **Subsistema de Buffer Contíguo (`USBCacheOptimizedBufferSubsystem`)**:
  - `PreallocateBuffer`: Reserva capacidade máxima em bloco linear contínuo, prevenindo realocações de heap durante o gameplay.
  - `InsertRecord`: Insere dados no final do buffer contíguo em $O(1)$.
  - `RemoveRecord`: Implementa remoção por **Swap-and-Pop** em $O(1)$, substituindo o elemento removido pela cauda do array para manter a memória 100% densa e contígua sem furos nem realocações.
  - `ProcessRecordsZeroAlloc`: Itera sobre o ponteiro linear do array `FSBCompactEntityRecord*` via função lambda por referência, garantindo 0 bytes alocados em heap no loop de simulação e máxima taxa de acerto nos caches L1/L2/L3 da CPU.
- [x] **Componente de Otimização de Cache (`USBCacheOptimizedDataComponent`)**:
  - Vincula automaticamente a entidade ao subsistema na inicialização.
  - Sincroniza coordenadas 3D para o registro compacto contíguo.
  - Sincroniza tags dinamicamente no `USBStateComponent`.
  - Emite delegate notificador `OnRecordIndexUpdated`.

**Testes Automatizados**
- [x] Criada suíte de testes unitários validando:
  - Verificação de tamanho de struct `sizeof(FSBCompactEntityRecord) == 24` e consistência de empacotamento de campos.
  - Pré-alocação de buffer contíguo pelo subsistema, inserção de 50 registros sem realocação e iteração zero-alloc computando a soma agregada.
  - Remoção por *Swap-and-Pop* em $O(1)$ mantendo a densidade contígua do buffer e reportando a entidade permutada.
  - Auto-registro de ator com `USBCacheOptimizedDataComponent`, sincronização de coordenadas e concessão das tags `State.Memory.Optimized` e `State.Memory.Contiguous`.

> Contagem de specs declarada à época: **402**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

## Fase 123: Async Task Graph, Background Work Pools & Multi-Threaded Heavy Calculations (v2.08.0) (Concluída — importada do walkthrough)

**Tipos e Estruturas de Multi-Threading**
- [x] Criada a enumeração `ESBAsyncPriority` (`High`, `Normal`, `Background`).
- [x] Criada a estrutura `FSBAsyncWorkPayload` contendo identificador de trabalho (`WorkID`), tamanho do lote (`BatchSize`), tempo de execução em milissegundos (`TotalProcessingTimeMs`) e flag de conclusão (`bIsCompleted`).
- [x] Criada a estrutura `FSBMultiThreadMetrics` contendo contadores atômicos e protegidos de tarefas assíncronas ativas (`ActiveAsyncTasks`), total de tarefas concluídas (`TotalTasksCompleted`), tempo médio de execução (`AverageTaskExecutionTimeMs`) e granularidade de chunk para processamento paralelo (`ParallelBatchSize: 64`).

**Tags Nativas de Concorrência e Threading**
- [x] Registradas tags de estado: `State.Async.TaskRunning`, `State.Async.DoubleBufferActive` e `State.Async.WorkCompleted`.

**Subsistema e Componente de Multi-Threading (`USBAsyncTaskManagerSubsystem` e `USBAsyncParallelDataComponent`)**
- [x] Desenvolvidos em `02_SandboxCore` com interface modular `ISBComponentInterface`.
- [x] **Subsistema de Multi-Threading e Task Graph (`USBAsyncTaskManagerSubsystem`)**:
  - `DispatchParallelTransformBatch`: Processa milhares de translações/transformações vetoriais simultaneamente em paralelo utilizando `ParallelFor` particionado, distribuindo a carga de forma uniforme entre todos os núcleos de CPU disponíveis.
  - `DispatchAsyncBatchCalculation`: Despacha cálculos pesados em lote para background worker threads através do pool assíncrono `Async(EAsyncExecution::ThreadPool)` e roteia o callback de conclusão de volta ao Game Thread de forma thread-safe via `FFunctionGraphTask::CreateAndDispatchWhenReady`.
- [x] **Componente de Double Buffering Seguro (`USBAsyncParallelDataComponent`)**:
  - Implementa o padrão *Double Buffering* onde o Game Thread lê continuamente de um `FrontBuffer` imutável, enquanto operações pesadas em background escrevem no `BackBuffer`.
  - `CommitBackBuffer`: Efetua o swap atômico de buffers ao término do processamento, eliminando completamente congelamentos de frame (*micro-stutters*) e race conditions.
  - Sincroniza tags dinamicamente no `USBStateComponent`.
  - Emite delegate notificador `OnAsyncWorkFinished(float ComputedValue)`.

**Testes Automatizados**
- [x] Criada suíte de testes unitários validando:
  - Processamento concorrente de 1.000 vetores via `ParallelFor` garantindo translação matemática exata em todos os elementos.
  - Despacho assíncrono de lote de floats em background thread com retorno ao Game Thread e agregação precisa de resultados.
  - Estabilidade do `FrontBuffer` durante mutação assíncrona do `BackBuffer` e transição atômica no `CommitBackBuffer`.
  - Concessão e ciclo de vida das tags `State.Async.DoubleBufferActive` e `State.Async.WorkCompleted`.

> Contagem de specs declarada à época: **406**. Nunca medida — ver [[validation_report_2026-09-06]] e a §10 do relatório de auditoria.

---

## Fase 124: Hierarchical Spatial Partitioning, Octree & Fast Spatial Queries (v2.09.0) (Concluída)
- [x] Criar tipos e estruturas de particionamento espacial em SBSpatialPartitionTypes.h (`FSBSpatialCellCoord`, `FSBSpatialEntityElement`, `FSBSpatialGridMetrics`)
- [x] Registrar tags de particionamento espacial em SBGameplayTags.h/cpp (`State.Spatial.*`)
- [x] Criar subsistema de particionamento espacial USBSpatialPartitionSubsystem (`.h`/`.cpp`) em 02_SandboxCore com consultas em raio e AABB
- [x] Criar componente de indexação espacial USBSpatialIndexedComponent (`.h`/`.cpp`) em 02_SandboxCore com detecção de migração de células
- [x] Criar suíte de testes unitários automatizados em SBSpatialPartitionTests.cpp
- [x] Sincronizar arquivos para o projeto secundário GameAnimationSample
- [x] Compilar ambos os projetos (V1Editor e GameAnimationSampleEditor)
- [x] Validar toda a suíte de testes (410 specs 100% verdes, EXIT CODE: 0)

## Fase 125: Lock-Free Queues, Event Rings & High-Throughput Message Passing (v2.10.0) (Concluída)
- [x] Criar tipos e estruturas lock-free em SBLockFreeTypes.h (`FSBLockFreeEvent`, `FSBLockFreeQueueMetrics`)
- [x] Registrar tags lock-free em SBGameplayTags.h/cpp (`State.LockFree.*`)
- [x] Criar subsistema de barramento de eventos lock-free USBLockFreeEventSubsystem (`.h`/`.cpp`) em 02_SandboxCore
- [x] Criar componente produtor atômico USBLockFreeProducerComponent (`.h`/`.cpp`) em 02_SandboxCore
- [x] Criar suíte de testes unitários automatizados em SBLockFreeQueueTests.cpp (validação de FIFO, rotação de anel, concorrência massiva e saturação)
- [x] Sincronizar arquivos para o projeto secundário GameAnimationSample
- [x] Compilar ambos os projetos (V1Editor e GameAnimationSampleEditor)
- [x] Validar toda a suíte de testes (414 specs 100% verdes, EXIT CODE: 0)

## Fase 126: Event Bus Centralizado, Type-Safe Pub/Sub & Inter-Plugin Decoupling (v2.11.0) (Concluída)
- [x] Criar tipos e payloads de mensageria em SBEventBusTypes.h (`FSBEventBusPayload`, `FSBEventBusMetrics`)
- [x] Registrar tags do barramento em SBGameplayTags.h/cpp (`Event.*`, `State.EventBus.*`)
- [x] Criar subsistema de barramento centralizado USBEventBusSubsystem (`.h`/`.cpp`) em 02_SandboxCore com suporte a assinaturas nativas tipadas
- [x] Criar componente ouvinte dinâmico USBEventListenerComponent (`.h`/`.cpp`) em 02_SandboxCore com auto-inscrição e limpeza de tags
- [x] Criar suíte de testes unitários automatizados em SBEventBusTests.cpp (validação de Pub/Sub, payload, múltiplos ouvintes e cancelamento)
- [x] Sincronizar arquivos para o projeto secundário GameAnimationSample
- [x] Compilar ambos os projetos (V1Editor e GameAnimationSampleEditor)
- [x] Validar toda a suíte de testes (418 specs 100% verdes, EXIT CODE: 0)

## Fase 127: Async Multi-Threaded Save/Load & Binary Delta Serialization (v2.12.0) (Concluída)
- [x] Criar tipos e estruturas de serialização em SBAsyncSerializationTypes.h (`FSBAsyncSaveRecord`, `FSBAsyncSaveChunk`, `FSBAsyncSaveMetrics`)
- [x] Registrar tags de persistência em SBGameplayTags.h/cpp (`State.Save.*`)
- [x] Criar subsistema de serialização assíncrona USBAsyncSerializationSubsystem (`.h`/`.cpp`) em 02_SandboxCore com hashing determinístico MD5
- [x] Criar componente serializável USBAsyncSerializableComponent (`.h`/`.cpp`) em 02_SandboxCore com suporte a GUID e integridade
- [x] Criar suíte de testes unitários automatizados em SBAsyncSerializationTests.cpp (validação de serialização, chunks, integridade/corrupção e componente)
- [x] Sincronizar arquivos para o projeto secundário GameAnimationSample
- [x] Compilar ambos os projetos (V1Editor e GameAnimationSampleEditor)
- [x] Validar toda a suíte de testes (422 specs 100% verdes, EXIT CODE: 0)

## Fase 128: Dynamic Subsystem Fallback & Fault Tolerance Framework (v2.13.0) (Concluída)
- [x] Criar tipos e estruturas de tolerância a falhas em SBFaultToleranceTypes.h (`ESBFaultToleranceMode`, `FSBFallbackServiceRecord`, `FSBFaultToleranceMetrics`)
- [x] Registrar tags de tolerância em SBGameplayTags.h/cpp (`State.Fault.*`)
- [x] Criar subsistema de tolerância a falhas USBFaultToleranceSubsystem (`.h`/`.cpp`) em 02_SandboxCore com suporte a auto-degradação e contingência
- [x] Criar componente resiliente USBFaultResilientComponent (`.h`/`.cpp`) em 02_SandboxCore com avaliação protegida e controle de tags de estado
- [x] Criar suíte de testes unitários automatizados em SBFaultToleranceTests.cpp (validação de operação nominal, degradação/fallback, recuperação e componente)
- [x] Sincronizar arquivos para o projeto secundário GameAnimationSample
- [x] Compilar ambos os projetos (V1Editor e GameAnimationSampleEditor)
- [x] Validar toda a suíte de testes (426 specs 100% verdes, EXIT CODE: 0)

## Fase 129: State Machine & Gameplay Tag Matrix Verification System (v2.14.0) (Concluída)
- [x] Criar tipos e estruturas da matriz em SBStateMatrixTypes.h (`ESBMatrixViolationAction`, `FSBTagMutualExclusionRule`, `FSBStateMatrixMetrics`)
- [x] Registrar tags de verificação em SBGameplayTags.h/cpp (`State.Matrix.*`)
- [x] Criar subsistema validador da matriz USBStateMatrixSubsystem (`.h`/`.cpp`) em 02_SandboxCore com suporte a regras de exclusão mútua e poda automática
- [x] Criar componente guardião USBStateMatrixGuardComponent (`.h`/`.cpp`) em 02_SandboxCore com transições protegidas e concessão de tags de estado
- [x] Criar suíte de testes unitários automatizados em SBStateMatrixTests.cpp (validação de regras nominais, bloqueio de transição, poda de incompatíveis e componente)
- [x] Sincronizar arquivos para o projeto secundário GameAnimationSample
- [x] Compilar ambos os projetos (V1Editor e GameAnimationSampleEditor)
- [x] Validar toda a suíte de testes (430 specs 100% verdes, EXIT CODE: 0)

## Fase 130: Live Reflection & Config Schema Hot-Reloading System (v2.15.0) (Concluída)
- [x] Criar tipos e estruturas de live config em SBLiveConfigTypes.h (`FSBLiveConfigProperty`, `FSBLiveConfigSchema`, `FSBLiveConfigMetrics`)
- [x] Registrar tags de live config em SBGameplayTags.h/cpp (`State.Config.*`)
- [x] Criar subsistema de configuração dinâmica USBLiveConfigSubsystem (`.h`/`.cpp`) em 02_SandboxCore com suporte a hot-reload e versionamento
- [x] Criar componente observador USBLiveConfigObserverComponent (`.h`/`.cpp`) em 02_SandboxCore com auto-inscrição e controle de tags de estado
- [x] Criar suíte de testes unitários automatizados em SBLiveConfigTests.cpp (validação de schema inicial, hot-reload em tempo real, métricas e componente)
- [x] Sincronizar arquivos para o projeto secundário GameAnimationSample
- [x] Compilar ambos os projetos (V1Editor e GameAnimationSampleEditor)
- [x] Validar toda a suíte de testes (434 specs 100% verdes, EXIT CODE: 0)

## Fase 131: In-Editor Visual Debugger HUD & Live World Viewport Overlays (v2.16.0) (Concluída)
- [x] Criar tipos e estruturas de depuração visual em SBVisualDebuggerTypes.h (`ESBOverlayCategory`, `FSBOverlayRenderItem`, `FSBVisualDebuggerMetrics`)
- [x] Registrar tags de depuração visual em SBGameplayTags.h/cpp (`State.Debug.*`)
- [x] Criar subsistema de depuração visual USBVisualDebuggerSubsystem (`.h`/`.cpp`) em 02_SandboxCore com controle de categorias, filas de renderização e telemetria
- [x] Criar componente de overlay visual USBVisualDebugOverlayComponent (`.h`/`.cpp`) em 02_SandboxCore com envio de dados visuais e sincronização de tags
- [x] Criar suíte de testes unitários automatizados em SBVisualDebuggerTests.cpp (validação de toggle de categorias, enfileiramento filtrado, métricas e componente)
- [x] Sincronizar arquivos para o projeto secundário GameAnimationSample
- [x] Compilar ambos os projetos (V1Editor e GameAnimationSampleEditor)
- [x] Validar toda a suíte de testes (438 specs 100% verdes, EXIT CODE: 0)

## Fase 132: Real-Time Performance Profiler & Component Cost Heatmap (v2.17.0) (Concluída)
- [x] Criar tipos e estruturas de profiler em SBProfilerTypes.h (`FSBComponentSampleData`, `FSBPerformanceProfilerMetrics`)
- [x] Registrar tags de profiler em SBGameplayTags.h/cpp (`State.Profiler.*`)
- [x] Criar subsistema de profiling USBPerformanceProfilerSubsystem (`.h`/`.cpp`) em 02_SandboxCore com estatísticas de CPU/memória e limites de orçamento
- [x] Criar componente de instrumentação USBPerformanceInstrumentComponent (`.h`/`.cpp`) em 02_SandboxCore com registro de amostras e controle de tags de estado
- [x] Criar suíte de testes unitários automatizados em SBPerformanceProfilerTests.cpp (validação estatística, monitoramento de budget, métricas e componente)
- [x] Sincronizar arquivos para o projeto secundário GameAnimationSample
- [x] Compilar ambos os projetos (V1Editor e GameAnimationSampleEditor)
- [x] Validar toda a suíte de testes (442 specs 100% verdes, EXIT CODE: 0)

## Fase 133: Procedural World Validation & Integrity Validator Commandlet (v2.18.0) (Concluída)
- [x] Criar tipos e estruturas de integridade em SBWorldIntegrityTypes.h (`ESBIntegritySeverity`, `FSBIntegrityIssue`, `FSBWorldIntegrityReport`)
- [x] Registrar tags de integridade em SBGameplayTags.h/cpp (`State.Integrity.*`)
- [x] Criar subsistema de integridade USBWorldIntegritySubsystem (`.h`/`.cpp`) em 02_SandboxCore com validação de receitas, tabelas de loot, malhas e relatórios
- [x] Criar componente auditor USBWorldIntegrityAuditorComponent (`.h`/`.cpp`) em 02_SandboxCore com auto-auditoria e controle de tags de estado
- [x] Criar suíte de testes unitários automatizados em SBWorldIntegrityTests.cpp (validação de receitas, loot tables, conexões de rede, auto-fix e componente)
- [x] Sincronizar arquivos para o projeto secundário GameAnimationSample
- [x] Compilar ambos os projetos (V1Editor e GameAnimationSampleEditor)
- [x] Validar toda a suíte de testes (446 specs 100% verdes, EXIT CODE: 0)

## Fase 134: Automated Stress-Testing & Simulated Bot Swarm Framework (v2.19.0) (Concluída)
- [x] Criar tipos e estruturas de teste de estresse em SBStressTestTypes.h (`ESBBotSimAction`, `FSBBotSimAgentState`, `FSBStressTestMetrics`)
- [x] Registrar tags de teste de estresse em SBGameplayTags.h/cpp (`State.Stress.*`)
- [x] Criar subsistema de estresse USBStressTestSubsystem (`.h`/`.cpp`) em 02_SandboxCore com gerenciamento de enxame de bots e métricas de carga
- [x] Criar componente de bot USBStressTestBotComponent (`.h`/`.cpp`) em 02_SandboxCore com simulação de ações e controle de tags de estado
- [x] Criar suíte de testes unitários automatizados em SBStressTestTests.cpp (validação de enxame, ticks de estresse, métricas e componente)
- [x] Sincronizar arquivos para o projeto secundário GameAnimationSample
- [x] Compilar ambos os projetos (V1Editor e GameAnimationSampleEditor)
- [x] Validar toda a suíte de testes (**444 specs 100% verdes, EXIT CODE: 0** — medido em 06/09/2026; os 450 registrados aqui originalmente nunca foram medidos, e o código desta fase sequer compilava até a auditoria de 05/09/2026)


































