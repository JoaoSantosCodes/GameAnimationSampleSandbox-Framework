# 🎮 Playbook de Execução via Unreal MCP

**Para**: uma sessão do Claude com o servidor `unrealMCP` conectado e o Unreal Editor **aberto**
no projeto `D:\Unreal\GameAnimationSample`.
**Propósito**: executar tarefas dentro do editor sem abrir a UI manualmente, com verificação a
cada passo.
**Levantado em**: 06/09/2026, lendo os schemas das 30 ferramentas e o código-fonte do framework.

> [!WARNING] Leia §2 e §5 antes de planejar qualquer coisa
> §2 lista o que as ferramentas **conseguem** fazer — o conjunto é pequeno, e o que não está lá
> não existe. §5 descreve como este framework **espera** ser montado; ignorar isso produz um
> personagem com componentes duplicados que compila, spawna e se comporta de forma errática.
>
> Planejar contra o que o MCP não suporta desperdiça a sessão e deixa o editor num estado
> meio-feito, sem desfazer.

---

## 1. Como usar este documento

1. Rode o **teste de conexão** (§3). Se falhar, pare.
2. Leia a **missão** que o usuário pediu (§7). Se não houver missão correspondente, verifique em
   §2 se as ferramentas cobrem o pedido **antes** de começar.
3. Execute **uma chamada de escrita por vez**, verificando o resultado (§6).
4. Feche a sessão pelo protocolo de §9 — em especial, avisando o que **não** foi salvo.

---

## 2. Referência completa das ferramentas

30 ferramentas, agrupadas. Assinaturas exatas, com valores padrão. **Parâmetro que não está
aqui não existe.**

### 2.1 Atores e nível

| Ferramenta | Parâmetros | Notas |
| :--- | :--- | :--- |
| `get_actors_in_level` | *(nenhum)* | Teste de conexão canônico |
| `find_actors_by_name` | `pattern` | Busca por padrão de nome |
| `spawn_actor` | `name`, `type`, `location=[0,0,0]`, `rotation=[0,0,0]` | `type` é classe **nativa** (`StaticMeshActor`, `PointLight`). Para Blueprint use `spawn_blueprint_actor` |
| `spawn_blueprint_actor` | `blueprint_name`, `actor_name`, `location=[0,0,0]`, `rotation=[0,0,0]` | |
| `delete_actor` | `name` | **Sem desfazer** |
| `set_actor_transform` | `name`, `location=None`, `rotation=None`, `scale=None` | Só o que for passado muda |
| `set_actor_property` | `name`, `property_name`, `property_value` | `property_value` é **string**, sempre |
| `get_actor_properties` | `name` | A verificação mais importante do playbook |

### 2.2 Blueprints

| Ferramenta | Parâmetros | Notas |
| :--- | :--- | :--- |
| `create_blueprint` | `name`, `parent_class` | `parent_class` **sem prefixo** (`SBCharacter`, não `ASBCharacter`) |
| `add_component_to_blueprint` | `blueprint_name`, `component_type`, `component_name`, `location=[]`, `rotation=[]`, `scale=[]`, `component_properties={}` | `component_type` sem prefixo `U`. ⚠️ **Ver §5 antes de usar em `SBCharacter`** |
| `set_component_property` | `blueprint_name`, `component_name`, `property_name`, `property_value` | `property_value` string |
| `set_blueprint_property` | `blueprint_name`, `property_name`, `property_value` | Atua no Class Default Object |
| `add_blueprint_variable` | `blueprint_name`, `variable_name`, `variable_type`, `is_exposed=False` | Tipos: `Boolean`, `Integer`, `Float`, `Vector`, … |
| `compile_blueprint` | `blueprint_name` | **Obrigatório após toda alteração estrutural** |
| `set_static_mesh_properties` | `blueprint_name`, `component_name`, `static_mesh="/Engine/BasicShapes/Cube.Cube"` | |
| `set_physics_properties` | `blueprint_name`, `component_name`, `simulate_physics=True`, `gravity_enabled=True`, `mass=1.0`, `linear_damping=0.01`, `angular_damping=0.0` | |

### 2.3 Grafo de Blueprint

