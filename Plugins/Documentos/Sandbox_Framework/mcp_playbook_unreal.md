# 🎮 Playbook de Execução via Unreal MCP

**Para**: uma sessão do Claude com o servidor `unrealMCP` conectado e o Unreal Editor **aberto**
no projeto `D:\Unreal\GameAnimationSample`.
**Propósito**: executar tarefas dentro do editor sem abrir a UI manualmente, com verificação a
cada passo.

> [!WARNING] Leia os limites antes de planejar qualquer coisa
> O conjunto de ferramentas MCP é **pequeno e específico**. Planejar uma tarefa que ele não
> suporta e só descobrir isso na terceira chamada desperdiça a sessão e deixa o editor num
> estado meio-feito. A seção **§2 Limites reais** lista o que existe. O que não está lá, não
> existe — não presuma equivalentes.

---

## 1. Pré-condições

Nada abaixo funciona sem isto. Verifique **antes** de qualquer chamada de escrita.

| # | Condição | Como verificar |
| :-: | :--- | :--- |
| 1 | Editor aberto no projeto certo | `get_actors_in_level` — se falhar, o editor não está pronto |
| 2 | Módulos C++ compilados | O editor não abre com C++ quebrado; se abriu, compilou |
| 3 | Nível carregado | `get_actors_in_level` deve listar atores do nível atual |

**Teste de conexão canônico** — sempre a primeira chamada da sessão:

```
get_actors_in_level()
```

Se devolver uma lista, o canal está vivo. Se der erro ou timeout, **pare** e peça ao usuário
para abrir o editor. Não tente contornar.

---

## 2. Limites reais do conjunto de ferramentas

Levantado lendo os schemas em 06/09/2026. **É isto e nada mais.**

### Atores e nível
`get_actors_in_level` · `find_actors_by_name` · `spawn_actor` · `spawn_blueprint_actor` ·
`delete_actor` · `set_actor_transform` · `set_actor_property` · `get_actor_properties`

### Blueprints
`create_blueprint(name, parent_class)` · `add_component_to_blueprint` · `set_component_property` ·
`set_blueprint_property` · `add_blueprint_variable` · `compile_blueprint` ·
`set_physics_properties` · `set_static_mesh_properties`

### Grafo de Blueprint
`add_blueprint_event_node` · `add_blueprint_function_node` · `add_blueprint_input_action_node` ·
`add_blueprint_self_reference` · `add_blueprint_get_self_component_reference` ·
`connect_blueprint_nodes` · `find_blueprint_nodes`

### UMG
`create_umg_widget_blueprint(widget_name, parent_class, path)` ·
`add_text_block_to_widget` · `add_button_to_widget` · `bind_widget_event` ·
`set_text_block_binding` · `add_widget_to_viewport`

### Input
`create_input_mapping`

> [!CAUTION] O que **não** existe, e as consequências
> - **Não há ferramenta para Progress Bar.** O UMG disponível cria apenas *Text Block* e
>   *Button*. Qualquer HUD de barra — vida, estamina, mana — **não pode** ser montado por MCP.
>   Ou se faz leitura em texto, ou se monta a barra à mão no editor.
> - **Não há ferramenta para importar assets, criar materiais, malhas ou animações.**
> - **Não há ferramenta para salvar o nível ou o pacote.** O que for criado existe na sessão do
>   editor; salvar é ação do usuário.
> - **Não há leitura de log do editor.** Se uma chamada devolver sucesso mas o resultado no
>   editor for errado, o MCP não vai contar. Verifique pelo estado, não pela resposta.

---

## 3. Regras de execução

1. **Uma chamada de escrita por vez, seguida de verificação.** `spawn_blueprint_actor` devolve
   sucesso mesmo quando o ator entra sem o componente esperado. Confirme com
   `get_actor_properties` ou `find_actors_by_name`.
