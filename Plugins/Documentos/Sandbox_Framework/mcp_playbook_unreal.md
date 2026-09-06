# 🎮 Playbook de Execução via Unreal MCP

**Para**: uma sessão do Claude com o servidor `unrealMCP` conectado e o Unreal Editor **aberto**
no projeto `D:\Unreal\GameAnimationSample`.
**Validado em**: 06/09/2026, **executando** cada ferramenta contra o editor aberto e lendo o
código-fonte do plugin em `Plugins/UnrealMCP/Source/`.

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

### 2.1 ✅ Funciona

| Ferramenta | Parâmetros | Observação da execução |
| :--- | :--- | :--- |
| `get_actors_in_level` | *(nenhum)* | Devolveu os 17 atores do nível |
| `find_actors_by_name` | `pattern` | Sem resultado devolve **saída vazia**, não erro |
| `spawn_actor` | `name`, `type`, `location`, `rotation` | `type` é classe nativa: `PointLight`, `StaticMeshActor` |
| `delete_actor` | `name` | Confirmado |
| `create_blueprint` | `name`, `parent_class` | Cria **sempre** em `/Game/Blueprints/`, **em memória** |
| `add_component_to_blueprint` | `blueprint_name`, `component_type`, `component_name`, … | ⚠️ `component_type` exige **caminho completo** — ver §2.3 |
| `set_static_mesh_properties` | `blueprint_name`, `component_name`, `static_mesh` | `/Engine/BasicShapes/Cube.Cube` funciona |
| `set_physics_properties` | `blueprint_name`, `component_name`, `simulate_physics`, `mass`, … | Confirmado |
| `compile_blueprint` | `blueprint_name` | Devolveu `compiled: true` |

### 2.2 ❌ Quebrado ou inalcançável

| Ferramenta | Problema | Evidência |
| :--- | :--- | :--- |
| **Toda a família UMG** | Os nomes de parâmetro do schema Python **não batem** com os que o C++ lê. Nada traduz entre as camadas | `create_umg_widget_blueprint(widget_name=…)` → `{"error": "Missing 'name' parameter"}` |
| `spawn_blueprint_actor` | Só encontra Blueprints **salvos em disco** sob `/Game/Blueprints/` | `BP_SBCharacter_Hero` (em `/Game/SandboxFramework/Blueprints/`) → *"not found – it must reside under /Game/Blueprints"* |
| `spawn_blueprint_actor` de BP recém-criado | `FPackageName::DoesPackageExist` checa o **disco**; `create_blueprint` não salva | `BP_MCPProbe_Cube` criado com sucesso e depois *"not found"* no spawn |

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
| `spawn_blueprint_actor` | `get_actor_properties(name=<actor_name>)` | Ator ausente ou sem componentes |
| `delete_actor` | `find_actors_by_name` com o mesmo padrão | Ainda presente |
| `create_blueprint` | `compile_blueprint` no mesmo nome | Erro ou nome não encontrado |
| `add_component_to_blueprint` | `compile_blueprint` | `Unknown component type` → §2.3 |
| `set_actor_property` | `get_actor_properties` e conferir o valor | Valor inalterado |
| Qualquer timeout | Ler o log (§4) | — |

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
