# Manual de Uso Detalhado - Sandbox Framework (Guia de Integração e Aula Prática)

Este manual detalhado é estruturado como uma aula técnica passo a passo. Ele foi projetado para guiar o desenvolvedor desde a configuração básica de dados até o teste físico de rede dos sistemas complexos, sem omitir etapas ou assumir pré-requisitos ocultos.

---

## 🎓 Introdução à Arquitetura do Framework

O **Sandbox Framework** baseia-se em três pilares fundamentais de engenharia de software:
1.  **Desacoplamento por Dados (Data-Driven)**: Personagens não possuem componentes ou lógica de input hardcoded. Todo o comportamento é descrito em ativos de dados.
2.  **Comunicação Indireta (Event-Driven)**: A comunicação entre sistemas (Interface, Inventário, Combate) ocorre assincronamente através de um subsistema de eventos baseado em tags, eliminando dependências circulares.
3.  **Segurança e Consistência**: Utilização de ganchos em C++ para garantir validação rígida de dados (*fail-closed*), gerenciamento de ciclo de vida seguro (auto-unsubscribe de delegates) e controle multiplayer robusto.

---

## 📝 Aula 1: Arquitetura Modular e Injeção Dinâmica (`PawnData`)

### 1.1 Conceito Teórico
Em projetos Unreal tradicionais, componentes de gameplay (como Inventário ou Combate) são instanciados diretamente no construtor do Character em C++ usando `CreateDefaultSubobject`. Isso gera acoplamento rígido.
No Sandbox Framework, o personagem é apenas um invólucro físico. Seus componentes são injetados dinamicamente na inicialização do ator pelo subsistema `USBComponentFactory` baseado em um ativo de dados chamado `USBPawnData`.

### 1.2 Passo a Passo no Editor
1.  Abra o Unreal Editor e acesse a pasta de conteúdo do seu projeto (ex: `Content/SandboxFramework`).
2.  Clique com o **botão direito** em uma área vazia do Content Browser -> selecione **Miscellaneous** -> selecione **Data Asset**.
3.  Na caixa de seleção, digite **`SBPawnData`** (ou `USBPawnData`) no campo de busca. Selecione-o e clique em **Select**.
4.  Nomeie o ativo de dados criado como `DA_HeroPawnData`.
5.  Dê um duplo clique para abrir o `DA_HeroPawnData`.
6.  No painel Details (à direita), localize a propriedade **`ComponentsToGrant`**. Esta lista define quais componentes lógicos serão injetados no personagem ao iniciar o jogo:
    *   Clique no botão **`+`** (Add Element) para adicionar uma linha. Na caixa de seleção, selecione **`SBMovementComponent`** (responsável por locomoção física predita e controle de sprint/crouch).
    *   Adicione outra linha e selecione **`SBAttributeComponent`** (gerencia os atributos de Vida, Mana e regeneração).
    *   Adicione outra linha e selecione **`SBInteractionComponent`** (gerencia prompts de interação física de foco e hold).
    *   Adicione outra linha e selecione **`SBInventoryComponent`** (gerencia a mochila de itens, slots de equipamento e persistência lógica).
    *   Adicione outra linha e selecione **`SBCombatComponent`** (gerencia ativação de habilidades e armas).
7.  Clique em **Save** no canto superior esquerdo e feche o ativo.

---

## 📝 Aula 2: Vinculação de Inputs Dinâmicos (Enhanced Input)

### 2.1 Passo a Passo no Editor
1.  Clique com o **botão direito** no Content Browser -> selecione **Miscellaneous** -> selecione **Data Asset**.
2.  Busque pela classe base **`SBInputConfig`** (ou `USBInputConfig`), selecione-a e clique em **Select**.
3.  Nomeie o ativo como `DA_InputConfig`.
4.  Abra o `DA_InputConfig` e localize a propriedade **`InputActions`**.
5.  Clique no botão **`+`** para adicionar um novo mapeamento de ação:
    *   **Action**: Selecione o seu InputAction correspondente (ex: `IA_Sprint` para a tecla Shift).
    *   **Input Tag**: Selecione ou crie a Gameplay Tag correspondente (ex: `Input.Action.Sprint`).