| Ferramenta | Parâmetros | Notas |
| :--- | :--- | :--- |
| `add_blueprint_event_node` | `blueprint_name`, `event_name`, `node_position=None` | Eventos padrão usam prefixo `Receive`: `ReceiveBeginPlay`, `ReceiveTick` |
| `add_blueprint_function_node` | `blueprint_name`, `target`, `function_name`, `params=None`, `node_position=None` | `target` é nome de componente ou `self` |
| `add_blueprint_input_action_node` | `blueprint_name`, `action_name`, `node_position=None` | |
| `add_blueprint_self_reference` | `blueprint_name`, `node_position=None` | Nó "Get Self" |
| `add_blueprint_get_self_component_reference` | `blueprint_name`, `component_name`, `node_position=None` | Equivale a arrastar o componente do painel |
| `connect_blueprint_nodes` | `blueprint_name`, `source_node_id`, `source_pin`, `target_node_id`, `target_pin` | IDs vêm do retorno dos `add_*_node` |
| `find_blueprint_nodes` | `blueprint_name`, `node_type=None`, `event_type=None` | Use para recuperar IDs se perdeu o retorno |

> Montar grafo por MCP é **caro e frágil**: cada nó é uma chamada, e conectar exige guardar os
> IDs retornados. Para lógica não trivial, é mais barato pedir ao usuário para fazer no editor,
> ou implementar em C++ — que é onde a lógica deste projeto vive.

### 2.4 UMG

| Ferramenta | Parâmetros | Notas |
| :--- | :--- | :--- |
| `create_umg_widget_blueprint` | `widget_name`, `parent_class="UserWidget"`, `path="/Game/UI"` | |
| `add_text_block_to_widget` | `widget_name`, `text_block_name`, `text=""`, `position=[0,0]`, `size=[200,50]`, `font_size=12`, `color=[1,1,1,1]` | |
| `add_button_to_widget` | `widget_name`, `button_name`, `text=""`, `position=[0,0]`, `size=[200,50]`, `font_size=12`, `color=[1,1,1,1]`, `background_color=[0.1,0.1,0.1,1]` | |
| `bind_widget_event` | `widget_name`, `widget_component_name`, `event_name`, `function_name=""` | Cria a função se não existir; padrão `{componente}_{evento}` |
| `set_text_block_binding` | `widget_name`, `text_block_name`, `binding_property`, `binding_type="Text"` | Exige que a propriedade/função de binding **já exista** |
| `add_widget_to_viewport` | `widget_name`, `z_order=0` | |

### 2.5 Input

| Ferramenta | Parâmetros | Notas |
| :--- | :--- | :--- |
| `create_input_mapping` | `action_name`, `key`, `input_type="Action"` | Sistema de input **legado**. O projeto usa Enhanced Input (`IA_*` em `/Game/SandboxFramework/Input/`) — ver §5.4 |

### 2.6 O que não existe

> [!CAUTION] Verifique aqui antes de prometer qualquer coisa ao usuário
> - **Progress Bar.** O UMG disponível cria apenas *Text Block* e *Button*. Qualquer HUD de
>   barra — vida, estamina, mana — **não pode** ser montado por MCP.
> - **Criar Data Assets.** Não há ferramenta para criar `USBPawnDataAsset`, `ComponentSet`,
>   `DA_MovementConfig` etc. Só dá para *referenciar* os que já existem.
> - **Importar assets, criar materiais, malhas, animações ou níveis.**
> - **Salvar.** Nem pacote, nem nível. O que for criado vive na sessão do editor; salvar é ação
>   do usuário.
> - **Ler o log do editor.** Se uma chamada devolver sucesso mas o resultado for errado, o MCP
>   não conta. Verifique pelo **estado**, nunca pela resposta.
> - **Desfazer.** Não há undo. `delete_actor` é definitivo.
> - **Rodar a suíte de testes.** Isso é linha de comando, fora do MCP — ver §9.

---

## 3. Pré-condições e teste de conexão

**Primeira chamada de toda sessão, sem exceção:**

```
get_actors_in_level()
```

| Resultado | Significado | Ação |
| :--- | :--- | :--- |
| Lista de atores | Canal vivo, nível carregado | Prosseguir |
| Lista vazia | Editor aberto mas sem nível, ou nível vazio | Confirmar com o usuário antes de escrever |
| Erro / timeout | Editor fechado, compilando, ou em diálogo modal | **Parar.** Pedir ao usuário para verificar. Não repetir em laço |

