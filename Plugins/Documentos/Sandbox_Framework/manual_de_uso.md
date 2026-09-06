# Manual de Uso — Sandbox Framework (v1.29.0)

Este documento é um guia prático para configurar, rodar e testar o Sandbox Framework dentro do Unreal Editor. Ele assume que os 11 plugins (01_SandboxCommon a 11_SandboxEditor) já compilam com sucesso e que a suíte automatizada (Session Frontend → Automation) está verde (95/95 specs).

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

### 2.6 Persistência Criptografada (Anti-Save Scumming - v1.26.0)
A persistência do Sandbox é encriptada nativamente via cifras XOR dinâmicas com verificação por assinatura de integridade HMAC-MD5 para evitar trapaças:
* **Configuração da Chave**: No arquivo `Config/DefaultGame.ini` do seu projeto, adicione:
  ```ini
  [/Script/SandboxCommon.SBDeveloperSettings]
  SaveEncryptionKey="SuaChaveSecretaCustomizadaAqui"
  ```
  *(Se nenhuma chave for configurada, o sistema usará o salt estático padrão do C++)*
* **Comportamento de Trapaça**: Se o arquivo `.sav` no disco for modificado ou a assinatura for apagada/adulterada, o subsistema aborta o carregamento e gera um log `SECURITY WARNING` informando sobre a violação de integridade.

### 2.7 Som de Passos Dinâmico (Surface-Aware Footsteps - v1.27.0)
* Crie um `USBSurfaceEffectsDataAsset` (`BP_SurfaceEffects_Default`) no Content Browser.
* Adicione mapeamentos no painel Details mapeando chaves de `EPhysicalSurface` (ex: `SurfaceType1` para Grama, `SurfaceType2` para Concreto) para configurações de `Sound` e/ou `VisualEffect`.
* Abra a animação do personagem (Anim Sequence) e, nos trilhos de notificação (Notifies), adicione a notificação **`USBAnimNotify_Footstep`** (Sandbox Footstep Notify).
* Configure a notificação:
  * **FootSocketName**: Nome do osso/socket do pé correspondente (ex: `Foot_L` ou `Foot_R`).
  * **SurfaceEffectsConfig**: Aponte para o seu `BP_SurfaceEffects_Default` recém-criado.
* **Escuta de Efeitos Visuais**: O AnimNotify emite o evento `Event.Character.Footstep` no `USBEventSubsystem` com a posição e o tipo de material detectado. Designers podem criar Blueprints ouvintes para spawna Niagara VFX de respingos ou poeira com desacoplamento absoluto.

### 2.8 Zonas de Áudio Ambiental (Ambient Zones - v1.27.0)
* Arraste a classe **`SBAmbientZoneTrigger`** do painel de classes/Atores para o nível.
* Posicione e escale a `TriggerBox` para delimitar a zona de áudio ambiente (ex: interior de uma caverna).
* No painel Details, configure:
  * **AmbientSound**: O arquivo looping de som ambiente a tocar (ex: barulho de goteiras e eco).
  * **FadeInDuration**: Tempo em segundos para a trilha entrar suavemente (ex: `1.5s`).
  * **FadeOutDuration**: Tempo em segundos para a trilha sumir ao sair da zona (ex: `1.5s`).
  * **VolumeMultiplier**: Multiplicador de volume.
* O trigger funciona de forma 100% otimizada no cliente, detectando apenas overlaps do jogador controlado localmente (`IsLocallyControlled`).

### 2.9 Missões e Objetivos (Quests & Objectives - v1.28.0)
* **USBQuestDataAsset**: Ativo de dados que define a estrutura de uma missão.
  * **QuestName**: Título amigável da missão.
  * **Objectives**: Array de `FSBQuestObjective` (cada um com uma tag de objetivo, ex: `Quest.Objective.Footstep`, e contagem requerida).
  * **Rewards**: Array de `FSBQuestReward` (XP a conceder, soft pointers para itens físicos e quantidades).
* **USBQuestComponent**: Componente que deve ser anexado ao personagem. O componente:
  * Replica o progresso das missões ativas na rede de forma autoritativa.
  * Escuta eventos do `USBEventSubsystem` e incrementa o progresso de forma dinâmica e síncrona.
  * Concede as recompensas de XP e dispara o evento `Event.Quest.RewardsClaimed` para o `USBInventoryComponent` spawnar os itens físicos de recompensa de forma desacoplada.

