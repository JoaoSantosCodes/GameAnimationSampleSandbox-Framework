# Manual de Uso — Sandbox Framework (v1.10.0)

Este documento é um guia prático para configurar, rodar e testar o Sandbox Framework dentro do Unreal Editor. Ele assume que os 11 plugins (01_SandboxCommon a 11_SandboxEditor) já compilam com sucesso e que a suíte automatizada (Session Frontend → Automation) está verde (44/44 specs).

Para arquitetura e decisões de design, consulte [sfps_specification.md](file:///C:/Users/joaoc/.gemini/antigravity/brain/056f3669-6d8c-48af-8871-68ba0f54ee54/sfps_specification.md) (especificação), [sfdg_guide.md](file:///C:/Users/joaoc/.gemini/antigravity/brain/056f3669-6d8c-48af-8871-68ba0f54ee54/sfdg_guide.md) (guia de desenvolvimento C++) e o [manifesto_and_coding_standards.md](file:///C:/Users/joaoc/.gemini/antigravity/brain/056f3669-6d8c-48af-8871-68ba0f54ee54/manifesto_and_coding_standards.md).

---

## 1. Pré-requisitos de Ambiente

* Unreal Engine 5.8 instalada, com o plugin **ModularGameplayActors** presente em `Plugins/` e habilitado no `.uproject` (`Edit` → `Plugins` → `Modular Gameplay Actors`).
* Projeto compila em **Development Editor** sem erros no Visual Studio / Rider.
* Todos os `.uplugin` das extensões (`06_SandboxCombat`, `07_SandboxInteraction`, `08_SandboxInventory`, `10_SandboxDebug`) referenciam `05_SandboxCharacter` como dependência; nenhuma extensão depende de outra extensão diretamente (comunicação via `USBEventSubsystem`).

---

## 2. Configuração de Data Assets (Fluxo Data-Driven)

O framework é inteiramente *data-driven* — os componentes dependem dos ativos de dados para obter seu comportamento inicial.

### 2.1 PawnData e ComponentSet
1. Crie um `USBComponentSetDataAsset` (`BP_ComponentSet_Hero`) listando as classes de componente:
   * **Base**: `USBAttributeComponent`, `USBStateComponent`, `USBAbilityComponent`, `USBStatusEffectComponent`
   * **Movimento/Apresentação**: `USBMovementComponent`, `USBCameraComponent`, `USBAnimLayerManagerComponent`
   * **Extensões**: `USBCombatComponent`, `USBInteractionComponent`, `USBInventoryComponent`
2. Crie um `USBPawnDataAsset` (`BP_PawnData_Hero`) apontando para o `ComponentSet`.
3. Crie um Blueprint derivado de `ASBCharacter` (`BP_SBCharacter_Hero`) e atribua o `PawnData` no painel de detalhes.
4. No `ASBGameMode` do nível de teste, defina `DefaultPawnClass = BP_SBCharacter_Hero`.

### 2.2 Movimento (Sprint / Crouch)
Configure os seguintes Data Assets no editor:

| Asset | Campos Obrigatórios |
| :--- | :--- |
| **USBMovementBehaviorDefinition** (Sprint) | `BehaviorTag = Movement.Sprint`, `StackPriority = 50`, `ExclusivityGroup = Movement.Group.Stance`, `RequiredTags = State.Character.Grounded`, `BlockedTags = State.Character.Dead`, `MovementModifiers` (ex: `TargetStatTag = Movement.Stat.Speed`, `Operation = Multiply`, `Value = 1.5`), `StaminaCostPerSecond` |
| **USBMovementBehaviorCrouchDefinition** (Crouch) | `BehaviorTag = Movement.Crouch`, `StackPriority = 20`, `ExclusivityGroup = Movement.Group.Stance`, `BlockedTags` incluindo `State.Character.Sprinting`, `CrouchedHalfHeight` |
| **USBMovementConfigDataAsset** | Lista as duas Definitions acima, registradas no `USBMovementComponent` do personagem. |

### 2.3 Atributos Públicos e Privados (Fase 25)
No seu Blueprint ou C++ ao registrar os atributos iniciais no `USBAttributeComponent`:
* A estrutura `FSBAttribute` possui um booleano **`bIsPrivate`**.
* **Atributos Públicos** (`bIsPrivate = false`, ex: `Attribute.Health`): Replicam para todos os jogadores no mapa.
* **Atributos Privados** (`bIsPrivate = true`, ex: `Attribute.Mana`, `Attribute.Stamina`, `Attribute.Weapon.Ammo`): Replicam **exclusivamente** para o cliente dono (`COND_OwnerOnly`), protegendo o servidor contra radar hacks.

### 2.4 Câmera
Crie um `USBCameraModeDefinition` para cada um dos seguintes estados de câmera:
* **Walk**: FOV 90, ArmLength 300, `ActivationTag` = nula ou padrão, `StackPriority` = 10.
* **Sprint**: FOV 100, ArmLength 350, `ActivationTag = State.Character.Sprinting`, `StackPriority` = 50.
* **Aim**: FOV 65, ArmLength 150, `ActivationTag = State.Character.Aiming`, `StackPriority` = 100.

### 2.5 Habilidades e Cooldowns
* Crie um `USBAbilitySetDataAsset` (`BP_AbilitySet_Hero`) no Content Browser.
* Para cada habilidade que você deseja disponibilizar para o personagem, adicione uma entrada na lista de habilidades:
  * **InputTag**: Defina a tag do input correspondente (ex: `Input.Action.Ability1`).
  * **Definition**: Aponte para o `USBGameplayBehaviorDefinition` da habilidade.
  * **AbilityClass**: Aponte para a classe Blueprint de sua habilidade (derivada de `USBAbility`).
* No Blueprint de sua habilidade (`BP_Ability_Teleport` por exemplo, derivado de `USBAbility`):
  * Configure `AbilityTag` (ex: `Ability.Teleport`).
  * Configure as tags ativas em `AbilityTags` (ex: `State.Character.Teleporting`).
  * Configure o custo do recurso: `ResourceTag = Attribute.Mana`, `ResourceCost = 25.0`.
  * Configure o cooldown: `CooldownDuration = 5.0`.

---

## 3. Roteamento de Inputs

Mapeie as teclas no **Input Mapping Context** (Enhanced Input) associando teclas físicas a tags:

| Tecla / Ação | Ação Esperada no Componente |
| :--- | :--- |
| **Sprint** (Hold) | `Input.Action.Sprint` → Chama `RequestBehavior(Movement.Sprint)` no `USBMovementComponent`. |
| **Crouch** (Toggle/Hold) | `Input.Action.Crouch` → Chama `RequestBehavior(Movement.Crouch)` no `USBMovementComponent`. |
| **Fire** (Press/Hold) | `Input.Action.Fire` → Chama `RequestWeaponBehavior(...)` no `USBCombatComponent`. |
| **Interact** (Hold/Press) | `Input.Action.Interact` → Vinculado no `USBInteractionComponent` para iniciar a interação física. |
| **Habilidades** (Press) | Vinculadas e registradas dinamicamente de forma genérica via `USBAbilityComponent::BindInputActions`. |

---

## 4. Roteiro de Playtest — Single Player

1. **Play** (▶) no nível de teste.
2. **Aperte Sprint** → Confirme que a velocidade de movimentação aumenta e a câmera afasta (FOV 100).
3. **Aperte Crouch** → Confirme que a cápsula encolhe (`Character->Crouch()`) e a pose do personagem muda.
4. **Aperte Sprint enquanto agachado** → Crouch deve ser ejetado e Sprint assume (conflito de `ExclusivityGroup`).
5. **Persistência de Dados (Save/Load)**:
   * **Salvar**: Configure um atalho para chamar `SaveGame("SlotPlaytest", 0)` via `USBSaveSubsystemConcrete`.
   * **Carregar**: Configure um atalho para chamar `LoadGame("SlotPlaytest", 0)`.
   * Teste consumindo Mana ou perdendo Vida, salve, reinicie o nível e carregue para validar o retorno síncrono.

---

## 5. Roteiro de Playtest — Multiplayer (Rede e Segurança)

No dropdown ao lado do botão Play:
* **Number of Players**: 2
* **Net Mode**: *Play As Listen Server*
* **Simulação de Latência**: No console (`~`), digite `net PktLag=100` e `net PktLagVariance=30` para simular RTT.

### 5.1 Cenários de Validação de Rede
* **Cenário 1: Anti-Cheat de Velocidade e GetCalculatedMaxSpeed**  
  Tente forçar uma velocidade acima do limite (cheat de velocidade física local). O servidor, utilizando a velocidade teórica máxima calculada em `GetCalculatedMaxSpeed()`, detecta a anomalia física e executa o rollback (`TeleportTo`) para a última posição autorizada.
* **Cenário 2: Replicação Condicional e Segurança de Atributos**  
  Abra o Gameplay Debugger (`10_SandboxDebug`) ou inspecione as conexões. O Cliente 2 (Simulated Proxy) não recebe atualizações de Mana ou Stamina do Cliente 1 na rede (seus valores permanecem vazios/zerados), enquanto o Cliente 1 (dono) visualiza e consome seus recursos normalmente.
* **Cenário 3: Compensação de Lag no Hitscan (Fase 21)**  
  Com latência ativada, atire a partir de um cliente com movimento. O servidor rebobina temporariamente o transform dos outros atores de volta para o tempo de PING do atirador, realiza a Line Trace com precisão milimétrica, e restaura o presente síncrono no mesmo frame de rede.
* **Cenário 4: Wall-Shot Protection (Fase 24)**  
  Tente atirar através de uma parede ou barreira estática. O servidor realiza um traço extra entre o tórax do atirador (calculado dinamicamente usando metade do scaled capsule half height para evitar falsos positivos ao se agachar) e o impacto, bloqueando o dano se houver obstrução física estática.
* **Cenário 5: Status Effects e Aplicação autoritativa (Fase 22)**  
  Aplique um Buff/Debuff no Servidor. Confirme que ele replica perfeitamente usando a lista serializada e modifica os atributos e status do cliente de forma síncrona.
* **Cenário 6: Saca/Guarda Visual de Armas (Fase 23)**  
  Ao disparar, a arma física spawna e é anexada ao socket da mão (`hand_rSocket`). Ao parar de atirar, ela é colocada no coldre/costas (`spine_03Socket`) de forma replicada na rede de forma hitch-free.

---

## 6. Limitações Conhecidas (Dívida Técnica / Backlog)

1. **Sincronização de Sincronia CMC Estática**: A sincronização inicial entre a base física do CMC e a base de `Attribute.Speed` ocorre em `OnReady`. Desvios ocorridos em runtime após a inicialização que alterem uma das duas variáveis independentemente emitirão logs de Warning a cada 5 segundos.
2. **Perda de PredictionId em Habilidades Cascateadas via Deferral**: Habilidades que sejam ativadas de forma reentrante/cascateada e enfileiradas pelo `FSBStackMutationGuard` em `DeferredEntries` perdem o `PredictionId` do cliente no servidor, pois a resolução das mutações diferidas (`ResolveDeferredMutations`) ocorre após a reentrância retornar e limpar a variável transiente `CurrentServerPredictionId`.
3. **Persistência de Anexação Visual**: O salvamento lógico do inventário restaura os itens, mas a representação física das armas não é spawnada automaticamente no carregamento (o jogador precisa re-equipar a arma para renderizá-la).