6.  Adicione outros mapeamentos essenciais (ex: `IA_Crouch` para `Input.Action.Crouch`, `IA_Interact` para `Input.Action.Interact`, e `IA_Ability1` para `Input.Action.Ability1`).
7.  Salve o ativo e feche.
8.  Abra novamente o `DA_HeroPawnData` criado na Aula 1.
9.  Procure pela propriedade **`InputConfig`** no painel Details e aponte para o seu `DA_InputConfig` recém-criado.
10. Salve e feche.

---

## 📝 Aula 3: Criação da Entidade Física do Personagem (`BP_Character`)

### 3.1 Passo a Passo no Editor
1.  No Content Browser, clique com o **botão direito** -> selecione **Blueprint Class**.
2.  Na barra de busca de classes sob All Classes, digite **`SBCharacter`** (ou `ASBCharacter`), selecione-o e clique em **Select**.
3.  Nomeie o Blueprint gerado como `BP_SBCharacter_Hero`.
4.  Abra o `BP_SBCharacter_Hero`.
5.  No painel Details, localize a propriedade **`PawnData`** e selecione o seu ativo `DA_HeroPawnData`.
6.  Selecione o componente **`Mesh`** na barra lateral de componentes (canto superior esquerdo).
7.  No painel Details, localize a seção **Mesh** e atribua uma Skeletal Mesh para o visual do personagem (ex: manequim padrão do projeto).
8.  Alinhe a malha física e a rotação dentro da cápsula de colisão conforme os padrões do seu projeto de animações.
9.  Clique no botão **Compile** no canto superior esquerdo e, em seguida, em **Save**. Feche a janela.
10. Crie um Blueprint para o seu GameMode (herdando de `GameModeBase` ou `SBGameMode`).
11. Nas propriedades do GameMode, configure **Default Pawn Class** como `BP_SBCharacter_Hero` e **HUD Class** como a classe C++ `SBHUD` (ou derivada).
12. Configure seu nível ou projeto para utilizar esse GameMode.

---

## 📝 Aula 4: Criação de Interface Dinâmica Reativa (UMG)

Esta aula descreve como criar interfaces de usuário dinâmicas ligadas diretamente ao barramento de eventos assíncronos do C++, utilizando a técnica de *BindWidget* para fiação automática e *fail-closed guards* para robustez.

### 4.1 Barra de Status de Vida e Mana (`WBP_StatusHUD`)
1.  No Content Browser, clique com o **botão direito** -> **User Interface** -> **Widget Blueprint** -> Escolha **Common User Widget** (User Widget padrão) e nomeie como `WBP_StatusHUD`.
2.  Abra o `WBP_StatusHUD` para edição.
3.  No painel superior direito, clique em **Class Settings**.
4.  No painel Details à esquerda, encontre a propriedade **Parent Class** e mude de `UserWidget` para **`USBStatusHUDWidget`**.
5.  No painel Palette (canto esquerdo), arraste dois componentes **Progress Bar** para dentro do Canvas.
6.  **Regra Estrita de Nomenclatura**: No painel Hierarchy (canto inferior esquerdo), nomeie uma barra exatamente como **`PB_Health`** e a outra exatamente como **`PB_Mana`**. O C++ localiza e vincula as instâncias em runtime síncronamente usando estes IDs literais.
7.  Clique em **Compile** e **Save**.

### 4.2 Prompt de Foco e Hold de Interação (`WBP_InteractionPrompt`)
1.  Crie outro Widget Blueprint e nomeie como `WBP_InteractionPrompt`.
2.  Nas **Class Settings**, altere a **Parent Class** para **`USBInteractionPromptWidget`**.
3.  Arraste um componente **Text Block** (Bloco de Texto) e um componente **Progress Bar** para o Canvas.
4.  Nomeie o Text Block exatamente como **`TXT_Prompt`**.
5.  Nomeie a Progress Bar exatamente como **`PB_HoldProgress`**.
6.  Compile e salve.