### 2.10 Comerciante e Transações (Merchant & Trading - v1.28.0)
* **USBMerchantComponent**: Componente que deve ser anexado a um ator comerciante no mapa.
  * **AvailableItems**: Mapa contendo soft/hard pointers para `USBItemDefinition` e o preço correspondente em Coins (moedas).
  * **BuyPriceMultiplier / SellPriceMultiplier**: Multiplicadores para personalizar a economia local.
  * **RPCs Autoritativos**: A compra e venda de itens é processada via Server RPCs (`ServerBuyItem` e `ServerSellItem`) no servidor, verificando o saldo do atributo privado `Attribute.Coins` do cliente, espaço de inventário e proximidade física com o NPC (limite de 400 unidades).

### 2.11 Construção e Edificação (Building & Construction - v1.29.0)
* **USBItemFragment_Placeable**: Fragmento de item criado na definição do item (`USBItemDefinition`) no Content Browser.
  * **BuildingPieceClass**: Aponta para a subclasse Blueprint de `ASBBuildingPiece` correspondente à peça física.
* **ASBBuildingPiece**: Classe base para peças físicas de edificação (paredes, tetos, portas).
  * **Health / MaxHealth**: HP da peça (padrão = 100.f), replicada para todos. Ao sofrer danos e atingir zero, ela se destrói automaticamente no servidor.
  * **SetPreviewMode()**: Helper C++ que ajusta a colisão local e transparência visual da peça ao ser instanciada como pré-visualização translúcida.
* **USBBuildingComponent**: Componente anexado ao personagem para gerenciar a mecânica:
  * **Snapping**: Grade de coordenadas determinística automática em passos de `200` unidades em X/Y e `100` em Z.
  * **Server RPC**: RPC autoritativo `ServerPlaceBuildingPiece` que efetua validações críticas no servidor: proximidade física limite do jogador (`600` unidades), sobreposição física (overlap) contra barreiras estáticas do mundo e consumo atômico do item de inventário.

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
6. **Mecânica de Comércio (Merchant)**:
   * Coloque o NPC com o componente `USBMerchantComponent` próximo ao spawn do player.
   * Chame o RPC `ServerBuyItem` passando a definição do item. Verifique que seu saldo de moedas diminui em `30` e o item é adicionado ao seu inventário.
   * Afaste-se do NPC por mais de 400 unidades e tente comprar novamente. O servidor deve rejeitar a transação de forma silenciosa e segura.
7. **Progressão de Missões (Quests)**:
   * Aceite uma quest que exige passos. Caminhe para disparar o `USBAnimNotify_Footstep`.
   * Cada som de passo publicará o evento e incrementará o progresso de sua quest ativa. Ao concluir a quest, o XP será adicionado e as recompensas em itens serão enviadas diretamente ao seu inventário.
8. **Mecânica de Construção (Building)**:
   * Adicione o item placeable (com o fragmento `USBItemFragment_Placeable` associando uma classe física como `BP_ASBBuildingPiece`) ao inventário.
   * Chame `StartPlacement` passando a classe da peça. Mova a mira e confirme que o preview translúcido acompanha sua mira no plano 3D aplicando snapping determinístico (múltiplos de 200/100).
   * Chame `RequestPlaceActivePiece`. A peça física replicada é instanciada e o item de inventário correspondente é consumido.

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
* **Cenário 7: Construção e Colisão de Edificações (Fase 44)**  
  Tente posicionar uma peça de construção que sobreponha ou atravesse uma parede física estática no mapa. O servidor detecta o overlap, bloqueia a criação e mantém o item no seu inventário intacto. Afaste-se por mais de 600 unidades da peça de preview e tente colocá-la. O servidor bloqueia por violação de distância limite (anti-cheat).

---

## 6. Ferramentas de Automação de Assets (Scripted Asset Actions - v1.39.0)

O framework disponibiliza um conjunto de automações nativas em C++ que estendem as ações do **Content Browser** para agilizar a criação e padronização dos assets conforme as diretrizes do projeto:

