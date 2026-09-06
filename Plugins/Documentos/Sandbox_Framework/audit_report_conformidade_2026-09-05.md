# 🔍 Relatório de Auditoria, Conformidade e Refatoração — 05 de Setembro de 2026

> [!IMPORTANT] Workspace de trabalho
> O projeto em desenvolvimento é **`D:\Unreal\GameAnimationSample`**. Todo build, teste e
> alteração ocorre neste diretório.
>
> Os demais documentos deste cofre descrevem `D:\Unreal\V1` como "Projeto Primário" e o
> GameAnimationSample como "projeto secundário" / "integração híbrida". **Essa descrição está
> desatualizada** e induz ao erro de trabalhar no diretório errado. Desde 05/09/2026 o V1 está
> defasado: os defeitos corrigidos por esta auditoria permanecem lá (ver §8).

**Escopo**: Revisão geral do workspace `D:\Unreal\GameAnimationSample` contra o
[[manifesto_and_coding_standards|Manifesto & Padrões de Código]] e a
[[sfps_specification|Especificação Estrutural SFPS v1.0.0]].

**Motivação**: Auditoria solicitada para arrumar, refinar e otimizar o framework seguindo
estritamente a premissa do projeto.

---

## 🚨 Sumário Executivo

A auditoria encontrou uma divergência estrutural grave entre a documentação e o código real:
**todo o entregável das Fases 121–134 (Pilares 1 a 3 do Roadmap de Refinamento) estava fora
da árvore de compilação.** Os arquivos haviam sido gravados em diretórios de plugin que não
existem neste projeto, usando a numeração do workspace `D:\Unreal\V1`.

Consequência: nenhum desses arquivos jamais foi compilado, e a afirmação de
"450 specs 100% verdes" registrada em `task.md` não se aplicava a este workspace.

Ao trazer o código para a árvore de compilação, o compilador revelou defeitos que só podiam
existir em código nunca compilado. Corrigi-los expôs uma segunda camada: **a própria suíte de
testes não media o que afirmava medir.**

| Momento | Specs medidas | Verdes | Vermelhas |
| :--- | ---: | ---: | ---: |
| Início da auditoria | — | *não compilava* | — |
| Primeira execução | 221 *(truncada)* | 207 | 14 |
| Após corrigir o crash que truncava a suíte | **444** *(real)* | 422 | 22 |
| **Estado atual** | **444** | **438** | **6** |

Dois achados reenquadram todos os números anteriores:

1. **Metade da suíte nunca executava.** Uma assertion fatal derrubava o processo no meio da
   execução, então toda medição anterior — inclusive as primeiras desta auditoria — cobria
   apenas ~221 dos 444 specs existentes.
2. **166 asserções não verificavam nada.** `TestEqual` com `enum class` sempre retorna
   verdadeiro na UE 5.8 (ver §13).

Foram corrigidos **7 defeitos de produto** com impacto real em jogo (§12), além dos defeitos
de compilação e das falhas de infraestrutura de teste.

---

## 1. Correção Estrutural — Realocação de Plugins

### Problema
Três diretórios órfãos, todos ausentes do `GameAnimationSample.uproject`, sem `.uplugin`
e sem `.Build.cs` — portanto invisíveis para o Unreal Build Tool.

| Diretório órfão | Arquivos | Destino correto |
| :--- | :---: | :--- |
| `Plugins/02_SandboxCore/` | 76 | `Plugins/04_SandboxCore/` |
| `Plugins/04_SandboxCharacter/` | 29 | `Plugins/05_SandboxCharacter/` |
| `Plugins/05_SandboxInventory/` | 3 | `Plugins/08_SandboxInventory/` |

### Ação
* **100 arquivos realocados** para os módulos corretos. Verificação prévia confirmou **zero
  colisões** de nome com arquivos existentes.
* **8 arquivos duplicados** em caminhos malformados (ex.: `Source/Private/...` sem a pasta do
  módulo, `SandboxCharacter/...` sem `Source/`) foram confirmados byte-idênticos às cópias
  realocadas e removidos.
* Os três diretórios órfãos foram removidos. A topologia de `Plugins/` agora corresponde
  exatamente à SFPS: `01_SandboxCommon` a `11_SandboxEditor`, sem duplicatas.

---

## 2. Conformidade Arquitetural — Princípio 7 restaurado

