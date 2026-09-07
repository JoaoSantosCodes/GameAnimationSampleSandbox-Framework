# 🎮 Playbook de Execução via Unreal MCP

**Para**: uma sessão do Claude com o servidor `unrealMCP` conectado e o Unreal Editor **aberto**
no projeto `D:\Unreal\GameAnimationSample`.
**Validado em**: 06/09/2026, **executando as 21 ferramentas expostas** contra o editor aberto e
lendo o código-fonte do plugin em `Plugins/UnrealMCP/Source/`. Nenhuma linha deste documento é
suposição: cada veredito tem chamada real por trás.

> [!IMPORTANT] Este plugin foi consertado em 06/09/2026 — 11 defeitos
> A primeira versão deste playbook descrevia um plugin em que a família UMG era inacessível e
> nada fora de `/Game/Blueprints/` podia ser spawnado. Isso foi corrigido no C++ e revalidado
> por execução. §2 traz a matriz atual; §10, o histórico dos defeitos e o que ele ensina.
>
> **Verifique sempre pelo estado, nunca pela resposta.** Foi assim que os 11 apareceram, e é a
> única disciplina que sobrevive a qualquer versão deste plugin.

---

## 1. Como usar este documento

1. Rode o **teste de conexão** (§3). Se falhar, pare.
2. Confira em **§2** se a ferramenta que você pretende usar está na tabela **funciona**.
3. Execute uma chamada por vez, verificando o resultado (§5).
4. Quando algo der "Timeout", **leia o log do editor** (§4) — quase sempre há um erro real ali.
5. Feche pelo protocolo de §8.

---

## 2. O que funciona

> [!TIP] Propriedade de qualquer tipo agora se escreve como texto
> Desde 06/09/2026 o `set_data_asset_property` e as `properties` do `add_instanced_object` aceitam a sintaxe de importacao da engine, o que cobre o que antes era inalcancavel:
> - `FText` — `Tocha`
> - `FGameplayTag` — `(TagName="Loot.Rarity.Common")`
> - `FGameplayTagContainer` — `(GameplayTags=((TagName="Item.Type.Material")))`
> - referencia a asset — `/08_SandboxInventory/Demo/DA_Item_Scrap.DA_Item_Scrap`
> - array de struct — `((ItemDef="/caminho.Asset",Quantity=2))`
>
> Conteudo de plugin vive em `/<NomeDoPlugin>/...`, nao em `/Game/`. A pasta `Content/` do plugin precisa existir **antes** de o editor abrir, senao o mount nao e registrado.

> [!SUCCESS] 11 defeitos do plugin foram corrigidos em 06/09/2026
> A primeira versao deste playbook documentava um plugin em que a familia UMG era inacessivel,
> nenhum Blueprint fora de `/Game/Blueprints/` podia ser spawnado, e erros do servidor chegavam
> como "Timeout". **Tudo isso foi corrigido no C++ do plugin e revalidado por execucao.**
> O historico dos defeitos esta em §10.

Cada linha abaixo foi **executada** contra o editor aberto em 06/09/2026.

### 2.1 Atores e nivel — tudo funciona

| Ferramenta | Observacao |
| :--- | :--- |
| `get_actors_in_level` | Devolveu os 17 atores do nivel |
| `find_actors_by_name` | Sem resultado devolve **saida vazia**, nao erro |
| `get_actor_properties` | Devolve transform, classe **e a lista de componentes** com `class_path` e `component_count` |
| `spawn_actor` | `type` e classe nativa: `PointLight`, `StaticMeshActor` |
| `spawn_blueprint_actor` | Aceita **nome curto ou caminho completo**, e encontra asset em qualquer pasta |
| `set_actor_transform` | Aceita `location`/`rotation`/`scale` parciais |
| `set_actor_property` | `bHidden=true` aplicado e confirmado |
| `delete_actor` | Confirmado. **Sem desfazer** |

### 2.2 Blueprints — tudo funciona

| Ferramenta | Observacao |
| :--- | :--- |
| `create_blueprint` | Cria em `/Game/Blueprints/` e **grava em disco** (`saved_to_disk`) |
| `add_component_to_blueprint` | Aceita **nome curto** (`StaticMeshComponent`) e caminho (`/Script/Engine.StaticMeshComponent`) |
| `set_component_property` | Confirmado |
| `set_blueprint_property` | Exige `compile_blueprint` antes, se a variavel foi criada agora — §2.6 |
| `add_blueprint_variable` | Confirmado |
| `compile_blueprint` | Devolveu `compiled: true` |
| `set_static_mesh_properties` | `/Engine/BasicShapes/Cube.Cube` funciona |
| `set_physics_properties` | Confirmado |

