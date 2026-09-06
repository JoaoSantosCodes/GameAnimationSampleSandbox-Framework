# 🎮 Playbook de Execução via Unreal MCP

**Para**: uma sessão do Claude com o servidor `unrealMCP` conectado e o Unreal Editor **aberto**
no projeto `D:\Unreal\GameAnimationSample`.
**Validado em**: 06/09/2026, **executando as 21 ferramentas expostas** contra o editor aberto e
lendo o código-fonte do plugin em `Plugins/UnrealMCP/Source/`. Nenhuma linha deste documento é
suposição: cada veredito tem chamada real por trás.

> [!DANGER] Este plugin MCP é muito mais limitado do que os schemas sugerem
> A primeira versão deste playbook foi escrita a partir dos schemas das ferramentas. A execução
> real derrubou **quatro** das cinco missões planejadas. Os schemas descrevem parâmetros que o
> C++ não lê, caminhos que ele ignora e nomes de classe que ele rejeita.
>
> **Leia §2 inteira antes de prometer qualquer coisa ao usuário.** Ela separa o que funciona do
> que está quebrado, com evidência de execução para cada item.

---

## 1. Como usar este documento

1. Rode o **teste de conexão** (§3). Se falhar, pare.
2. Confira em **§2** se a ferramenta que você pretende usar está na tabela **funciona**.
3. Execute uma chamada por vez, verificando o resultado (§5).
4. Quando algo der "Timeout", **leia o log do editor** (§4) — quase sempre há um erro real ali.
5. Feche pelo protocolo de §8.

---

## 2. O que funciona e o que não funciona

Cada linha foi **executada** em 06/09/2026 contra o editor aberto.

### 2.1 ✅ Funciona — 15 de 21, todas executadas

| Ferramenta | Observação da execução |
| :--- | :--- |
| `get_actors_in_level` | Devolveu os 17 atores do nível |
| `find_actors_by_name` | Sem resultado devolve **saída vazia**, não erro |
| `get_actor_properties` | ⚠️ Devolve **só** `name`, `class`, `location`, `rotation`, `scale`. **Não lista componentes** — ver §5 |
| `spawn_actor` | `type` é classe nativa: `PointLight`, `StaticMeshActor` |
| `set_actor_transform` | Aceita `location`/`rotation`/`scale` parciais |
| `set_actor_property` | `bHidden=true` aplicado e confirmado |
| `delete_actor` | Confirmado |
| `create_blueprint` | Cria **sempre** em `/Game/Blueprints/`, **em memória** |
| `add_component_to_blueprint` | ⚠️ `component_type` exige **caminho completo** — §2.3 |
| `set_component_property` | `bVisible` aplicado no componente |
| `set_static_mesh_properties` | `/Engine/BasicShapes/Cube.Cube` funciona |
| `set_physics_properties` | Confirmado |
| `set_blueprint_property` | ⚠️ Só encontra a variável **após `compile_blueprint`** — §2.6 |
| `add_blueprint_variable` | `TestHealth`/`Float` criada |
| `compile_blueprint` | Devolveu `compiled: true` |

### 2.1b ✅ Grafo de Blueprint — funciona

| Ferramenta | Observação da execução |
| :--- | :--- |
| `add_blueprint_event_node` | `ReceiveBeginPlay` → devolveu `node_id` |
| `add_blueprint_function_node` | Só funções que o `target` **realmente possui**: `PrintString` em `self` falha (é estática de `UKismetSystemLibrary`); `SetActorHiddenInGame` funciona |
| `add_blueprint_input_action_node` | Funcionou após `create_input_mapping` |
| `add_blueprint_self_reference` | Devolveu `node_id` |
| `add_blueprint_get_self_component_reference` | Devolveu `node_id` |
| `connect_blueprint_nodes` | Pinos `then` → `execute` conectados |
| `create_input_mapping` | `MCPTest_Jump`/`SpaceBar` criado |

### 2.2 ❌ Quebrado ou inalcançável