O editor não abre com C++ quebrado — se ele abriu, os módulos compilaram. Não há necessidade de
verificar compilação separadamente.

---

## 4. Regras de execução

1. **Uma chamada de escrita por vez, seguida de verificação.** `spawn_blueprint_actor` devolve
   sucesso mesmo quando o ator entra sem o componente esperado.
2. **`compile_blueprint` após toda alteração estrutural.** Variável, componente ou nó novo não
   recompila sozinho, e um Blueprint não compilado se comporta como a versão anterior — falha
   silenciosa clássica.
3. **Nunca `delete_actor` sem listar antes.** Nome parecido apaga o ator errado, sem desfazer.
4. **Nunca invente nome de classe.** Use §5.2. Errar em `parent_class` ou `component_type` faz a
   chamada falhar ou criar algo com a classe errada.
5. **`property_value` é sempre string**, mesmo para número ou booleano. Passe `"true"`, `"100.0"`.
6. **Reporte o que criou, com caminho no Content Browser.** O usuário precisa saber o que
   apareceu para poder salvar ou descartar.
7. **Se um passo de verificação falhar, pare e reporte.** Não siga construindo sobre uma base
   que não se confirmou — este projeto tem histórico documentado de trabalho empilhado sobre
   fundação não verificada (ver [[audit_report_conformidade_2026-09-05]]).

---

## 5. Como este framework espera ser montado

Ler isto evita o erro mais caro possível nesta base de código.

### 5.1 Composição por PawnData, não por componentes no Blueprint

`ASBCharacter` deriva de `AModularCharacter` e **não cria nenhum componente no construtor**. A
composição acontece em tempo de execução, em `ASBCharacter::InitializeFromPawnData()`:

```cpp
if (PawnData->ComponentSet)
{
    USBComponentFactory::InitializeComponentsFromSet(this, PawnData->ComponentSet);
}
CachedAttributeComponent = FindComponentByClass<USBAttributeComponent>();
CachedStateComponent     = FindComponentByClass<USBStateComponent>();
CachedAbilityComponent   = FindComponentByClass<USBAbilityComponent>();
```

> [!DANGER] Não adicione componentes do framework a um Blueprint de `SBCharacter`
> Se o Blueprint já tiver um `SBAttributeComponent` e o `ComponentSet` do PawnData adicionar
> outro, o personagem fica com **dois**. `FindComponentByClass` devolve o primeiro que encontrar,
> e a ordem não é garantida. O resultado é um personagem com dois conjuntos de atributos, onde
> um é escrito e o outro é lido — sem erro, sem log, sem crash.
>
> **A forma correta** de mudar a composição é editar o `ComponentSet` do PawnData no editor.
> O MCP **não** cria nem edita Data Assets (§2.6).

Um personagem sem PawnData registra `No PawnData set on character X!` como Warning e fica sem
componente nenhum. Se você spawnar um `SBCharacter` e `get_actor_properties` não listar
componentes, é isso.

### 5.2 Classes utilizáveis

Todas compiladas e cobertas pela suíte (446 specs verdes em 06/09/2026).

| Papel | `parent_class` / `component_type` | Onde vive |
| :--- | :--- | :--- |
| Personagem | `SBCharacter` | `05_SandboxCharacter` |
| Controlador de IA | `SBAIController` | `06_SandboxCombat` |
| Atributos | `SBAttributeComponent` | `05_SandboxCharacter` |
| Estado por tags | `SBStateComponent` | `05_SandboxCharacter` |
| Movimento | `SBMovementComponent` | `05_SandboxCharacter` |
| Habilidades | `SBAbilityComponent` | `05_SandboxCharacter` |
| Efeitos de status | `SBStatusEffectComponent` | `05_SandboxCharacter` |
| Combate | `SBCombatComponent` | `06_SandboxCombat` |
| Interação | `SBInteractionComponent` | `07_SandboxInteraction` |
| Inventário | `SBInventoryComponent` | `08_SandboxInventory` |
| HUD de status | `SBStatusHUDWidget` | `09_SandboxUI` — ⚠️ ver §7.1 |