**Fluxo completo validado, sem intervencao humana:**
```
create_blueprint -> add_component_to_blueprint -> compile_blueprint -> spawn_blueprint_actor -> get_actor_properties
```
Antes isso era impossivel: o spawn nao achava o asset recem-criado porque nada gravava em disco.

### 2.3 Grafo de Blueprint — funciona

`add_blueprint_event_node` (`ReceiveBeginPlay`) · `add_blueprint_function_node` ·
`add_blueprint_input_action_node` · `add_blueprint_self_reference` ·
`add_blueprint_get_self_component_reference` · `connect_blueprint_nodes` (`then` -> `execute`) ·
`create_input_mapping`

> `add_blueprint_function_node` so aceita funcoes que o `target` **realmente possui**:
> `PrintString` em `self` falha porque e estatica de `UKismetSystemLibrary`;
> `SetActorHiddenInGame` funciona.

### 2.4 UMG — funciona

| Ferramenta | Observacao |
| :--- | :--- |
| `create_umg_widget_blueprint` | Cria, **honra o parametro `path`** e grava em disco |
| `add_text_block_to_widget` | Confirmado no `.uasset`: canvas, nome e texto |
| `add_button_to_widget` | Construido pela WidgetTree, com label |
| `bind_widget_event` | Cria `K2Node_ComponentBoundEvent` — confirmado no asset |
| `add_widget_to_viewport` | ⚠️ **O nome engana**: nao adiciona nada ao viewport. Resolve a classe do widget e diz isso na propria resposta. Para exibir em jogo, use `CreateWidget` + `AddToViewport` no Blueprint |

**Aceita as duas convencoes de nome**: `widget_name` + `text_block_name` (esquema Python) ou
`blueprint_name` + `widget_name` (esquema C++).

### 2.5 O que continua nao existindo

> [!IMPORTANT] Nivel nao se cria nem se salva por MCP
> Nao ha comando para criar, abrir ou salvar mapa. O caminho usado em 06/09/2026 foi um commandlet Python (`PythonScriptPlugin`, habilitado so para o alvo Editor), com o script versionado em `Plugins/Documentos/Sandbox_Framework/scripts/montar_nivel_demo.py`.
>
> Duas armadilhas que esse script teve que aprender:
> 1. **O Asset Registry nao varre conteudo de plugin sozinho no commandlet.** Sem `scan_paths_synchronous([pasta], force_rescan=True)`, o `load_asset` falha com *could not be found in the Asset Registry* mesmo com o arquivo em disco.
> 2. **Blueprint se instancia pela classe gerada.** `spawn_actor_from_object(bp, ...)` devolve `None`; use `load_blueprint_class(caminho)` e `spawn_actor_from_class`.

- **Progress Bar.** O UMG do plugin cria apenas Text Block e Button.
- **Criar ou editar Data Asset.** O dispatch aceita 36 comandos e nenhum cria asset que nao seja
  Blueprint ou Widget. `PawnData`, `ComponentSet` e configs sao trabalho manual no editor.
- **Importar asset, criar material, malha, animacao ou nivel.**
- **Desfazer.**
- **Rodar a suite de testes** — e linha de comando, §8.
- **Apagar asset.** So ator. Asset e pelo Content Browser.

> Seis comandos existem no C++ mas **nao estao expostos** como ferramenta: `ping`,
> `focus_viewport`, `take_screenshot`, `create_actor`, `set_pawn_properties` e
> `add_blueprint_get_component_node`. `take_screenshot` seria o mais util — daria verificacao
> visual, que hoje nao existe.

### 2.6 Ordem obrigatoria: variavel antes de propriedade

`add_blueprint_variable` devolve sucesso, mas a propriedade **nao existe no CDO** ate recompilar:

```
add_blueprint_variable(variable_name="TestHealth", variable_type="Float")  -> success
set_blueprint_property(property_name="TestHealth", ...)                    -> "Property not found"
compile_blueprint(...)                                                     -> compiled: true
set_blueprint_property(property_name="TestHealth", ...)                    -> success
```

### 2.7 `find_blueprint_nodes` continua insatisfazivel

O schema expoe `event_type`; o C++ exige `event_name`, que nao e exposto. Nao corrigido — nao
apareceu necessidade real de uso.

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

> [!IMPORTANT] A causa foi corrigida, mas o hábito continua valendo
> O "Timeout receiving Unreal response" era um defeito do próprio plugin: a resposta era enviada
> com o comprimento em `TCHAR` sobre um buffer UTF-8, então qualquer caractere fora do ASCII a
> truncava — e faltava o terminador de linha. A mensagem de erro do plugin contém um travessão,
> o que **garantia** corrupção justamente ao relatar erro. Corrigido em 06/09/2026.
>
> Ainda assim, ao ver timeout, **leia o log antes de concluir qualquer coisa**. Foi lendo o log
> que essa causa apareceu, e nenhuma mensagem do cliente dava pista dela.

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