| Ferramenta | Problema | Evidência |
| :--- | :--- | :--- |
| **Toda a família UMG** (6 ferramentas) | Os nomes de parâmetro do schema Python **não batem** com os que o C++ lê. Nada traduz entre as camadas | `create_umg_widget_blueprint(widget_name=…)` → `{"error": "Missing 'name' parameter"}` |
| `spawn_blueprint_actor` | Só encontra Blueprints **salvos em disco** sob `/Game/Blueprints/` | `BP_SBCharacter_Hero` (em `/Game/SandboxFramework/Blueprints/`) → *"not found – it must reside under /Game/Blueprints"* |
| `spawn_blueprint_actor` de BP recém-criado | `FPackageName::DoesPackageExist` checa o **disco**; `create_blueprint` não salva | `BP_MCPProbe_Cube` criado com sucesso e depois *"not found"* no spawn |
| `find_blueprint_nodes` | O schema expõe `event_type`; o C++ exige **`event_name`**, que não é exposto | `node_type="Event"` com e sem `event_type` → *"Missing 'event_name' parameter for Event node search"* |

> [!CAUTION] Quatro ferramentas devolvem `"status": "success"` embrulhando um erro
> `add_button_to_widget`, `bind_widget_event` e `set_text_block_binding` respondem
> `{"status": "success", "result": {"error": "Missing blueprint_name parameter"}}`.
>
> Um chamador que olhe só o `status` conclui que funcionou. **Sempre inspecione o `result`**, não
> apenas o campo `status`.

#### A família UMG, em detalhe

O C++ (`UnrealMCPUMGCommands.cpp`) lê nomes diferentes dos que o schema expõe:

| Comando | Schema Python expõe | C++ realmente lê |
| :--- | :--- | :--- |
| `create_umg_widget_blueprint` | `widget_name`, `parent_class`, `path` | **`name`** — `path` e `parent_class` são **ignorados** (grava sempre em `/Game/Widgets/`) |
| `add_text_block_to_widget` | `widget_name`, `text_block_name` | **`blueprint_name`** + `widget_name` (= nome do text block) |
| `add_button_to_widget` | `widget_name`, `button_name` | **`blueprint_name`** + `widget_name` |
| `bind_widget_event` | `widget_name`, `widget_component_name` | **`blueprint_name`** + `widget_name` |
| `set_text_block_binding` | `widget_name`, `text_block_name`, `binding_property` | **`blueprint_name`** + `widget_name` + **`binding_name`** |

**Consequência**: não é possível criar nem editar widget por MCP. Como
`create_umg_widget_blueprint` é a porta de entrada e ela falha sempre, o resto da família fica
inacessível mesmo que os parâmetros dos outros comandos batessem.

### 2.6 Ordem obrigatória: variável antes de propriedade

`add_blueprint_variable` devolve sucesso, mas a propriedade **não existe no CDO** até recompilar.
Verificado por execução:

```
add_blueprint_variable(variable_name="TestHealth", variable_type="Float")  -> success
set_blueprint_property(property_name="TestHealth", property_value="75.0")  -> "Property not found: TestHealth"
compile_blueprint(...)                                                     -> compiled: true
set_blueprint_property(property_name="TestHealth", property_value="75.0")  -> success
```

É a evidência concreta da regra "compile após toda mudança estrutural".

### 2.3 `component_type` exige caminho completo

A docstring da ferramenta diz *"use component class name without U prefix"*. **Está errada.**
O C++ resolve com `FindObject<UClass>(nullptr, *ComponentType)`, forma que exige caminho
completo — `ANY_PACKAGE` foi removido na UE5.

```
add_component_to_blueprint(component_type="StaticMeshComponent")
  -> {"error": "Unknown component type: StaticMeshComponent"}

add_component_to_blueprint(component_type="/Script/Engine.StaticMeshComponent")
  -> {"status": "success"}
```

**Formato**: `/Script/<Módulo>.<Classe>` — ver §6.2 para as classes do framework.

### 2.4 Caminhos fixos no código

| Família | Caminho fixo | Fonte |
| :--- | :--- | :--- |
| Blueprints | `/Game/Blueprints/` | `UnrealMCPBlueprintCommands.cpp:80`, `UnrealMCPCommonUtils.cpp:156`, `UnrealMCPEditorCommands.cpp:420` |
| Widgets | `/Game/Widgets/` | `UnrealMCPUMGCommands.cpp:73` |

Os assets deste projeto vivem em `/Game/SandboxFramework/…` — **fora do alcance do MCP**.
Nenhum Blueprint do framework pode ser spawnado, e nenhum widget existente pode ser editado.

