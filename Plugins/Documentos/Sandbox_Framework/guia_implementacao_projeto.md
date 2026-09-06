# 🏗️ Guia de Implementação do Projeto — do Framework ao Jogo

**Para**: você, no Unreal Editor.
**Levantado em**: 06/09/2026, inspecionando os `.uasset` e o código-fonte, com o editor aberto.
**Escopo**: tudo que falta para o framework virar jogo jogável. Não é só a HUD.

---

## 1. Onde o projeto realmente está

| Camada | Estado |
| :--- | :--- |
| **C++ do framework** | ✅ 11 plugins, build verde, **446 specs verdes**, exit code 0 |
| **Conteúdo (assets do editor)** | ⚠️ **28 assets** para 134 fases de sistemas |
| **Cadeia de jogo montada** | ❌ Quebrada em pontos específicos — §3 |

O projeto tem 4.131 `.uasset`, mas **4.103 deles são conteúdo do Game Animation Sample**
(personagens, MetaHumans, áudio, locomoção). O framework Sandbox tem 28.

> [!IMPORTANT] O que isso significa, sem rodeio
> O C++ está pronto e **medido** — não é promessa, é suíte verde reproduzível. O que não existe
> é o **conteúdo** que liga esse C++ a um jogo. Sistemas inteiros — crafting, loot, quests,
> efeitos de superfície — funcionam nos testes e **não têm um único asset** no projeto.
>
> Isso não é defeito. É o estágio natural: fundação construída, jogo por construir. Mas explica
> por que "446 specs verdes" e "nada acontece quando eu dou Play" são as duas coisas verdadeiras
> ao mesmo tempo.

---

## 2. Inventário: o que existe e o que não existe

Tipos de Data Asset que o framework define, e quantos assets existem de cada:

| Tipo | Qtd | Assets | Sistemas que dependem |
| :--- | :-: | :--- | :--- |
| `SBPawnDataAsset` | 2 | `DA_HeroPawnData`, `DA_HeroPawnDataV2` | Composição do personagem |
| `SBComponentSetDataAsset` | 2 | `DA_ComponentSet_Hero`, `_HeroV2` | idem |
| `SBCameraModeDefinition` | 3 | `DA_Camera_Walk/Sprint/Aim` | Câmera |
| `SBMovementBehaviorDefinition` | 2 | `DA_Movement_Sprint/Crouch` | Movimento |
| `SBMovementConfigDataAsset` | 1 | `DA_MovementConfig` | Estamina, velocidades |
| `SBAbilitySetDataAsset` | 1 | `DA_AbilitySet_Hero` | Habilidades |
| `SBGameplayBehaviorDefinition` | 1 | `DA_Def_Teleport` | Habilidade de exemplo |
| `SBWeaponBehaviorDefinition` | 1 | `DA_RifleWeapon` | Combate |
| `SBStatusEffectDefinition` | 1 | `DA_StatusEffect_Veneno` | Efeitos de status |
| `SBInputConfig` | 1 | `DA_InputConfig` | Input |
| **`SBItemDefinition`** | **0** | — | **Inventário, crafting, loot, mercador, quests, equipamento** |
| **`SBCraftingRecipeDataAsset`** | **0** | — | Crafting, estações |
| **`SBLootTableDataAsset`** | **0** | — | Loot, drops, PCG |
| **`SBQuestDataAsset`** | **0** | — | Quests, missões |
| **`SBInteractionConfigDataAsset`** | **0** | — | Interação (usa defaults do C++) |
| **`SBCombatConfigDataAsset`** | **0** | — | Combate (usa defaults do C++) |
| **`SBSurfaceEffectsDataAsset`** | **0** | — | Passos, impactos por superfície |
| **`SBAnimLayerConfigDataAsset`** | **0** | — | Camadas de animação por estado |

> [!TIP] `SBItemDefinition` é o gargalo de maior alcance
> Seis sistemas dependem dele e **nenhum item existe**. Crafting não tem o que produzir, loot
> não tem o que largar, o mercador não tem o que vender, quests não têm o que exigir, o
> inventário não tem o que guardar. É o primeiro asset a criar depois do §3.

---