2. **`compile_blueprint` depois de toda alteração estrutural.** Adicionar variável, componente
   ou nó não recompila sozinho. Um Blueprint não compilado se comporta como a versão anterior —
   falha silenciosa clássica.
3. **Nunca `delete_actor` sem antes listar.** Nome parecido apaga o ator errado, e não há
   desfazer pelo MCP.
4. **Não invente nomes de classe.** `parent_class` e `component_type` usam o nome sem prefixo
   (`SBCharacter`, não `ASBCharacter`; `SBInventoryComponent`, não `USBInventoryComponent`). Se
   errar, a chamada falha ou cria algo com a classe errada.
5. **Reporte o que foi criado, com nome e caminho.** O usuário precisa saber o que apareceu no
   Content Browser para poder salvar ou descartar.

---

## 4. Classes do framework disponíveis

Para `parent_class` e `component_type`. Todas compiladas e cobertas pela suíte (446 specs).

| Papel | Classe | Observação |
| :--- | :--- | :--- |
| Personagem | `SBCharacter` | Base do framework, com pilha de comportamentos |
| Controlador de IA | `SBAIController` | Agro, foco, fases de boss, Smart Objects |
| Atributos | `SBAttributeComponent` | Health, Mana, Stamina, MaxHealth, MaxWeight |
| Estado por tags | `SBStateComponent` | Contrato `ISBStateComponentInterface` |
| Movimento | `SBMovementComponent` | Sprint, crouch, estamina |
| Habilidades | `SBAbilityComponent` | Cooldowns replicados |
| Combate | `SBCombatComponent` | Armas, agro, `ISBCombatComponentInterface` |
| Inventário | `SBInventoryComponent` | Replicado, `ISBInventoryComponentInterface` |
| Interação | `SBInteractionComponent` | Foco e hold |
| HUD de status | `SBStatusHUDWidget` | ⚠️ ver §5 |

---

## 5. Missão 1 — HUD de status: o asset está vazio

**Diagnóstico feito em 06/09/2026, não presumido.** O asset
`Content/SandboxFramework/Widgets/USBStatusHUDWidget.uasset` deriva da classe C++
`SBStatusHUDWidget`, mas **não contém widget nenhum**: os três `UProgressBar` que o C++ espera
(`PB_Health`, `PB_Mana`, `PB_Stamina`) estão declarados como `OptionalWidget = true` e não
existem no Blueprint. Em jogo, a HUD não mostra nada — e nunca mostrou.

O C++ está pronto: lê os atributos por `ISBAttributeComponentInterface` a 30 Hz por timer, sem
alocação. O que falta é a UI.

> [!IMPORTANT] O MCP **não** consegue terminar esta missão sozinho
> Não há ferramenta de Progress Bar. As barras precisam ser adicionadas à mão no editor, com os
> nomes exatos `PB_Health`, `PB_Mana` e `PB_Stamina` — o `meta = (BindWidget)` casa por nome, e
> qualquer divergência resulta em barra nula silenciosa.

**O que o MCP consegue fazer**: um HUD de leitura em texto, útil como diagnóstico enquanto a UI
real não existe.

```
create_umg_widget_blueprint(widget_name="WBP_SBStatusReadout", parent_class="UserWidget", path="/Game/SandboxFramework/Widgets")

add_text_block_to_widget(widget_name="WBP_SBStatusReadout", text_block_name="TXT_Health",  text="Health: --",  position=[40, 40],  size=[300, 40], font_size=20, color=[1.0, 0.25, 0.25, 1.0])
add_text_block_to_widget(widget_name="WBP_SBStatusReadout", text_block_name="TXT_Mana",    text="Mana: --",    position=[40, 84],  size=[300, 40], font_size=20, color=[0.3, 0.5, 1.0, 1.0])
add_text_block_to_widget(widget_name="WBP_SBStatusReadout", text_block_name="TXT_Stamina", text="Stamina: --", position=[40, 128], size=[300, 40], font_size=20, color=[0.4, 1.0, 0.4, 1.0])

compile_blueprint(blueprint_name="WBP_SBStatusReadout")
add_widget_to_viewport(widget_name="WBP_SBStatusReadout", z_order=10)
```