### 2.5 O que não existe de forma alguma

- **Progress Bar** — o UMG só teria Text Block e Button, e mesmo esses estão inacessíveis.
- **Criar ou editar Data Asset** (`PawnData`, `ComponentSet`, configs).
- **Importar asset, criar material, malha, animação ou nível.**
- **Salvar.** Nem pacote, nem nível. É o gargalo central: um Blueprint criado por MCP só pode
  ser spawnado **depois** que o usuário salvar manualmente.
- **Desfazer.**
- **Rodar a suíte de testes** — é linha de comando, ver §8.

> [!IMPORTANT] Criar Data Asset é impossível, e isso é definitivo
> Não é limitação do schema exposto: o dispatch do plugin aceita **36 comandos**, enumerados em
> `Plugins/UnrealMCP/Source/UnrealMCP/Private/`, e **nenhum** cria asset que não seja Blueprint
> ou Widget. Não há `create_data_asset`, `create_asset` nem `save_asset`.
>
> Consequência para este projeto: `PawnData`, `ComponentSet`, `DA_MovementConfig`,
> `DA_AbilitySet` e similares **só podem ser criados à mão no editor**. Como a composição de
> personagem deste framework passa inteiramente por eles (§6.1), montar um personagem novo por
> MCP é impossível de ponta a ponta.
>
> Seis comandos existem no C++ mas **não estão expostos** como ferramenta: `ping`,
> `focus_viewport`, `take_screenshot`, `create_actor`, `set_pawn_properties` e
> `add_blueprint_get_component_node`. `take_screenshot` seria o mais útil — daria verificação
> visual, que hoje não existe.

---

## 3. Teste de conexão

**Primeira chamada de toda sessão:**

```
get_actors_in_level()
```

| Resultado | Significado | Ação |
| :--- | :--- | :--- |
| Lista de atores | Canal vivo | Prosseguir |
| Saída vazia | Editor fechado ou sem nível | Verificar o processo antes de concluir |
| Erro / timeout | Ver §4 — pode ser erro real disfarçado | Ler o log |

Para distinguir "editor fechado" de "editor ocupado", verifique o processo:

```powershell
Get-Process -Name 'UnrealEditor*' -ErrorAction SilentlyContinue
```

---

## 4. "Timeout" quase nunca é timeout

> [!IMPORTANT] A descoberta mais útil desta validação
> O cliente MCP devolve `{"status": "error", "error": "Timeout receiving Unreal response"}` em
> casos onde o servidor **respondeu normalmente, com uma mensagem de erro precisa**. A mensagem
> se perde no caminho.
>
> Tratar isso como "editor ocupado" e tentar de novo desperdiça a sessão inteira contra um erro
> que o log explica em uma linha.

**Sempre que der timeout, leia o log:**

```bash
tail -40 "D:/Unreal/GameAnimationSample/Saved/Logs/GameAnimationSample.log" \
  | grep -iE "Sending response|error|Received:"
```

O plugin registra tudo: o JSON recebido, o comando executado e a resposta enviada. Foi assim
que se descobriu a restrição de `/Game/Blueprints`, que nenhuma mensagem do cliente revelava.

---

## 5. Protocolo de verificação

**Resposta de sucesso não é prova.** Para cada escrita, a verificação correspondente:

| O que você fez | Como verificar | Sinal de falha |
| :--- | :--- | :--- |
| `spawn_actor` | `find_actors_by_name(pattern=<name>)` | Saída vazia |
| `spawn_blueprint_actor` | `find_actors_by_name` | Saída vazia |
| `delete_actor` | `find_actors_by_name` com o mesmo padrão | Ainda presente |
| `create_blueprint` | `compile_blueprint` no mesmo nome | Erro ou nome não encontrado |
| `add_component_to_blueprint` | `compile_blueprint` | `Unknown component type` → §2.3 |
| `add_blueprint_variable` | `compile_blueprint`, depois `set_blueprint_property` na variável | `Property not found` antes de compilar — §2.6 |
| `set_actor_property` / `set_actor_transform` | `get_actor_properties` | Valor inalterado |
| Qualquer resposta | Ler o **`result`**, não só o `status` | `status: success` com `result.error` — §2.2 |
| Qualquer timeout | Ler o log (§4) | — |