### 5.3 Tags de atributo reais

Nomes exatos, de `SBGameplayTags.cpp`. Errar a string faz o atributo não ser encontrado, sem
erro.

`Attribute.Health` · `Attribute.MaxHealth` · `Attribute.Stamina` · `Attribute.Mana` ·
`Attribute.Speed` · `Attribute.Weapon.Ammo` · `Attribute.MaxWeight`

> Note que **não existem** `Attribute.MaxStamina` nem `Attribute.MaxMana`. O teto desses vive em
> `FSBAttribute::MaxValue` e é lido por `ISBAttributeComponentInterface::GetAttributeMaxValue`.

### 5.4 Assets que já existem

`/Game/SandboxFramework/`

| Caminho | Conteúdo |
| :--- | :--- |
| `Blueprints/BP_SBCharacter_Hero` | Personagem jogável do framework |
| `Blueprints/BP_SBGameMode_Test` | GameMode de teste |
| `Blueprints/BP_BauDeTeste` | Baú, para testar inventário/interação |
| `Blueprints/BP_Ability_Teleport` | Habilidade de exemplo |
| `Data/Pawn/DA_HeroPawnData`, `DA_HeroPawnDataV2` | PawnData — define componentes, malha, HUD |
| `Data/Pawn/DA_ComponentSet_Hero`, `DA_ComponentSet_HeroV2` | Conjuntos de componentes |
| `Data/Movement/DA_MovementConfig`, `DA_Movement_Sprint`, `DA_Movement_Crouch` | Config de movimento |
| `Data/Camera/DA_Camera_Walk`, `_Sprint`, `_Aim` | Modos de câmera |
| `Data/Ability/DA_AbilitySet_Hero`, `DA_Def_Teleport` | Habilidades |
| `Data/Weapon/DA_RifleWeapon` | Arma |
| `Data/StatusEffect/DA_StatusEffect_Veneno` | Efeito de status |
| `Data/Input/DA_InputConfig` | Config de input |
| `Input/IA_SB_Sprint`, `IA_SB_Crouch`, `IA_SB_Interact`, `IA_Attack`, `IA_Fire`, `IA_Ability1` | Enhanced Input Actions |
| `Widgets/USBStatusHUDWidget` | ⚠️ **vazio** — ver §7.1 |

> O projeto usa **Enhanced Input** (`IA_*`), enquanto `create_input_mapping` cria mapeamento do
> sistema **legado**. Os dois coexistem sem conflito, mas um mapeamento legado **não** aciona os
> `IA_*` existentes. Só use `create_input_mapping` para protótipo isolado.

---

## 6. Protocolo de verificação

Para cada tipo de escrita, a verificação correspondente. **Resposta de sucesso não é prova.**

| O que você fez | Como verificar | Sinal de falha |
| :--- | :--- | :--- |
| `create_blueprint` | `compile_blueprint` no mesmo nome | Erro de compilação, ou nome não encontrado |
| `add_component_to_blueprint` | `compile_blueprint`, depois spawnar e `get_actor_properties` | Componente ausente na lista |
| `spawn_blueprint_actor` | `get_actor_properties(name=<actor_name>)` | Ator ausente, ou sem os componentes esperados |
| `spawn_actor` | `find_actors_by_name(pattern=<name>)` | Não encontrado |
| `set_actor_property` | `get_actor_properties` e conferir o valor | Valor inalterado — geralmente nome de propriedade errado |
| `delete_actor` | `find_actors_by_name` com o mesmo padrão | Ainda presente |
| `create_umg_widget_blueprint` | `compile_blueprint` no nome do widget | Falha de compilação |
| `add_text_block_to_widget` | `compile_blueprint`, e pedir ao usuário para olhar o Designer | Sem verificação programática disponível |
| `add_widget_to_viewport` | Pedir confirmação visual ao usuário | Nada aparece: z-order ou widget vazio |

---

## 7. Missões

### 7.1 HUD de status — o asset está vazio

**Diagnóstico feito em 06/09/2026, não presumido.** `Content/SandboxFramework/Widgets/
USBStatusHUDWidget.uasset` deriva da classe C++ `SBStatusHUDWidget`, mas **não contém widget
nenhum**. Os três `UProgressBar` que o C++ espera — `PB_Health`, `PB_Mana`, `PB_Stamina` — estão
declarados com `meta = (BindWidget, OptionalWidget = true)` e não existem no Blueprint. Como são
opcionais, nada reclama: em jogo a HUD não mostra nada, e nunca mostrou.