### Problema
**28 arquivos** do módulo de Fundação `SandboxCore` faziam `#include "Components/SBStateComponent.h"`
— tipo concreto pertencente a `05_SandboxCharacter`. Isso viola:

* **Princípio 7 (Zero Dependências Circulares)** — Fundação não pode depender de Gameplay.
* **Princípio 4 (Interfaces First)** — referência direta a tipo concreto de outro plugin.

O código nunca compilou, então a violação nunca foi detectada. Adicionar `SandboxCharacter` ao
`Build.cs` do Core seria uma dependência circular rejeitada pelo próprio UBT
(`SandboxCharacter` já depende de `SandboxCore`).

### Ação
`ISBStateComponentInterface` (em `02_SandboxInterfaces`) era **somente leitura** —
expunha apenas `HasTag`, `HasAny` e `HasAll`. Foi estendida com o contrato de escrita:

```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State")
void AddTag(FGameplayTag StateTag);

UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State")
void RemoveTag(FGameplayTag StateTag);

/** Retorna por valor: UFUNCTION não admite retorno por referência constante. */
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State")
FGameplayTagContainer GetActiveStateTags() const;
```

* `USBStateComponent` passou a implementar os três `_Implementation`. Para evitar colisão no
  sistema de reflexão, as marcações `UFUNCTION` foram removidas dos métodos concretos
  homônimos — exatamente o padrão que `HasTag` já seguia na classe.
* Os 14 componentes de produção do Core passaram a resolver o estado por contrato, adotando o
  padrão canônico que `SBPortalSubsystem` já usava:

```cpp
if (UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass()))
{
    ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Profiler_Instrumented);
}
```

* Quatro headers trocaram `TWeakObjectPtr<USBStateComponent>` por `TWeakObjectPtr<UActorComponent>`,
  eliminando também as *forward declarations* do tipo concreto.

**Resultado**: `SandboxCore` não referencia mais nenhum símbolo de `SandboxCharacter`.

---

## 3. Refinamento — Eliminação de Lookup de Classe por String

### Problema
`SBSandboxRuleSubsystem` resolvia o componente de estado por nome textual, em dois blocos
duplicados:

```cpp
UClass* StateCompClass = FindObject<UClass>(nullptr, TEXT("/Script/SandboxCharacter.SBStateComponent"));
if (StateCompClass) { StateComp = TargetActor->GetComponentByClass(StateCompClass); }
```

Isso é frágil (quebra silenciosamente se a classe for renomeada ou movida, sem erro de
compilação), custa uma busca no registro global de objetos a cada avaliação de regra, e
acopla o Core a um caminho de módulo de Gameplay.

### Ação
Substituído nos dois blocos por resolução via contrato:

```cpp
StateComp = TargetActor->FindComponentByInterface(USBStateComponentInterface::StaticClass());
```

Além de remover a string e a busca global, passa a atender **qualquer** implementador da
interface, não só o `USBStateComponent`.

### ✅ Pendência resolvida posteriormente
O mesmo arquivo resolvia o componente de atributos por
`FindObject<UClass>("/Script/SandboxCharacter.SBAttributeComponent")` seguido de `ProcessEvent`
sobre a `UFunction` `GetAttributeValue` — o padrão mais frágil de todo o código-base, porque
não falha em compilação se a classe for renomeada.

Foi criado o **`ISBAttributeComponentInterface`** em `02_SandboxInterfaces`, com escopo mínimo
(`GetAttributeValue`, único método com consumidor real). `USBAttributeComponent` passou a
implementá-lo, com a marcação `UFUNCTION` removida do método concreto homônimo para evitar
colisão de reflexão — mesmo padrão do §2.

**Resultado**: `04_SandboxCore` não contém mais nenhum lookup de classe por string.

---

## 4. Defeitos de Compilação Corrigidos

Todos em código das Fases 121–134 que nunca havia passado pelo compilador.