## 3. Camada 0 — Fazer o personagem funcionar em jogo

Antes de criar conteúdo novo, a cadeia existente precisa fechar. Verifiquei elo por elo:

| # | Elo | Estado | Ação |
| :-: | :--- | :--- | :--- |
| 1 | `BP_SBCharacter_Hero` deriva de `SBCharacter` | ✅ | — |
| 2 | `DA_HeroPawnData` → `ComponentSet`, `AbilitySet`, `Mesh`, `AnimClass`, `InputConfig` | ✅ Todos preenchidos | — |
| 3 | `DA_ComponentSet_Hero` traz os 10 componentes | ✅ | — |
| 4 | `BP_SBCharacter_Hero` tem `PawnData` apontando para o `DA_HeroPawnData` | ⚠️ **Verificar** | §3.1 |
| 5 | GameMode usa esse personagem como `DefaultPawnClass` | ❌ **Não** | §3.2 |
| 6 | GameMode define `HUDClass = SBHUD` | ❌ **Não** | §3.3 |
| 7 | PawnData define `HUDLayoutClass` | ❌ **Vazio** | §3.3 |
| 8 | Widget da HUD tem as barras | ❌ **Vazio** | [[guia_implementacao_hud]] |
| 9 | `Attribute.Health` / `Mana` registrados | ❌ **Ninguém registra** | §3.4 |

Os 10 componentes que o `DA_ComponentSet_Hero` monta, para referência: `SBAttributeComponent`,
`SBStateComponent`, `SBMovementComponent`, `SBAbilityComponent`, `SBCameraComponent`,
`SBCombatComponent`, `SBInteractionComponent`, `SBInventoryComponent`,
`SBStatusEffectComponent`, `SBAnimLayerManagerComponent`.

### 3.1 Confirmar o PawnData no personagem

Abra `BP_SBCharacter_Hero` → **Class Defaults** → procure **Pawn Data**.

- Se estiver vazio, aponte para `DA_HeroPawnData`.
- **Verificação**: dê Play e olhe o Output Log com filtro `LogSandboxCharacter`. Deve aparecer
  `Successfully initialized character ... from PawnData ...`. Se aparecer
  `No PawnData set on character X!`, o campo continua vazio.

### 3.2 Fazer o GameMode usar o personagem do framework

`BP_SBGameMode_Test` hoje tem `DefaultPawnClass = /Game/Blueprints/SandboxCharacter_Mover_Ragdoll`
— um pawn do Game Animation Sample, **não** um `SBCharacter`.

Isso quebra a cadeia inteira: sem `SBCharacter`, não há PawnData, não há componentes, não há
atributos, e o `ASBHUD` nem encontra o que ler.

1. Abra `BP_SBGameMode_Test` → **Class Defaults** → **Default Pawn Class**.
2. Selecione `BP_SBCharacter_Hero`.
3. Compile e salve.

> Se você quer manter o pawn de animação por outros motivos, a alternativa é fazer
> `SandboxCharacter_Mover_Ragdoll` derivar de `SBCharacter` — mudança maior, mas preserva o
> trabalho de animação. Decida antes de seguir; o resto do guia assume o `BP_SBCharacter_Hero`.

### 3.3 Ligar a HUD

Três passos, detalhados em **[[guia_implementacao_hud]]**: `HUDClass = SBHUD` no GameMode,
`HUDLayoutClass` no PawnData, e as três barras no widget.

### 3.4 Registrar vida e mana

`USBAttributeComponent` **não registra atributo nenhum sozinho** — não há lista editável de
padrões. Quem registra, registra em código:

| Atributo | Quem registra |
| :--- | :--- |
| `Attribute.Stamina` | `USBMovementComponent::OnReady` |
| `Attribute.Weapon.Ammo` | `USBCombatComponent` |
| `Attribute.Weight`, `Attribute.MaxWeight` | `USBInventoryComponent` |
| `Attribute.Health`, `Attribute.MaxHealth`, `Attribute.Mana` | **ninguém** |

No `BeginPlay` do `BP_SBCharacter_Hero`, chame **Register Attribute** (é `BlueprintCallable`):