O lado C++ está pronto e eficiente: lê os três atributos por `ISBAttributeComponentInterface` a
30 Hz por timer, sem alocação por frame.

> [!IMPORTANT] O MCP **não** termina esta missão
> Não há ferramenta de Progress Bar (§2.6). As barras precisam ser criadas à mão no editor, com
> os nomes **exatos** `PB_Health`, `PB_Mana` e `PB_Stamina` — `BindWidget` casa por nome, e
> qualquer divergência resulta em barra nula silenciosa, que é o estado atual.

**O que o MCP consegue**: um readout em texto, útil como diagnóstico enquanto a UI real não
existe.

```
create_umg_widget_blueprint(widget_name="WBP_SBStatusReadout", parent_class="UserWidget", path="/Game/SandboxFramework/Widgets")

add_text_block_to_widget(widget_name="WBP_SBStatusReadout", text_block_name="TXT_Health",  text="Health: --",  position=[40, 40],  size=[300, 40], font_size=20, color=[1.0, 0.25, 0.25, 1.0])
add_text_block_to_widget(widget_name="WBP_SBStatusReadout", text_block_name="TXT_Mana",    text="Mana: --",    position=[40, 84],  size=[300, 40], font_size=20, color=[0.3, 0.5, 1.0, 1.0])
add_text_block_to_widget(widget_name="WBP_SBStatusReadout", text_block_name="TXT_Stamina", text="Stamina: --", position=[40, 128], size=[300, 40], font_size=20, color=[0.4, 1.0, 0.4, 1.0])

compile_blueprint(blueprint_name="WBP_SBStatusReadout")
add_widget_to_viewport(widget_name="WBP_SBStatusReadout", z_order=10)
```

Os textos ficam **estáticos**. `set_text_block_binding` exige que a função de binding já exista,
e criá-la por MCP significa montar o grafo nó a nó (§2.3) — caro e frágil.

**Ao encerrar**: diga que o readout é diagnóstico, que as barras reais continuam pendentes de
trabalho manual, e liste os três nomes exatos que elas precisam ter.

### 7.2 Validar o personagem do framework no editor

Confirma no editor o que a suíte confirma em memória: que a composição por PawnData funciona.

```
get_actors_in_level()
spawn_blueprint_actor(blueprint_name="BP_SBCharacter_Hero", actor_name="SBHero_Test01", location=[0, 0, 200])
get_actor_properties(name="SBHero_Test01")
```

**O passo que importa é o terceiro.** Procure na resposta por `SBAttributeComponent`,
`SBStateComponent`, `SBMovementComponent`.

| Resultado | Significado |
| :--- | :--- |
| Componentes presentes | PawnData e ComponentSet estão corretos ✅ |
| Nenhum componente do framework | PawnData não está setado no Blueprint — reporte, **não tente consertar adicionando componentes** (§5.1) |
| Componentes duplicados | O Blueprint tem componentes **e** o ComponentSet adiciona os mesmos. Defeito real: reporte |

### 7.3 Cenário de teste de interação

Usa o baú já existente para exercitar inventário e interação juntos.

```
spawn_blueprint_actor(blueprint_name="BP_SBCharacter_Hero", actor_name="SBHero_Test01", location=[0, 0, 200])
spawn_blueprint_actor(blueprint_name="BP_BauDeTeste",       actor_name="SBBau_Test01",  location=[300, 0, 100])
get_actor_properties(name="SBBau_Test01")
find_actors_by_name(pattern="SBHero_Test01")
```

300 unidades de distância ficam dentro do alcance típico de interação sem sobrepor as cápsulas
de colisão. Colocar os dois na origem é o erro que fez três suítes deste projeto falharem por
motivo errado — ver [[audit_report_conformidade_2026-09-05]] §14.1.

### 7.4 Ator de teste físico

Quando precisar de um objeto simples para testar colisão, empurrão ou queda.