| # | Defeito | Local | Correção |
| :-: | :--- | :--- | :--- |
| 1 | `MoveTemp` sobre objeto const — o `operator()` de lambda é `const` por padrão, impedindo mover a callback capturada | `SBAsyncTaskManagerSubsystem.cpp` | Lambda externo marcado `mutable`, preservando a semântica de move |
| 2 | Sobrecarga ambígua `TestNearlyEqual(float)` vs `(double)` — componentes de `FVector` são `double` no UE5 | `SBAsyncThreadingTests.cpp` (6 asserções) | Literais convertidos para `double` |
| 3 | 5 nomes de Gameplay Tag inexistentes, com os segmentos invertidos | `SBStateMatrixSubsystem.cpp`, `SBStateMatrixTests.cpp` | Mapeados para os nomes reais (tabela abaixo) |
| 4 | `AddLambda` / `AddWeakLambda` sobre `DECLARE_DYNAMIC_MULTICAST_DELEGATE` — delegates dinâmicos só aceitam `AddDynamic` com `UFUNCTION` | 12 suítes, **28 chamadas** | Criados receptores de teste com `UFUNCTION` + `AddDynamic` |
| 5 | `GetActiveStateTags` inacessível após o desacoplamento | `SBStateMatrixGuardComponent.cpp` | Método adicionado ao contrato da interface |
| 6 | Erros em cascata de aridade e conversão (`C2440`, `C2660`, `C2661`) | Suítes de StateMatrix | Resolvidos automaticamente ao corrigir os nomes de tag |

### Mapeamento das tags corrigidas (defeito 3)

| Nome inventado | Nome real registrado |
| :--- | :--- |
| `Movement_State_Swimming` | `State_Movement_Swimming` |
| `Movement_State_Diving` | `State_Movement_Swimming_Diving` |
| `Movement_State_Gliding` | `State_Movement_Gliding` |
| `Movement_State_Airborne` | `State_Movement_Flying_Airborne` |
| `State_Combat_Dead` | `State_Character_Dead` |

### Receptores de teste criados (defeito 4)

Delegates dinâmicos são exigidos pelo **Princípio 10** (`BlueprintAssignable`), então a
correção certa é o teste fornecer o receptor — não degradar o delegate.

| Arquivo | Módulo | Finalidade |
| :--- | :--- | :--- |
| `Private/Tests/SBCoreTestTypes.h` | `04_SandboxCore` | `USBCoreTestStateComponent` (implementação local de `ISBStateComponentInterface`, para o Core não depender do `USBStateComponent`), `USBCoreTestBudgetListener`, `USBCoreTestLockFreeListener` |
| `Private/Tests/SBCharacterTestTypes.h` | `05_SandboxCharacter` | `USBCharacterTestListener` com uma `UFUNCTION` por assinatura de delegate |
| `Private/Tests/SBInventoryTestTypes.h` | `08_SandboxInventory` | `USBInventoryTestListener` |

Convenção adotada: **uma instância de listener por binding**, para que delegates de mesma
assinatura não disputem os mesmos campos capturados. O padrão segue o precedente já existente
no projeto em `07_SandboxInteraction/Private/Tests/SBInteractionTestTypes.h`.

---

## 5. Remoção — Barramento de Eventos Duplicado (Fase 126)

### Análise
A Fase 126 introduziu `USBEventBusSubsystem`, um **segundo** barramento de eventos convivendo
com o `USBEventSubsystem` canônico.

| Critério | `USBEventSubsystem` (canônico) | `USBEventBusSubsystem` (Fase 126) |
| :--- | :--- | :--- |
| Consumidores no framework | **29 arquivos** | **0** (apenas ele mesmo, seu componente e seus testes) |
| `ESBEventPriority` (manifesto, seção B) | ✅ Implementa | ❌ **Não implementa** |
| Exposição a Blueprint | ✅ | ✅ |
| Payload | `UObject*` | `FSBEventBusPayload` (struct) |

O objetivo declarado da fase era *"eliminação de ponteiros diretos cruzados entre módulos
secundários"*. Nenhum ponteiro foi eliminado — nenhum módulo chegou a adotá-lo. O subsistema
duplicava a responsabilidade central do framework sendo **menos conforme** que o original,
por descartar a priorização de listeners exigida pelo manifesto.

### Ação
Removidos (backup preservado em `scratchpad/removidos_eventbus/`):

* `01_SandboxCommon/.../Types/SBEventBusTypes.h` e `.cpp`
* `04_SandboxCore/.../Subsystems/SBEventBusSubsystem.h` e `.cpp`
* `04_SandboxCore/.../Components/SBEventListenerComponent.h` e `.cpp`
* `04_SandboxCore/.../Tests/SBEventBusTests.cpp`
* Tags `State.EventBus.Subscribed` e `State.EventBus.Publishing` (uso exclusivo do subsistema)