### 4.3 Slot de Cooldown da Barra de Habilidades (`WBP_AbilitySlot`)
1.  Crie outro Widget Blueprint e nomeie como `WBP_AbilitySlot`.
2.  Nas **Class Settings**, altere a **Parent Class** para **`USBAbilityBarWidget`**.
3.  Arraste uma **Image** (Imagem escura de máscara) e um **Text Block** (Contador numérico decimal) para o Canvas.
4.  Nomeie a Image exatamente como **`IMG_CooldownMask`**.
5.  Nomeie o Text Block exatamente como **`TXT_CooldownTime`**.
6.  Selecione a raiz do Widget Blueprint. No painel Details, localize a propriedade **`WatchedAbilityTag`** e defina qual Gameplay Tag de habilidade esse slot específico deve monitorar (ex: `Ability.Teleport`).
7.  Compile e salve.

### 4.4 Visualização do Grid de Inventário (`WBP_InventoryGrid`)
1.  Crie outro Widget Blueprint e nomeie como `WBP_InventoryGrid`.
2.  Nas **Class Settings**, altere a **Parent Class** para **`USBInventoryGridWidget`**.
3.  Vá para a aba **Graph** (visualização de nós e Blueprint Graph).
4.  No painel My Blueprint lateral esquerdo, localize a seção **Interfaces** -> encontre a função **`BP_OnSlotUpdated`** -> clique com o **botão direito** e escolha **Implement Function**.
5.  O nó de evento vermelho `Event BP On Slot Updated` será criado no gráfico. O pino `ItemInstance` carrega o objeto lógico do item atualizado.
6.  Arraste o pino `ItemInstance`, solte no gráfico e adicione o nó **Cast To SBItemInstance** (ou `USBItemInstance`).
7.  A partir do sucesso do Cast, obtenha a variável **`StackCount`** (quantidade empilhada) e a variável **`ItemDef`** (definição do item contendo imagem do ícone e nome) para desenhar e atualizar os slots visuais do seu grid dinamicamente.
8.  Compile e salve.

---

## 📝 Aula 5: Persistência de Estado (Save Game System)

### 5.1 Conceito Teórico
A persistência do Sandbox grava variáveis e estados dinâmicos decorrentes de ações de gameplay. Qualquer ator ou componente que precise salvar e carregar dados assina o contrato da interface `ISBSaveInterface`.