**Verificação**: o widget deve aparecer no Content Browser em
`/Game/SandboxFramework/Widgets/`. Os textos ficam estáticos — `set_text_block_binding` exige
uma função de binding que só existe se for criada no Blueprint, e criá-la por MCP requer montar
o grafo nó a nó.

**Ao terminar, diga ao usuário**: que o readout é diagnóstico, que as barras reais continuam
pendentes de trabalho manual, e quais nomes exatos elas precisam ter.

---

## 6. Missão 2 — Personagem de teste do framework

Valida no editor o que a suíte valida em memória: que os componentes montam e convivem.

```
create_blueprint(name="BP_SBTestCharacter", parent_class="SBCharacter")

add_component_to_blueprint(blueprint_name="BP_SBTestCharacter", component_type="SBAttributeComponent", component_name="Attributes")
add_component_to_blueprint(blueprint_name="BP_SBTestCharacter", component_type="SBStateComponent",     component_name="State")
add_component_to_blueprint(blueprint_name="BP_SBTestCharacter", component_type="SBMovementComponent",  component_name="SBMovement")
add_component_to_blueprint(blueprint_name="BP_SBTestCharacter", component_type="SBInventoryComponent", component_name="Inventory")

compile_blueprint(blueprint_name="BP_SBTestCharacter")
spawn_blueprint_actor(blueprint_name="BP_SBTestCharacter", actor_name="SBTestChar_01", location=[0, 0, 200])
get_actor_properties(name="SBTestChar_01")
```

**O passo que importa é o último.** `spawn_blueprint_actor` devolvendo sucesso não prova que os
componentes entraram. Se `get_actor_properties` não listar os quatro, **pare e reporte** — não
siga adicionando coisas sobre uma base que não se confirmou.

> `SBCharacter` já cria alguns componentes por código. Se um componente aparecer duplicado,
> **não** adicione o seu: use o que já existe. Duplicata de componente de atributos gera dois
> conjuntos de valores, e o comportamento passa a depender de qual for encontrado primeiro.

---

## 7. Modos de falha conhecidos

| Sintoma | Causa provável | O que fazer |
| :--- | :--- | :--- |
| Chamada dá timeout | Editor fechado, compilando ou num diálogo modal | Pedir ao usuário para verificar; não repetir em laço |
| `create_blueprint` falha | `parent_class` com prefixo (`ASBCharacter`) ou classe inexistente | Usar o nome sem prefixo (§4) |
| Componente não aparece após spawn | Blueprint não recompilado | `compile_blueprint` e spawnar de novo |
| Widget criado mas invisível | Falta `add_widget_to_viewport`, ou z-order abaixo de outro | Conferir z-order |
| Barra do HUD sempre vazia | Nome do widget diverge do `BindWidget` | Renomear para `PB_Health` / `PB_Mana` / `PB_Stamina` exatamente |
| Tudo some ao fechar o editor | Nada foi salvo — o MCP não salva | Avisar o usuário para salvar |

---

## 8. Encerramento de sessão

Antes de dizer que terminou:

1. **Liste o que foi criado**, com caminho no Content Browser.
2. **Diga explicitamente o que não foi salvo** — o MCP não salva pacote nem nível.
3. **Não afirme que funciona em jogo.** O MCP monta a estrutura; comportamento em jogo só se
   verifica jogando ou por teste automatizado. Este projeto tem um histórico documentado de
   afirmações de "100% homologado" que nunca tinham sido medidas — ver
   [[validation_report_2026-09-06]] e [[audit_report_conformidade_2026-09-05]]. Não acrescente
   mais uma.