> [!WARNING] Não existe verificação de componente
> `get_actor_properties` devolve apenas `name`, `class`, `location`, `rotation` e `scale` —
> **nunca a lista de componentes**. Verificado por execução.
>
> Ou seja: **não há como confirmar por MCP que um componente foi realmente adicionado a um ator
> spawnado.** `compile_blueprint` sem erro é a evidência mais forte disponível, e ela é fraca.
> Para confirmação real, peça ao usuário para olhar o painel de componentes no editor.

---

## 6. Contexto do projeto

### 6.1 Composição por PawnData

`ASBCharacter` deriva de `AModularCharacter` e **não cria componente algum no construtor**
(verificado: zero `CreateDefaultSubobject`). A composição acontece em runtime, em
`InitializeFromPawnData()`, via `PawnData->ComponentSet` e `USBComponentFactory`.

> [!DANGER] Nunca adicione componentes do framework a um Blueprint de `SBCharacter`
> Se o Blueprint tiver um `SBAttributeComponent` e o `ComponentSet` do PawnData adicionar outro,
> o personagem fica com **dois**. `FindComponentByClass` devolve o primeiro encontrado, sem
> ordem garantida — um conjunto de atributos é escrito e o outro lido, sem erro nem log.
>
> A forma correta de mudar a composição é editar o `ComponentSet` no editor. O MCP não cria nem
> edita Data Assets (§2.5).

Na prática isso é acadêmico: os Blueprints do framework estão fora de `/Game/Blueprints/` e o
MCP não os alcança (§2.4).

### 6.2 Classes do framework, em formato utilizável

Para `component_type`, com o caminho completo que o §2.3 exige:

| Componente | `component_type` |
| :--- | :--- |
| Atributos | `/Script/SandboxCharacter.SBAttributeComponent` |
| Estado por tags | `/Script/SandboxCharacter.SBStateComponent` |
| Movimento | `/Script/SandboxCharacter.SBMovementComponent` |
| Habilidades | `/Script/SandboxCharacter.SBAbilityComponent` |
| Efeitos de status | `/Script/SandboxCharacter.SBStatusEffectComponent` |
| Combate | `/Script/SandboxCombat.SBCombatComponent` |
| Interação | `/Script/SandboxInteraction.SBInteractionComponent` |
| Inventário | `/Script/SandboxInventory.SBInventoryComponent` |

Componentes de engine: `/Script/Engine.StaticMeshComponent`,
`/Script/Engine.PointLightComponent`, `/Script/Engine.BoxComponent`.

Para `parent_class` em `create_blueprint`, o nome curto funciona (`Actor` foi confirmado).

### 6.3 Tags de atributo

Nomes exatos, verificados em `SBGameplayTags.cpp`:

`Attribute.Health` · `Attribute.MaxHealth` · `Attribute.Stamina` · `Attribute.Mana` ·
`Attribute.Speed` · `Attribute.Weapon.Ammo` · `Attribute.MaxWeight`

**Não existem** `Attribute.MaxStamina` nem `Attribute.MaxMana` — o teto desses vive em
`FSBAttribute::MaxValue`, lido por `ISBAttributeComponentInterface::GetAttributeMaxValue`.

### 6.4 O HUD de status está vazio

`Content/SandboxFramework/Widgets/USBStatusHUDWidget.uasset` deriva de `SBStatusHUDWidget`, mas
**não contém widget nenhum**. Os três `UProgressBar` que o C++ espera — `PB_Health`, `PB_Mana`,
`PB_Stamina` — são `OptionalWidget = true` e não existem no Blueprint. Em jogo a HUD não mostra
nada, e nunca mostrou.

O C++ está pronto: lê os atributos por contrato a 30 Hz por timer, sem alocação.

**O MCP não consegue ajudar aqui** — sem UMG funcional (§2.2) e sem alcance a
`/Game/SandboxFramework/` (§2.4). As barras precisam ser criadas à mão no editor, com os nomes
**exatos** acima; `BindWidget` casa por nome e divergência dá barra nula silenciosa.

---

## 7. Missão viável: ator de teste físico

A única do playbook original que sobreviveu à validação — e mesmo assim com uma ressalva.