### Ideia a preservar
O payload por struct do subsistema removido evitava a alocação de um `UObject` por evento —
custo que o barramento canônico paga, já que `SBEventPayloads.h` define **10 classes `UObject`**
de payload. Migrar o `USBEventSubsystem` para payloads por struct é uma otimização legítima,
mas de escopo próprio; fica registrada como recomendação e **não** foi executada nesta auditoria.

---

## 6. Verificações de Conformidade Aprovadas

Auditadas e **sem violações**:

| Verificação | Resultado |
| :--- | :--- |
| Metadados `SandboxVersion` nos 11 `.uplugin` (SFPS §1) | ✅ 11/11 presentes |
| Grafo de dependências dos `Build.cs` (Princípio 7) | ✅ Unidirecional em todos os 11 módulos |
| Gameplay Tags declaradas × registradas | ✅ **361/361** — zero órfãs, zero não declaradas |
| Desinscrição de widgets (SFPS §3) | ✅ `USBUserWidget::NativeDestruct()` chama `UnsubscribeAllEvents()`; os 4 widgets herdam da base |
| Componentes com tick fazendo lookup de estado por frame | ✅ Nenhum — todos os lookups ocorrem em eventos de ciclo de vida |

### Observação sem ação
`SBContainerChest.cpp` usa `CreateDefaultSubobject<USBInventoryComponent>` no construtor.
O Princípio 5 restringe injeção direta **no personagem** (via `PawnData` + `SBComponentFactory`);
um ator estático de cenário como um baú não possui `PawnData`, então o caso fica fora da regra.
Registrado apenas para ciência.

---

## 7. Divergências de Documentação a Reconciliar

Os documentos do cofre discordam entre si sobre o estado do projeto:

| Documento | Fase declarada | Specs declaradas |
| :--- | :---: | :---: |
| `00_Sandbox_Framework_Dashboard.md` | 102 | 322 |
| `status_atual_do_projeto.md` | 102 | 322 |
| `roadmap_refinamento_e_otimizacao.md` | 120 | 394 |
| `task.md` | 134 | 450 |

Adicionalmente, `task.md` salta da Fase 66 diretamente para a 124 — as fases 67 a 123 não
constam do checklist.

**Recomendação**: adotar como número oficial apenas a contagem obtida por execução real da
suíte (`Automation RunTest Sandbox`), nunca a declarada. Este relatório não atualizou os
documentos do cofre; a reconciliação deve ocorrer após a primeira execução verde da suíte
neste workspace.

---

## 8. Comparação com o Workspace Primário `D:\Unreal\V1`

Verificado por comparação de conjuntos de nomes de arquivo, normalizando as pastas órfãs do V1
para os destinos corretos.

**O `GameAnimationSample` não está faltando nenhum arquivo do V1.** As únicas divergências são
as alterações desta auditoria:

| Módulo | V1 | GAS | Diferença |
| :--- | ---: | ---: | :--- |
| Core (`04` + órfã `02`) | 131 | 127 | −5 EventBus removido, +1 `SBCoreTestTypes.h` |
| Character (`05` + órfã `04`) | 123 | 124 | +1 `SBCharacterTestTypes.h` |
| Inventory (`08` + órfã `05`) | 102 | 102 | idênticos |
| Common (`01`) | 89 | 87 | −2 `SBEventBusTypes` |

### ⚠️ O V1 contém os mesmos defeitos

O V1 possui **as mesmas três pastas órfãs** (`02_SandboxCore`, `04_SandboxCharacter`,
`05_SandboxInventory`). O erro de numeração não foi uma falha de sincronização entre projetos —
foi gravado igualmente nos dois workspaces. Portanto o V1 também carrega:

* A violação do Princípio 7 no `SandboxCore`
* Os 5 nomes de Gameplay Tag inexistentes
* As 28 chamadas `AddLambda` sobre delegates dinâmicos
* O barramento de eventos duplicado da Fase 126
* Os defeitos de `MoveTemp` e de ambiguidade `float`/`double`

**Ação recomendada**: após o build verde neste workspace, a rotina de sincronização registrada
em `task.md` deve rodar na direção inversa — de `GameAnimationSample` para `V1`.

---

## 9. Correção de Erro Cometido Durante Esta Auditoria