| Attribute Tag | Base | Min | Max |
| :--- | :-: | :-: | :-: |
| `Attribute.MaxHealth` | 100 | 0 | 100 |
| `Attribute.Health` | 100 | 0 | 100 |
| `Attribute.Mana` | 50 | 0 | 50 |

> **Verificação**: Output Log, filtro `LogSandboxCharacter`, procure `Registered attribute:
> Attribute.Health`.

> [!NOTE] Vale considerar mover isso para C++
> Registrar atributo por Blueprint funciona, mas espalha configuração de personagem por dois
> lugares. O coerente com o desenho seria o `ComponentSet` ou o PawnData carregarem valores
> iniciais de atributo — hoje não existe esse campo. É mudança de código; me peça se quiser.

**Ao fim da Camada 0** você tem: personagem com 10 componentes, HUD com três barras vivas,
estamina descendo ao correr. É o menor marco que prova a fundação inteira de pé.

---

## 4. Camada 1 — Itens, o desbloqueio de maior alcance

Nenhum `SBItemDefinition` existe. Crie os primeiros e seis sistemas saem do zero de uma vez.

**Onde**: sugiro `Content/SandboxFramework/Data/Items/`.
**Como**: botão direito → Miscellaneous → Data Asset → `SBItemDefinition`.

**Campos**:

| Campo | O que é |
| :--- | :--- |
| `DisplayName` | Texto exibido |
| `ItemTags` | Container de tags — usado por quests, crafting e filtros |
| `MaxStackCount` | 1 para equipamento, >1 para recurso |
| `Fragments` | **Onde mora o comportamento** — array instanciado |

Os fragmentos disponíveis, e o que cada um habilita:

| Fragmento | Habilita |
| :--- | :--- |
| `SBItemFragment_Equippable` | Equipar; liga arma ao `SBCombatComponent` |
| `SBItemFragment_Weight` | Peso, contra `Attribute.MaxWeight` |
| `SBItemFragment_Durability` | Durabilidade e desgaste |
| `SBItemFragment_Armor` | Redução de dano |
| `SBItemFragment_Consumable` | Uso e consumo |
| `SBItemFragment_Rarity` | Raridade, usado pelo loot |
| `SBItemFragment_Placeable` | Construção |
| `SBItemFragment_Salvageable` | Desmanche |
| `SBItemFragment_Upgrade` | Melhoria |
| `SBItemFragment_WorldActor` | Ator no mundo ao largar |

**Conjunto mínimo sugerido para destravar tudo** — quatro itens:

| Item | Fragmentos | Serve para testar |
| :--- | :--- | :--- |
| `DA_Item_Rifle` | Equippable (aponta `DA_RifleWeapon`), Weight, Durability | Equipar, combate, durabilidade |
| `DA_Item_Madeira` | Weight, Rarity, `MaxStackCount = 50` | Recurso, stacking, loot |
| `DA_Item_Bandagem` | Consumable, Weight | Consumo, cura |
| `DA_Item_Capacete` | Armor, Equippable, Weight, Durability | Armadura |

**Verificação**: com o `DA_Item_Rifle` criado, encha o `BP_BauDeTeste` com ele e teste pegar e
equipar em jogo. Isso exercita inventário, interação e combate numa tacada.

---

## 5. Camada 2 — Sistemas que passam a ser possíveis

Cada um depende dos itens do §4.

### 5.1 Crafting — `SBCraftingRecipeDataAsset`

| Campo | Conteúdo |
| :--- | :--- |
| `RecipeTag` | Tag da receita |
| `DisplayName` | Nome exibido |
| `RequiredStationTag` | Vazio = craft em qualquer lugar; com tag = exige estação |
| `Ingredients` | Array de `{ItemDef, Quantity}` |
| Resultado | `{ItemDef, Quantity}` |

Comece com uma: `DA_Recipe_Bandagem` = 2× Madeira → 1× Bandagem. Sem estação, para testar o
caminho simples primeiro.

### 5.2 Loot — `SBLootTableDataAsset`

Entradas com `ItemDefinition`, `MinStackCount`, `MaxStackCount`, `Weight` (peso relativo do
sorteio) e `DropChance`.