### 6.1 Como Usar no Editor
1. Navegue até a pasta de destino no **Content Browser** onde deseja criar o asset.
2. Clique com o botão direito em **qualquer** asset ou pasta.
3. Vá no menu **Scripted Asset Actions** -> **Sandbox Tools** e escolha uma das opções:
   * **Create Sandbox Character**: Digite o nome do personagem (ex: `Guerreiro`). O sistema criará uma pasta com o nome especificado contendo:
     * `CS_Guerreiro` (ComponentSet pré-configurado com os 10 componentes padrão).
     * `PD_Guerreiro` (Pawn Data referenciando o ComponentSet).
     * `BP_Guerreiro` (Blueprint herdando de `ASBCharacter` com o PawnData já preenchido em seu CDO).
   * **Create Sandbox Weapon**: Digite o nome da arma e selecione se ela utiliza projétil ou hitscan. O sistema criará:
     * `DA_NomeDaArma` (Weapon Definition com valores padrão).
     * `BP_NomeDaArma` (Blueprint herdando de `USBWeaponBehaviorHitscan` ou `USBWeaponBehaviorProjectile` referenciando a definição).
   * **Create Sandbox Ability**: Digite o nome da habilidade. O sistema gerará:
     * `DA_NomeDaHabilidade` (Gameplay Behavior Definition).
     * `BP_NomeDaHabilidade` (Blueprint herdando de `USBAbility` pré-configurado com cooldown de 2.0s e custo de 10 de Mana).
   * **Create Sandbox Quest**: Digite o nome da missão. O sistema criará:
     * `DA_NomeDaQuest` (Quest Data Asset estruturado com um objetivo de passos padrão).

### 6.2 Vantagens da Automação Nativa C++
* **Padronização Estrita**: Impede erros manuais de nomenclatura (prefixos `BP_`, `DA_`, `CS_`, `PD_`) e evita o esquecimento de vincular referências cruzadas ou injetar componentes necessários.
* **Salvamento Automático**: Todos os assets gerados são notificados ao Asset Registry, marcados como salvos e gravados no disco de forma atômica no mesmo frame.

### 6.3 Descoberta de Áreas e Apresentação (v1.49.0)
Implementamos um sistema modular para detectar e notificar a descoberta de territórios ou áreas do jogo:
* **Trigger Físico (`ASBAmbientZoneTrigger`)**:
  * Adicione um `SBAmbientZoneTrigger` no mapa e ajuste seu tamanho via `TriggerBox`.
  * Habilite `bEnableAreaDiscovery`.
  * Preencha o **AreaName** (ex: "Vale do Eco") e **AreaDescription** (ex: "Território Proibido").
  * Opcionalmente, configure uma cutscene no campo **PresentationSequence** (como uma Level Sequence de câmera) e defina se ela deve rodar apenas uma vez (`bTriggerSequenceOnlyOnce`).
* **Acoplamento Descentralizado via Eventos**:
  * Ao entrar na área, o trigger publica um evento global com a tag `Event.Area.Discovered` enviando um payload do tipo `USBAreaDiscoveryPayload`.
  * Crie um Widget Blueprint que escuta (`SubscribeToEvent`) essa tag e exibe um letreiro animado com o nome e a descrição da área.
  * Crie um listener para tocar a cutscene da câmera caso `bIsFirstDiscovery` seja verdadeiro.

### 6.4 Feature & Capability Management (v1.50.0)
O Sandbox v1.50.0 introduz o controle reativo e modular de Features de Gameplay a nível global:
* **Gameplay Tags de Feature**:
  * Use tags hierárquicas como `Feature.Combat`, `Feature.Inventory`, `Feature.Crafting`, etc., para modularizar a visibilidade de sistemas.
* **Subsistema Global (`USBSandboxFeatureSubsystem`)**:
  * Controle o estado ativo de features em C++ ou Blueprints:
    * `IsFeatureEnabled(FeatureTag)`: Consulta se a flag está ativa.
    * `EnableFeature(FeatureTag)`: Ativa e dispara as notificações.
    * `DisableFeature(FeatureTag)`: Desativa e dispara as notificações.
* **Escuta Desacoplada de Mudanças**:
  * Inscreva-se no delegate `OnFeatureToggled` do subsistema ou no Barramento de Eventos Global (`Event.Feature.Toggled` com o payload `USBSandboxFeaturePayload`) para mostrar/ocultar elementos de UI, habilitar habilidades ou instanciar componentes sob demanda.

---

## 7. Limitações Conhecidas (Dívida Técnica / Backlog)

1. **Sincronização de Sincronia CMC Estática**: A sincronização inicial entre a base física do CMC e a base de `Attribute.Speed` ocorre em `OnReady`. Desvios ocorridos em runtime após a inicialização que alterem uma das duas variáveis independentemente emitirão logs de Warning a cada 5 segundos.
2. **Perda de PredictionId em Habilidades Cascateadas via Deferral**: Habilidades que sejam ativadas de forma reentrante/cascateada e enfileiradas pelo `FSBStackMutationGuard` em `DeferredEntries` perdem o `PredictionId` do cliente no servidor, pois a resolução das mutações diferidas (`ResolveDeferredMutations`) ocorre após a reentrância retornar e limpar a variável transiente `CurrentServerPredictionId`.
