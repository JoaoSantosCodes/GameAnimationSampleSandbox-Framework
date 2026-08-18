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

## Fase 32: Dano Crítico, Resistências e Reações de Impacto Replicadas (Pendente)
- [ ] Implementar leitura de `HitResult.BoneName` para multiplicar danos no `USBWeaponBehaviorHitscan`
- [ ] Adicionar mitigação baseada no atributo `Attribute.Defense` no `USBAttributeComponent`
- [ ] Integrar montagem de impacto síncrona forçando a tag `State.Character.HitReacting` predita e replicada
- [ ] Escrever suíte de testes `SBCriticalDamageTests.cpp` validando multiplicadores, mitigação e impactos

## Fase 33: Tabela de Loot e Drop Físico Replicado (Pendente)
- [ ] Criar classe `ASBPhysicalLootDrop` com simulação física e componente interativo `USBInteractionComponent`
- [ ] Criar Data Asset `USBLootTableDataAsset` mapeando itens lógicos e percentuais de raridade
- [ ] Implementar spawn seguro no servidor ao tocar o solo e integração com inventário lúdico
- [ ] Escrever suíte de testes `SBLootDropTests.cpp` validando probabilidade e coletas físicas

---

## ✅ Refatoração Pós-Fase 31 (2026-08-18) — Não Requer Nova Fase

As seguintes melhorias foram implementadas e validadas contra o Manifesto, prontas para commit:

| Item | Arquivos | Princípio do Manifesto |
|------|----------|------------------------|
| `SBMovementConfigDataAsset`: `FSBStaminaConfig` + `FSBAntiCheatConfig` | `SBMovementConfigDataAsset.h`, `SBMovementComponent.cpp/.h` | **#2 Orientação a Dados**, **#10 C++ Core BP Configurable** |
| `SBStatusEffectComponent`: Herda `UGameFrameworkComponent` + 3 interfaces + Save/Load/Debug | `SBStatusEffectComponent.cpp/.h` | **#4 Desacoplamento por Interfaces**, **#8 Multiplayer Ready**, **#10 C++ Core** |
| `SBInventoryComponent`: `EquippableFragmentClass` configurável | `SBInventoryComponent.cpp/.h` | **#4 Desacoplamento**, **#5 Injeção Dinâmica** |
| `SBStatusHUDWidget`: `FSBGameplayTags` singleton | `SBStatusHUDWidget.cpp` | **#2 Orientação a Dados**, **#10 C++ Core** |
| `LoadMovementConfig`: `AddUnique` idempotente | `SBMovementComponent.cpp` | **Qualidade** |