```
create_blueprint(name="BP_TestPhysicsCube", parent_class="Actor")
add_component_to_blueprint(blueprint_name="BP_TestPhysicsCube", component_type="StaticMeshComponent", component_name="Mesh")
set_static_mesh_properties(blueprint_name="BP_TestPhysicsCube", component_name="Mesh", static_mesh="/Engine/BasicShapes/Cube.Cube")
set_physics_properties(blueprint_name="BP_TestPhysicsCube", component_name="Mesh", simulate_physics=True, gravity_enabled=True, mass=50.0)
compile_blueprint(blueprint_name="BP_TestPhysicsCube")
spawn_blueprint_actor(blueprint_name="BP_TestPhysicsCube", actor_name="TestCube_01", location=[500, 0, 400])
get_actor_properties(name="TestCube_01")
```

Aqui `add_component_to_blueprint` é seguro: `Actor` puro não tem PawnData, então não há conflito
com o §5.1.

### 7.5 Limpeza após testar

```
find_actors_by_name(pattern="SBHero_Test")
find_actors_by_name(pattern="SBBau_Test")
find_actors_by_name(pattern="TestCube_")
```

Confira a lista, **depois** apague um por um por nome exato. Nunca apague por padrão sem ler o
que ele casou — `delete_actor` não desfaz.

---

## 8. Modos de falha conhecidos

| Sintoma | Causa provável | O que fazer |
| :--- | :--- | :--- |
| Timeout na chamada | Editor fechado, compilando, ou em diálogo modal | Pedir ao usuário para verificar. **Não repetir em laço** |
| `create_blueprint` falha | `parent_class` com prefixo (`ASBCharacter`) ou classe inexistente | Nome sem prefixo (§5.2) |
| `add_component_to_blueprint` falha | `component_type` com prefixo `U` | Remover o `U` |
| Componente não aparece após spawn | Blueprint não recompilado | `compile_blueprint`, apagar o ator e spawnar de novo |
| Personagem spawna sem componente nenhum | PawnData não setado | Reportar. **Não** adicionar componentes à mão (§5.1) |
| Personagem com componentes duplicados | Blueprint **e** ComponentSet adicionam os mesmos | Reportar como defeito |
| `set_actor_property` "funciona" mas nada muda | Nome de propriedade errado | Conferir com `get_actor_properties` |
| Widget criado mas invisível | Falta `add_widget_to_viewport`, z-order baixo, ou widget vazio | Conferir os três |
| Barra do HUD sempre vazia | Nome divergente do `BindWidget` | `PB_Health` / `PB_Mana` / `PB_Stamina`, exatos |
| Tudo some ao fechar o editor | Nada foi salvo — o MCP não salva | Avisar o usuário |

---

## 9. Encerramento de sessão

Antes de dizer que terminou:

1. **Liste o que foi criado**, com caminho no Content Browser e nome de ator no nível.
2. **Diga explicitamente que nada foi salvo.** O MCP não salva pacote nem nível; fechar o editor
   descarta tudo.
3. **Liste o que ficou pendente** e por quê — especialmente o que o MCP não consegue fazer.
4. **Não afirme que funciona em jogo.** O MCP monta estrutura; comportamento só se verifica
   jogando ou por teste automatizado, e nenhum dos dois passa por aqui.

> [!IMPORTANT] A regra que este projeto aprendeu da pior forma
> Uma auditoria em 05/09/2026 encontrou 14 fases inteiras marcadas como "concluídas, compiladas
> e homologadas, 100% verde" cujo código **nunca tinha passado pelo compilador** — estava em
> pastas fora da árvore de build. Depois disso, 166 asserções de teste foram descobertas
> inertes, verificando nada.
>
> Relatar mais do que se verificou é o defeito recorrente desta base de código. Ao encerrar,
> descreva **o que você observou**, não o que deveria ter acontecido.
> Ver [[audit_report_conformidade_2026-09-05]] e [[validation_report_2026-09-06]].

**Rodar a suíte de testes não é tarefa de MCP.** É linha de comando, com o editor fechado:

```powershell
& "D:\Unreal\Unreal Sistema\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" `
  "D:\Unreal\GameAnimationSample\GameAnimationSample.uproject" `
  -NullRHI -NoSound -NoSplash -stdout -unattended -nopause `
  -ExecCmds="Automation RunTest Sandbox; Quit" -log
```