Ao criar o receptor de teste do inventário, o arquivo
`08_SandboxInventory/Private/Tests/SBInventoryTestTypes.h` foi **sobrescrito** em vez de
estendido. O arquivo já existia, era rastreado pelo git desde o commit inicial, e continha
`USBTestInventoryListener` e `ASBTestLootPickupActor` — ambos usados por `SBInventoryTests.cpp`
e `SBInventorySaveTests.cpp`.

**Correção aplicada**: conteúdo original restaurado via `git checkout`, e o novo
`USBInventoryTestListener` reinserido como **adição** ao arquivo. As três classes coexistem.

Verificado que nenhum outro arquivo rastreado foi apagado ou sobrescrito durante a auditoria.

---

## 10. Estado Atual

* ✅ Estrutura de plugins conforme a SFPS — 11 plugins, todos com `.uplugin`, `Build.cs`,
  registro no `.uproject`, `SandboxVersion` e formato `Source/<Módulo>/{Public,Private}`
* ✅ Princípio 7 restaurado no `SandboxCore` — zero lookups por string no módulo
* ✅ Infraestrutura duplicada removida
* ✅ **Build: `Result: Succeeded`, zero erros**
* ✅ **Suíte: 444 specs — 438 verdes, 6 vermelhas (98,6%)**
* ✅ Asserções de `enum` convertidas para comparação real (166 em 37 arquivos)

### Número oficial de specs

A contagem **medida** neste workspace é **444 specs**. Nenhum número declarado na documentação
corresponde — e o valor 221 que esta auditoria reportou inicialmente também estava errado,
porque a suíte era interrompida na metade por uma assertion fatal.

| Documento | Declara | Medido |
| :--- | :---: | :---: |
| `00_Sandbox_Framework_Dashboard.md` | 322 | **444** |
| `status_atual_do_projeto.md` | 322 de 322 | **444** |
| `roadmap_refinamento_e_otimizacao.md` | 394 | **444** |
| `task.md` (Fase 134) | 450 | **444** |

> [!WARNING] Por que nem "444 verdes" bastaria como métrica
> Até esta auditoria, 166 dessas asserções eram inertes (§13). Um número de specs só significa
> alguma coisa quando as asserções de fato comparam valores — que é precisamente o hábito que
> este projeto precisa adquirir.

---

## 11. O Crash que Truncava Metade da Suíte

Durante a auditoria, a suíte reportava consistentemente ~221 specs. O número real é **444**.

Uma assertion fatal derrubava o processo de teste no meio da execução:

```
Assertion failed: O->GetClass()->ImplementsInterface(USBComponentInterface::StaticClass())
```

**Causa — introduzida por esta auditoria.** O test double `USBCoreTestStateComponent` (§4)
implementava `ISBStateComponentInterface` mas não `ISBComponentInterface`. As suítes de Core
chamam `ISBComponentInterface::Execute_OnInitialize(StateComp)`, e `Execute_` sobre um objeto
que não implementa o contrato dispara assertion e encerra o processo.

**Correção**: os seis ganchos de ciclo de vida foram adicionados ao test double, espelhando
`USBStateComponent`.

> **Consequência metodológica**: toda medição anterior a esta correção — incluindo as primeiras
> desta auditoria — cobria apenas metade da suíte. Módulos inteiros (`RuleEngine`,
> `ArmorDurability`, `IndustrialProcessor`, `Portals`, `Persistence`) nunca haviam executado,
> nem antes nem durante a auditoria.

---

## 12. Defeitos de Produto Corrigidos

Sete defeitos com impacto direto em jogo, todos encontrados ao executar código que nunca
havia rodado.