```
create_blueprint(name="BP_TestPhysicsCube", parent_class="Actor")
add_component_to_blueprint(blueprint_name="BP_TestPhysicsCube", component_type="/Script/Engine.StaticMeshComponent", component_name="Mesh")
set_static_mesh_properties(blueprint_name="BP_TestPhysicsCube", component_name="Mesh", static_mesh="/Engine/BasicShapes/Cube.Cube")
set_physics_properties(blueprint_name="BP_TestPhysicsCube", component_name="Mesh", simulate_physics=True, gravity_enabled=True, mass=50.0)
compile_blueprint(blueprint_name="BP_TestPhysicsCube")
```

Os cinco passos acima **foram executados com sucesso** em 06/09/2026.

> [!WARNING] O spawn exige uma ação humana no meio
> `spawn_blueprint_actor` falha aqui, porque `FPackageName::DoesPackageExist` checa o **disco** e
> o Blueprint recém-criado só existe em memória.
>
> **Peça ao usuário para salvar** (`Ctrl+S` no Content Browser, em `/Game/Blueprints/`) e só
> então:
> ```
> spawn_blueprint_actor(blueprint_name="BP_TestPhysicsCube", actor_name="TestCube_01", location=[500, 0, 400])
> get_actor_properties(name="TestCube_01")
> ```

**Alternativa sem Blueprint**: para um objeto simples, `spawn_actor` funciona direto e não
precisa de salvamento.

```
spawn_actor(name="TestMesh_01", type="StaticMeshActor", location=[500, 0, 400])
find_actors_by_name(pattern="TestMesh_01")
```

---

## 8. Encerramento de sessão

1. **Liste o que criou** — nome do ator no nível e caminho no Content Browser.
2. **Diga que nada foi salvo.** Blueprints criados por MCP existem só em memória; fechar o
   editor os descarta. Se o usuário quiser mantê-los, precisa salvar à mão.
3. **Liste o que não deu para fazer e por quê**, apontando a seção deste documento.
4. **Não afirme que funciona em jogo.** O MCP monta estrutura; comportamento só se verifica
   jogando ou por teste automatizado.

> [!IMPORTANT] A regra que este projeto aprendeu da pior forma
> Uma auditoria em 05/09/2026 encontrou 14 fases marcadas como "concluídas, compiladas e
> homologadas, 100% verde" cujo código **nunca tinha passado pelo compilador**. Depois disso,
> 166 asserções de teste foram descobertas inertes, verificando nada. Este próprio playbook, na
> primeira versão, descrevia cinco missões das quais quatro não funcionavam.
>
> Relate **o que você observou**, não o que deveria ter acontecido.
> Ver [[audit_report_conformidade_2026-09-05]] e [[validation_report_2026-09-06]].

**Rodar a suíte de testes não é tarefa de MCP** — é linha de comando, com o editor fechado:

```powershell
& "D:\Unreal\Unreal Sistema\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" `
  "D:\Unreal\GameAnimationSample\GameAnimationSample.uproject" `
  -NullRHI -NoSound -NoSplash -stdout -unattended -nopause `
  -ExecCmds="Automation RunTest Sandbox; Quit" -log
```

---

## 9. Se for consertar o plugin

As limitações de §2.2 e §2.4 são **defeitos do plugin**, não da ideia. Em ordem de retorno:

| # | Correção | Arquivo | Impacto |
| :-: | :--- | :--- | :--- |
| 1 | Alinhar nomes de parâmetro do UMG entre Python e C++ | `UnrealMCPUMGCommands.cpp` | Destrava a família UMG inteira |
| 2 | Aceitar caminho completo em vez de fixar `/Game/Blueprints/` | `UnrealMCPEditorCommands.cpp:420`, `UnrealMCPBlueprintCommands.cpp:80`, `UnrealMCPCommonUtils.cpp:156` | Dá acesso a `/Game/SandboxFramework/` |
| 3 | Salvar o pacote após `create_blueprint` | `UnrealMCPBlueprintCommands.cpp` | Remove a ação humana do meio do fluxo |
| 4 | Propagar a mensagem de erro do servidor em vez de "Timeout" | camada Python | Torna o log desnecessário para diagnóstico |
| 5 | Usar `FindObject` com caminho ou `TryFindType` documentado | `UnrealMCPBlueprintCommands.cpp:197` | Faz a docstring virar verdade |