### 5.2 Passo a Passo para Gravação e Carregamento
Durante o gameplay em execução (PIE ou standalone):
1.  Abra o console do jogo pressionando a tecla **`** (crase) ou **'** (aspas simples).
2.  Para gravar o estado completo dos componentes (como a vida atual, mana e itens no inventário) em um slot físico no disco, digite o comando de trapaça (*cheat command*):
    `ce SaveGame Slot01`
3.  Modifique o estado do jogo (tome dano, colete itens, altere sua posição no cenário).
4.  Para restaurar o estado salvo anteriormente, digite no console:
    `ce LoadGame Slot01`
5.  O subsistema de save carregará síncronamente as informações do disco, aplicando as variáveis com base na prioridade do `GetSavePriority()` de cada componente.

---

## 📝 Aula 6: Configuração de Atributos Seguros (Fase 25 - COND_OwnerOnly)

### 6.1 Configuração do bIsPrivate
1. Ao registrar atributos lógicos no `USBAttributeComponent` do seu Blueprint (ou via C++), note que a estrutura `FSBAttribute` agora possui a propriedade **`bIsPrivate`** (bool).
2. Marque `bIsPrivate = True` para atributos como **Mana**, **Stamina** e **Ammo**.
3. Deixe `bIsPrivate = False` para atributos que outros jogadores precisam visualizar (como **Health** para renderizar a barra de vida de inimigos/aliados).
4. O C++ registra o array `PrivateAttributes` com a flag `COND_OwnerOnly` em tempo de compilação. Isso garante que a Unreal Engine nunca transmita em rede o valor desses atributos privados para clientes remotos (impedindo exploits de radar hack), sincronizando-os exclusivamente com o jogador proprietário.

---

## 📝 Aula 7: Criação e Aplicação de Status Effects (Fase 22)

### 7.1 Passo a Passo no Editor
1. Clique com o **botão direito** no Content Browser -> selecione **Miscellaneous** -> selecione **Data Asset**.
2. Busque pela classe base **`SBStatusEffectDefinition`** (ou `USBStatusEffectDefinition`), selecione-a e clique em **Select**.
3. Nomeie o ativo como `DA_StatusEffect_Veneno`.
4. Abra o `DA_StatusEffect_Veneno` e configure:
   * **Behavior Tag**: `State.Status.Poison`
   * **Duration**: `5.0` (duração em segundos).
   * **Default Period**: `1.0` (executa a cada 1.0 segundo).
   * **Period Attribute Tag**: `Attribute.Character.Health` (atributo afetado).
   * **Period Attribute Change**: `-10.0` (dano a cada segundo).
   * **Granted Tags**: `State.Status.Poisoned`
5. Para aplicar o efeito: obtenha `USBStatusEffectComponent` do personagem e chame `ApplyStatusEffect(DA_StatusEffect_Veneno)`.

---

## 📝 Aula 8: Sockets e Armas Visuais Replicadas (Fase 23)

### 8.1 Passo a Passo no Editor
1. Abra o seu Data Asset de Definição de Arma (derivado de `USBWeaponBehaviorDefinition`, ex: `DA_RifleWeapon`).
2. Localize as propriedades sob a categoria **Behavior | Visual**:
   * **Weapon Actor Class**: Selecione a classe do ator visual da arma (ex: `BP_RifleActor`).
   * **Active Socket Name**: `hand_rSocket` (mão direita do personagem para quando a arma estiver empunhada).
   * **Holster Socket Name**: `spine_03Socket` (socket nas costas do personagem para quando a arma estiver no coldre).
3. Quando a arma é equipada na mochila (por eventos de inventário), o `USBCombatComponent` realiza o spawn do ator no servidor e o anexa inicialmente ao socket do coldre.
4. Ao acionar o comando de disparo, a ativação do comportamento da arma (`USBWeaponBehavior`) reposiciona dinamicamente e com replicação o ator para o socket de empunhadura (`ActiveSocketName`). Ao cessar o disparo, o ator retorna ao coldre automaticamente.

---

## 📝 Aula 9: Anti-Cheat Avançado e Prevenção contra Trapaças (Fase 24)

### 9.1 Validação de Movimento (Speedhack)
* O `USBMovementComponent` realiza verificações físicas contra manipulação de posições no servidor. Ele calcula a velocidade teórica máxima a cada frame chamando `GetCalculatedMaxSpeed()`.
* **Sincronização**: O componente sincroniza `MaxWalkSpeed` do CMC com a base de `Attribute.Speed` na inicialização (`OnReady`). Qualquer dessincronização posterior em runtime que viole a tolerância de velocidade emitirá avisos periódicos no log (a cada 5 segundos).

### 9.2 Wall-Shot Protection (Obstrução Física)
* Ao disparar uma arma hitscan, o servidor realiza um traço físico extra a partir do tórax do personagem até o ponto de impacto.
* **Altura Dinâmica**: Para evitar falsos positivos quando o personagem está agachado (Crouch), a origem do traço é calculada dinamicamente com base em metade do scaled capsule half height do personagem. Se houver alguma parede estática entre o atirador e o impacto, o dano é rejeitado imediatamente pelo servidor.