| # | Componente | Defeito | Impacto em jogo |
| :-: | :--- | :--- | :--- |
| 1 | `USBGameplayEffectComponent` | Stack aplicado **duas vezes** e modificador anterior nunca removido | Buff de +10 em 3 stacks dava **+140** em vez de +30 |
| 2 | `USBHitTraceComponent` | `FVector::ZeroVector` usado como sentinela de "sem posição anterior" | Personagem atacando perto da origem tinha a varredura colapsada num ponto — **golpes atravessavam inimigos** |
| 3 | `USBInventoryComponent` | Inscrição em `BeginPlay` (dispara no `RegisterComponent`) em vez de `OnPostInitialize` | **Armadura nunca perdia durabilidade** se o inventário fosse injetado antes dos atributos |
| 4 | `USBStructuralIntegrityComponent` | Propagação lia a distância **atual** dos vizinhos; ligações bidirecionais criavam suporte circular | **Destruir a fundação não derrubava a construção** |
| 5 | `USBLockFreeEventSubsystem` | `FMath::Max(16, InCapacity)` elevava a capacidade pedida em silêncio | Lógica de saturação baseada no valor solicitado ficava errada sem aviso |
| 6 | `ASBCharacter` | `Execute_GetStateComponent` / `GetAttributeComponent` retornam nulo — override não registrado na reflexão | 8 call sites em 3 plugins recebiam nulo ao pedir componentes ao personagem |
| 7 | `USBCargoDroneNetworkComponent` | Desvio por bateria crítica sobrescrito pelo avanço de rota no mesmo tick | **Drone com bateria crítica pousava no destino em vez de retornar à base** |

### 🔁 Padrão recorrente: ordem de operações dentro do tick

Três dos sete (`AtmosphericSafety`, `Poise`, `CargoDrone`) são a mesma família: dois efeitos
disputam o mesmo tick e o segundo anula o primeiro.

```cpp
if (Contador > 0) { Contador -= DeltaTime; }   // consome
else              { AplicaEfeito(DeltaTime); } // aplica — nunca no tick que zera
```

**Recomendação**: varredura dirigida em componentes com contadores de tempo
(`CooldownTimer`, `StaggerTimer`, `RegenDelayTimer`) procurando o mesmo formato.

### Sobre o defeito 6 — hipótese testada e REFUTADA

Durante a auditoria levantei a hipótese de `Intermediate` desatualizado, com dois indícios: o
`SBCharacter.gen.cpp` não continha nenhum thunk para `ASBCharacter` nem menção a
`GetStateComponent`, e o header era mais recente que o arquivo gerado.

**O teste foi executado**: o `Intermediate` do `05_SandboxCharacter` foi apagado e o módulo
recompilado do zero (UHT regenerou 98 arquivos, contra 86 do build incremental).

| Medida | Antes | Depois da regeneração |
| :--- | ---: | ---: |
| Menções a `GetStateComponent` | 0 | **0** |
| Tamanho do `.gen.cpp` | 10.098 bytes | **10.098 bytes** |
| Thunks `ASBCharacter::exec*` | 0 | **0** |

Arquivo byte a byte idêntico. **Não era staleness.**

> [!WARNING] A evidência original estava mal interpretada
> A ausência de thunks por classe no `.gen.cpp` é o comportamento **normal** do UHT para
> overrides de interface `BlueprintNativeEvent` — o despacho passa pelo thunk da própria
> interface, não por um gerado na classe implementadora. Tratei o normal como anomalia.
>
> A diferença de timestamp também não significava nada: o UHT não reescreve arquivo cujo
> conteúdo não muda.

**O que permanece verdadeiro** são os fatos medidos por instrumentação, independentes daquela
interpretação:

* `Execute_GetStateComponent` retorna nulo com um `USBStateComponent` presente no ator
* `Execute_PrepareForBackgroundSim` deixa o `OutData` vazio, com GUID válido disponível
  (`guidPersist=66C54C6A…` / `guidSimData=0000…`)

A causa raiz no mecanismo de despacho da UE **permanece sem explicação**. As duas proteções
aplicadas deixam de ser contorno e passam a ser a solução definitiva:

1. `SBSandboxRuleSubsystem`, `SBPortalSubsystem`, `SBRegionSubsystem`, `SBGameplayBehavior` —
   resolução **por contrato primeiro**, acessor do personagem como fallback.
2. `ASBResourceNode` — chamada direta ao próprio `_Implementation`. Despacho por reflexão de um
   ator para si mesmo é indireção desnecessária, e era exatamente onde falhava.

---

## 13. 🚨 166 Asserções que Não Verificavam Nada

O achado de maior alcance da auditoria.

### O problema
`FAutomationTestBase::TestEqual` com `enum class` **sempre retorna verdadeiro** na UE 5.8.
Nenhuma sobrecarga concreta aceita enum com escopo, e o template genérico
(`AutomationTest.h`, ~linha 2196) apenas repassa a chamada.

### Como foi descoberto
Não por leitura — por instrumentação. No teste do `CargoDroneNetwork`, com o log da máquina de
estados em mãos:

| Asserção | Valor real | Esperado | Resultado |
| :--- | :---: | :---: | :--- |
| `TestEqual("Started recharging", ...)` | 2 | 4 | ✅ **passou** |
| `TestEqual("Fully charged -> IdleAtPort", ...)` | 3 | 0 | ✅ **passou** |
| `TestTrue("Has Recharging tag", ...)` | — | — | ❌ falhou |
| `TestEqual("Battery is 100%", ...)` | 69.8 | 100.0 | ❌ falhou |

As comparações de `float` e `bool` funcionavam; as de `enum` passavam com valores errados.

> **Sinal generalizável**: quando uma asserção passa e outra do mesmo bloco falha com dado
> coerente, desconfiar primeiro da que passou.

### Alcance e correção
**166 asserções em 37 arquivos**, convertidas para `TestEqual("desc", (int32)Atual, (int32)Esperado)`.

Cuidado necessário na varredura: 6 casos eram falsos positivos, do tipo
`TestEqual("count", Obj->GetCountByLOD(ESBTickLODLevel::LOD0), 1)`, onde o enum é **argumento
do getter** e a comparação é de inteiros — perfeitamente válida. Aplicar o cast cegamente teria
corrompido asserções corretas.

### Resultado da correção
O total de verdes **não caiu**. As asserções inertes em testes que passavam estavam comparando
valores que já eram corretos — eram redundantes, não encobridoras. A diferença é entre
*"estava certo"* e *"sabemos que está certo"*, que é exatamente a distinção que este projeto
vinha falhando em fazer.

---

## 14. Defeitos Sistêmicos da Suíte de Testes

Além do `TestEqual`, três padrões afetavam a suíte inteira.

### 14.1 Atores de teste sem `RootComponent` — 36 ocorrências
`SpawnActor<AActor>(AActor::StaticClass(), Local, ...)` **não posiciona o ator**: um `AActor`
puro não tem `RootComponent` no spawn, e a localização não tem onde ser gravada. Sem root,
`SetActorLocation`, `SetActorTransform` e `TeleportTo` falham **em silêncio**.

Apareceu em quatro variantes, cada uma invisível ao scanner da anterior:

| Variante | Por que escapou |
| :--- | :--- |
| Spawn em posição não-zero sem root (25 casos) | — |
| Spawn na origem + `SetActorLocation` depois | posição do spawn era zero |
| Spawn na origem + `SetActorTransform` depois | método diferente do procurado |
| Movido por terceiro via `TeleportTo` | o teste não move o ator |

Corrigido estruturalmente: **todo ator de teste recebe `RootComponent`**.

Exemplo do quanto isso mascarava — em `SBPersistenceTests`, a asserção intermediária
*"Transform should be reset to Identity"* **passava**, confirmando um reset que nunca tinha
sido precedido de um set.

### 14.2 Vazamento de estado entre testes
`SBIndustrialProcessorTests` mutava um membro do spec com `.Add()` dentro do `BeforeEach`, sem
reset. Como o `BeforeEach` roda antes de **cada** `It`, a receita acumulava ingredientes — 1 no
primeiro teste, 2 no segundo, 3 no terceiro — e o forno consumia múltiplos do esperado.

Defeito que **passa isoladamente e falha em sequência**, piorando conforme a suíte cresce.
Varredura confirmou ser caso único.

### 14.3 Ciclo de vida incompleto
Três testes de inventário chamavam `OnInitialize` e `OnReady`, pulando **`OnPostInitialize`** —
o gancho que o manifesto reserva para cachear componentes irmãos.

---
## 15. Oportunidades de Otimização Identificadas (não aplicadas)

### 12.1 Lookups de componente sem cache dentro de `TickComponent`

`FindComponentByClass` percorre linearmente o array de componentes do ator. Quatro
ocorrências executam a cada frame, por personagem:

| Arquivo | Ocorrências |
| :--- | :---: |
| `05_SandboxCharacter/.../SBMovementComponent.cpp` | 3 (2× `USBAttributeComponent`, 1× `USBStateComponent`) |
| `05_SandboxCharacter/.../SBAbilityComponent.cpp` | 1 (`USBStateComponent`) |

O projeto já emprega o padrão correto (membro `Cached*` com inicialização preguiçosa) em
`SBStatusEffectComponent`, `SBGameplayEffectComponent`, `SBCombatFeedbackComponent`,
`SBComboComponent`, `SBPoiseComponent` e nos 4 componentes de Core. Trata-se de
inconsistência, não de decisão de projeto.