`DA_Loot_Basico` com Madeira 1–5 e Bandagem com chance baixa já exercita o sistema inteiro.

### 5.3 Quests — `SBQuestDataAsset`

Objetivos com `ObjectiveTag`, `RequiredCount`, `Description`; recompensas com `ItemDef`,
`Quantity`, `Experience`.

### 5.4 Efeitos de superfície — `SBSurfaceEffectsDataAsset`

`SurfaceEffectsMap` mapeia `EPhysicalSurface` → `{Sound, VisualEffect}`. Alimenta passos e
impactos. **Este é o de melhor retorno para o BLACK VEIL**: som de passo por superfície é
metade da atmosfera de um survival horror, e o C++ já está pronto.

### 5.5 Camadas de animação — `SBAnimLayerConfigDataAsset`

`LayerMappings` liga `StateTag` → `AnimLayerClass` com `Priority`. Conecta as tags de estado do
framework às animações do Game Animation Sample — que é o ativo mais valioso já presente no
projeto e hoje não conversa com o framework.

### 5.6 Configs opcionais

`SBInteractionConfigDataAsset` e `SBCombatConfigDataAsset` **não são obrigatórios** — o C++ usa
defaults embutidos quando não há asset. Crie só quando quiser afinar tolerâncias de rede e
alcance.

---

## 6. Ordem recomendada para o BLACK VEIL

Sabendo que o alvo é survival horror single-player:

| Prioridade | O quê | Por quê |
| :-: | :--- | :--- |
| **1** | Camada 0 (§3) | Sem isso nada é jogável. É o marco que prova a fundação |
| **2** | 4 itens (§4) | Desbloqueia seis sistemas de uma vez |
| **3** | Efeitos de superfície (§5.4) | Atmosfera é o núcleo do horror, e o C++ está pronto |
| **4** | Camadas de animação (§5.5) | Liga o acervo de animação ao framework |
| **5** | Loot (§5.2) + Crafting (§5.1) | Núcleo do laço de sobrevivência |
| **6** | Quests (§5.3) | Estrutura narrativa, depois do laço funcionar |

Sistemas que o framework tem e o BLACK VEIL provavelmente **não** vai usar — automação
industrial, redes de energia, esteiras, drones de carga, elevador espacial, mechas, espaçonaves
— não precisam de conteúdo nenhum. Estão em C++, testados, e ficam dormentes sem custo.

---

## 7. Nada disso pode ser feito por MCP

Verificado por execução em 06/09/2026 — ver [[mcp_playbook_unreal]]:

- **Criar Data Asset**: impossível. O plugin aceita 36 comandos e nenhum cria asset que não seja
  Blueprint ou Widget.
- **Criar/editar widget**: quebrado — os nomes de parâmetro do schema Python não batem com os
  do C++.
- **Alcançar `/Game/SandboxFramework/`**: impossível. Os caminhos são fixos em
  `/Game/Blueprints/` e `/Game/Widgets/`.
- **Salvar**: não existe.

O MCP serve para spawnar atores de teste e montar Blueprints simples em `/Game/Blueprints/`. O
conteúdo do framework é trabalho manual no editor.

---

## 8. Como saber que funcionou

Sem afirmar mais do que se observou — a regra que este projeto aprendeu da pior forma
(ver [[audit_report_conformidade_2026-09-05]]):

| Marco | Evidência observável |
| :--- | :--- |
| Camada 0 completa | Barra de estamina desce ao correr e sobe ao parar |
| Personagem composto | `LogSandboxCharacter`: `Successfully initialized character ... from PawnData` |
| Atributos registrados | `LogSandboxCharacter`: `Registered attribute: Attribute.Health` |
| HUD instanciada | `LogSandboxUI`: `Successfully spawned HUD layout widget` |
| Item funcionando | Pegar do baú e equipar, com o modelo aparecendo na mão |
| Crafting funcionando | Ingredientes somem do inventário e o resultado aparece |

**Suíte verde não prova que o jogo funciona.** Ela prova que o C++ faz o que os testes dizem.
São garantias diferentes, e confundir as duas foi exatamente o defeito que a auditoria de
05/09/2026 encontrou.
