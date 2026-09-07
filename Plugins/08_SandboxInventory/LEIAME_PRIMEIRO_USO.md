# Sandbox Inventory — primeiro uso

Guia de dez minutos: instalar, abrir a demonstração e criar o primeiro item próprio.
Se em algum ponto você precisar perguntar alguma coisa a alguém, este documento falhou.

---

## 1. Instalar

Copie as pastas de plugin para `SeuProjeto/Plugins/`:

```
01_SandboxCommon      02_SandboxInterfaces   03_SandboxAssets
04_SandboxCore        05_SandboxCharacter    07_SandboxInteraction
08_SandboxInventory   09_SandboxUI
```

O inventário depende dos sete acima. Nenhum deles depende de conteúdo do seu jogo.

> [!WARNING] Instale num caminho curto
> O Windows aborta o build quando um caminho de compilação passa de 260 caracteres, e a
> mensagem não deixa claro que a causa é o caminho. `D:\Projetos\MeuJogo` funciona;
> `C:\Users\voce\Documents\Unreal Projects\Meu Jogo de Sobrevivencia` provavelmente não.

Habilite no `.uproject` (ou pelo menu Plugins do editor) — todos vêm com a engine:

```
ModularGameplay · GameplayAbilities · EnhancedInput · PCG · SmartObjects · StateTree · GameplayStateTree
```

Gere os arquivos de projeto e compile. **Editor fechado durante o build**: Live Coding trava a
compilação.

---

## 2. Abrir a demonstração

`/08_SandboxInventory/Demo/L_InventoryDemo` — abra e dê **Play**.

O painel de inventário aparece no canto e reage ao conteúdo do inventário do personagem. No
mapa estão quatro atores, cada um ligado a um Data Asset de exemplo:

| No mapa | O que é | Ligado a |
| :--- | :--- | :--- |
| `ScrapNode` | nó de recurso que dropa ao ser quebrado | `DA_LootTable_Crate` |
| `Forge` | bancada de crafting | `DA_Recipe_Torch` |
| `SupplyChest` | baú com inventário próprio | — |
| `TorchDrop` | item largado no chão | `DA_Item_Torch` |

A cadeia fecha em si mesma: o nó dropa **Sucata** pela loot table, a forja transforma
2× Sucata em uma **Tocha**, e o drop físico carrega a mesma Tocha.

---

## 3. Criar o primeiro item

Botão direito no Content Browser → **Miscellaneous → Data Asset** → `SBItemDefinition`.

| Campo | Para que serve |
| :--- | :--- |
| `DisplayName` | nome que o jogador lê |
| `MaxStackCount` | quantos cabem numa pilha (1 = não empilha) |
| `ItemTags` | classificação por Gameplay Tag |
| `Fragments` | **é aqui que o item ganha comportamento** |

Um `SBItemDefinition` sem fragmentos é só um nome. Cada fragmento acrescenta uma capacidade,
e o item tem exatamente as que você adicionar:

| Fragmento | O que o item passa a ter |
| :--- | :--- |
| `Weight` | peso, contra a capacidade de carga |
| `Rarity` | raridade por tag |
| `Durability` | desgaste e quebra |
| `Consumable` | efeito ao consumir (ex.: `Attribute.Health` +25) |
| `Armor` | slot de equipamento e modificadores de atributo |
| `Equippable` | pode ser equipado |
| `Salvageable` | pode ser desmontado em outros itens |
| `Upgrade` | pode subir de nível |
| `Placeable` / `WorldActor` | vira ator no mundo |

Compare com os quatro prontos em `Demo/`: `DA_Item_Scrap` (material simples),
`DA_Item_Bandage` (consumível), `DA_Item_Torch` (durabilidade) e `DA_Item_LeatherVest`
(armadura com slot e bônus de defesa).

---

## 4. Colocar no jogo

- **Inventário no personagem** — adicione `SBInventoryComponent` ao seu Pawn.
- **Receitas** — `SBCraftingRecipeDataAsset` com os ingredientes, e a estação recebe a receita
  em `SupportedRecipes` (veja `BP_Forge`).
- **Loot** — `SBLootTableDataAsset` com entradas ponderadas, atribuída ao nó ou ao baú.
- **UI** — herde de `SBInventoryGridWidget` e dê a um Text Block o nome exato **`ContentsText`**:
  a classe C++ o encontra por nome e escreve o conteúdo do inventário nele, sem nenhum nó de
  Blueprint. `WBP_InventoryPanel` é esse exemplo.

---

## 5. O que este pacote **não** faz

Dito aqui para você não descobrir depois:

- **A UI é uma lista de texto**, não uma grade com ícone e arrastar-e-soltar. `SBItemDefinition`
  não tem campo de ícone.
- **Não há arte**: nenhum mesh, ícone, som ou animação. Os atores da demo usam formas básicas
  da engine.
- **O vocabulário de tags de item é mínimo** — há uma única tag `Item.*` registrada. Espere
  declarar as suas.
- **Replicação é parcial**: inventário, itens e o baú replicam; a UI é local, como convém.

---

## 6. Se algo der errado

| Sintoma | Causa provável |
| :--- | :--- |
| Build aborta falando em 260 caracteres | caminho do projeto fundo demais (§1) |
| Erro de módulo não encontrado ao compilar | falta um dos sete plugins, ou um plugin de engine não habilitado |
| Painel não aparece no Play | o GameMode do mapa não é o `BP_DemoGameMode` |
| Painel aparece vazio | o Pawn não tem `SBInventoryComponent` |
| Item não empilha | `MaxStackCount` é 1 |
| Item sem peso na capacidade | falta o fragmento `Weight` |