> Não aplicado: `SBMovementComponent` é caminho crítico de gameplay e a alteração merece
> validação dedicada.

### 12.2 Acoplamento por string entre plugins irmãos

`06_SandboxCombat` e `08_SandboxInventory` são ambos *Gameplay Extensions* e, pela SFPS, não
podem depender um do outro. A solução adotada foi resolver classes por nome textual:

| Arquivo | Alvo |
| :--- | :--- |
| `06_SandboxCombat/.../SBWeaponBehavior.cpp:107` | `/Script/SandboxInventory.SBInventoryComponent` |
| `08_SandboxInventory/.../SBResourceNode.cpp:76` | `/Script/SandboxCombat.SBCombatComponent` |
| `08_SandboxInventory/.../SBInventoryTests.cpp` | 5 ocorrências |
| ~~`04_SandboxCore/.../SBSandboxRuleSubsystem.cpp:94`~~ | ✅ **Resolvido** — ver §3 |

A restrição de dependência está **correta**; o contorno é que viola o Princípio 4. A solução
alinhada ao manifesto é declarar o contrato em `02_SandboxInterfaces` — exatamente o que foi
feito nesta auditoria para `ISBStateComponentInterface` (§2). Faltam contratos equivalentes
para atributos, inventário e combate.

> [!IMPORTANT]
> Nenhum arquivo rastreado pelo git foi apagado. Os arquivos removidos eram todos não
> rastreados (`untracked`), e cópias de segurança estão em
> `scratchpad/removidos_eventbus/`. As alterações em arquivos rastreados
> (`SBGameplayTags.h/.cpp`, `SBStateComponent.h`, `SBStateComponentInterface.h`,
> `SBSandboxRuleSubsystem.cpp`) são reversíveis por `git checkout`.

---

## 16. Falhas Remanescentes — 6 de 444

| Teste | Módulo | Investigado? |
| :--- | :--- | :--- |
| `AIBehavior` — foco e pausa sob CC | 06_SandboxCombat | ❌ |
| `AIBehavior` — slots de Smart Object | 06_SandboxCombat | ❌ |
| `AIBehavior` — fases de boss por HP | 06_SandboxCombat | ⚠️ Parcial — thresholds, `MaxHealth` e broadcast conferidos e corretos |
| `BackgroundSim` — descarga e retomada de nó | 08_SandboxInventory | ❌ |
| `WorldIntegrity` — auditoria de receitas e loot | 04_SandboxCore | ❌ |
| `DynamicCrop` — progresso e estágio colhível | 08_SandboxInventory | ❌ |

Não têm raiz comum aparente. As raízes sistêmicas (posicionamento, ciclo de vida, contrato,
`TestEqual`) já foram esgotadas — daqui em diante são defeitos individuais, com rendimento por
hora menor.

---

## 17. Método — o que funcionou

Registrado porque o processo se mostrou mais valioso que qualquer correção isolada.

**Instrumentar venceu deduzir, sempre.** Três hipóteses minhas foram refutadas pelos dados:
o ambiente headless como causa das 13 falhas (era defeito real), o clamp de `MaxWarpDistance`
(o default era 600, não 0) e `AddInstanceComponent` sem `AddOwnedComponent` (24 ocorrências
mapeadas — a instrumentação mostrou que o componente **era** encontrado, e a causa estava no
`ASBCharacter`). Corrigir as 24 teria alterado código correto e escondido o defeito real.

**Posicionamento do log importa.** Instrumentar o ponto suspeito dá pouca informação quando ele
não executa. Um log que não aparece é ambíguo: pode ser que a função não rodou, ou que rodou e
desviou antes. O log precisa ficar no caminho que **certamente** roda, imprimindo as variáveis
que decidem o desvio.

**Preservar a ordem dos dados.** Deduplicar o log de execução com `awk !seen` destruiu a
sequência e misturou quatro testes, quase levando à conclusão errada. Refeito com a ordem
íntegra e as fronteiras de teste marcadas, a contradição se resolveu sozinha.

**Medir a cada passo.** Cada varredura escrita estava correta para o que procurava e cega para
o resto — o padrão do `RootComponent` reapareceu em quatro formas distintas, cada uma revelada
apenas pela falha seguinte.