---

## 10. Histórico — os 11 defeitos corrigidos em 06/09/2026

Registrado porque o **padrão** vale mais que a lista: os defeitos estavam em camadas, e cada
correção só tornava a seguinte alcançável.

| # | Defeito | Onde |
| :-: | :--- | :--- |
| 1 | Resposta enviada com comprimento em `TCHAR` sobre buffer UTF-8 — truncava | `MCPServerRunnable.cpp` |
| 2 | Faltava terminador de linha num dos caminhos de envio | `MCPServerRunnable.cpp` |
| 3 | Caminhos fixos `/Game/Blueprints/` e `/Game/Widgets/` | 4 arquivos |
| 4 | `spawn_blueprint_actor` exigia asset em disco; `create_blueprint` não salvava | `EditorCommands`, `BlueprintCommands` |
| 5 | `component_type` rejeitava nome curto, contra a própria docstring | `BlueprintCommands.cpp` |
| 6 | `get_actor_properties` não listava componentes | `CommonUtils.cpp` |
| 7 | Nomes de parâmetro divergentes entre Python e C++ na família UMG | `UMGCommands.cpp` |
| 8 | `create_umg_widget_blueprint` pedia `UBlueprint` em vez de `UWidgetBlueprint` | `UMGCommands.cpp` |
| 9 | Widgets não registrados em `WidgetVariableNameToGuidMap` — `ensure` do compilador | `UMGCommands.cpp` |
| 10 | Botão criado com `NewObject` sobre o CDO, fora da `WidgetTree` | `UMGCommands.cpp` |
| 11 | `bind_widget_event` passava `nullptr` como propriedade; widgets sem `bIsVariable` | `UMGCommands.cpp` |
| 12 | `SetObjectProperty` só escrevia bool/int/float/string/enum — nem `FText` passava | `CommonUtils.cpp` |
| 13 | Pacote novo nascia parcialmente carregado: fora de `/Game` o save era recusado e o comando ainda dizia `success` | `DataAssetCommands.cpp` |
| 14 | `create_blueprint` procurava a classe pai so em `/Script/Engine` e `/Script/Game`; ao falhar, caia em `AActor` **em silencio** | `BlueprintCommands.cpp` |
| 15 | `set_blueprint_property` e `compile_blueprint` marcavam como modificado e **nao gravavam** | `BlueprintCommands.cpp` |

### O que isso ensina

**Ler o código encontrou o defeito 7. Executar encontrou os outros dez.**

A primeira análise deste plugin foi feita lendo os schemas e o C++, e concluiu "a família UMG
está quebrada por divergência de nomes de parâmetro". Estava correto — e era a **primeira de
cinco camadas**. Abaixo dela estavam as classes erradas no `CreateBlueprint`, o mapa de GUIDs, o
`NewObject` sobre o CDO e o `nullptr` no bind. Nenhuma era visível enquanto a anterior barrava a
execução no primeiro passo.

**Três respostas de sucesso eram mentira.** `add_button_to_widget`, `bind_widget_event` e
`set_text_block_binding` devolviam `{"status":"success","result":{"error":"..."}}`. Um chamador
que lê só o `status` conclui que funcionou. Por isso a regra de verificar por estado não é
zelo excessivo — é o que separa "funcionou" de "respondeu".

**Como verificar por estado, na prática:**

| O que você fez | Verificação real |
| :--- | :--- |
| Spawnou ator | `get_actor_properties` e conferir `components` |
| Criou/alterou asset | Ler o `.uasset` no disco, ou reabrir no editor |
| Qualquer coisa | Ler `Saved/Logs/GameAnimationSample.log`, filtro `MCPServerRunnable` |

> Os widgets desta validação foram conferidos extraindo as strings do `.uasset` — foi assim que
> se confirmou o nó `BndEvt__WBP_ValProbe3_BTN_Confirmar_K2Node_ComponentBoundEvent_0_...`, que
> nenhuma resposta de ferramenta teria provado.

### Limitação que nenhuma correção remove

`spawn_blueprint_actor` de um `SBCharacter` devolve **só os componentes de engine** — capsule,
movimento, malha, câmera. Nenhum componente do framework aparece.

Não é defeito: `ASBCharacter::InitializeFromPawnData` roda em `BeginPlay`/`PossessedBy`, que não
acontecem no mundo do editor. **Composição do framework só se verifica em PIE**, e concluir pela
lista do editor que o PawnData está quebrado seria erro de leitura.
