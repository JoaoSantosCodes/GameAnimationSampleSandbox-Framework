# 📦 Plano — Transformar o Sandbox Framework em Plugin Distribuível

**Levantado em**: 06/09/2026, inspecionando os 11 `.uplugin`, os `Build.cs` e a árvore de
código.
**Alvo**: um plugin publicável na Fab (ex-Marketplace) como *Code Plugin*.

> [!NOTE] O que este documento não pode garantir
> As regras exatas de submissão da Fab mudam e são da Epic. Aqui está o que **este código**
> precisa para chegar lá em condições — o que é verificável e é onde eu posso ajudar. A política
> de submissão você confirma no portal da Epic na hora de publicar.

---

## 1. Ponto de partida: melhor do que eu esperava

Três coisas que costumam matar plugins na revisão e que **já estão certas aqui**:

| ✅ | Verificado |
| :--- | :--- |
| **Zero acoplamento com Lyra** | A única dependência externa é `ModularGameplay`, plugin da própria Epic. Nenhum `CommonGame`, `CommonUI`, `GameFeatures`, `LyraGame` ou `UIExtension` nos `Build.cs`. O framework é portátil |
| **Módulo de editor separado** | `SandboxEditor` é `"Type": "Editor"` com `LoadingPhase: PostEngineInit` — não vai para o build de jogo |
| **PCH explícito** | Todos os `Build.cs` usam `PCHUsageMode.UseExplicitOrSharedPCHs` |
| **Versionamento consistente** | Os 11 `.uplugin` declaram `VersionName 1.0.0` |
| **446 specs verdes** | Raríssimo em plugin de marketplace, e é o maior diferencial comercial que este projeto tem |

Isso significa que o trabalho é de **empacotamento e higiene**, não de reescrita.

---

## 2. Bloqueadores reais, em ordem de gravidade

### 🔴 B1 — 112 arquivos de teste compilam dentro de módulos Runtime

**Evidência**: 112 arquivos em `*/Private/Tests/*.cpp`, e **zero** deles usa
`#if WITH_DEV_AUTOMATION_TESTS`.

**Consequência**: o código de teste entra no build de jogo do cliente. Puxa
`Misc/AutomationTest.h`, engorda o binário e expõe classes de teste na reflexão. É o tipo de
coisa que revisor da Epic aponta, e com razão.

**Correção**: envolver cada arquivo de teste em

```cpp
#if WITH_DEV_AUTOMATION_TESTS
// ... conteúdo atual ...
#endif // WITH_DEV_AUTOMATION_TESTS
```

Mecânico, 112 arquivos, script resolve. **Verificação obrigatória**: rodar a suíte depois — a
macro é definida em builds de Editor e Development, então os 446 specs devem continuar verdes.
Se caírem, algum arquivo ficou de fora do guard ou o guard cortou algo que não era teste.

> Alternativa mais limpa a longo prazo: mover os testes para um módulo próprio
> `SandboxTests` do tipo `DeveloperTool`. Mais trabalho, e some com o problema pela raiz.

### 🔴 B2 — 11 plugins separados, com prefixo numérico no nome

**Evidência**: `01_SandboxCommon` … `11_SandboxEditor`. O nome do plugin é o que o usuário vê no
gerenciador, e é a chave de dependência entre eles.

**Dois problemas distintos**:

1. **Distribuição**: um produto da Fab entrega um plugin. Onze plugins interdependentes viram
   onze instalações manuais na ordem certa, e qualquer uma faltando quebra o build do cliente.
2. **Nome**: `01_SandboxCommon` é um nome de pasta de trabalho, não de produto.

**Correção proposta**: **consolidar em um plugin com onze módulos.**

```
Plugins/SandboxFramework/
├── SandboxFramework.uplugin        ← um só descritor, onze módulos
└── Source/
    ├── SandboxCommon/
    ├── SandboxInterfaces/
    ├── SandboxAssets/
    ├── SandboxCore/
    ├── SandboxCharacter/
    ├── SandboxCombat/
    ├── SandboxInteraction/
    ├── SandboxInventory/
    ├── SandboxUI/
    ├── SandboxDebug/
    └── SandboxEditor/
```

> [!IMPORTANT] A disciplina arquitetural não se perde nisso
> Os Princípios da SFPS — zero dependências circulares, camadas, plugins irmãos que não se
> referenciam — são impostos por **dependência de módulo no `Build.cs`**, que o UBT valida
> independentemente de fronteira de plugin. Consolidar não afrouxa nada: as mesmas onze
> fronteiras continuam existindo e continuam sendo verificadas pelo compilador.
>
> A ordenação numérica pode ser preservada na documentação e no `LoadingPhase`, que é onde ela
> tem efeito real.

**Custo**: mover pastas, reescrever os 11 `Build.cs` (só o bloco de dependências de plugin some;
as de módulo ficam idênticas), reescrever o `.uproject`, um `.uplugin` novo. Um dia de trabalho,
com a suíte como rede de segurança.

### 🟠 B3 — O plugin não entrega conteúdo nenhum

**Evidência**: os 11 `.uplugin` declaram `"CanContainContent": true`, e **nenhum plugin tem
pasta `Content/`**. Os 28 assets do framework vivem em `Content/SandboxFramework/` do
**projeto**.

**Consequência**: quem instalar o plugin recebe C++ e mais nada. Sem PawnData de exemplo, sem
ComponentSet, sem config de movimento — e como a composição de personagem passa inteiramente
por esses assets, o cliente não consegue nem montar um personagem.

**Correção**: mover os assets de exemplo para `Plugins/SandboxFramework/Content/`, com caminhos
`/SandboxFramework/…`. Cuidado: referências entre assets são por caminho, então mover exige
*fix up redirectors* no editor.

> Isso se cruza com [[guia_implementacao_projeto]]: sete tipos de Data Asset têm zero
> instâncias. Um plugin comercial precisa de exemplo funcional de **cada** sistema que anuncia —
> hoje crafting, loot e quests não têm nem um asset para mostrar que funcionam.

### 🟠 B4 — `LogTemp` em 9 arquivos de produção

**Evidência**: 9 arquivos fora de `Tests/` logam em `LogTemp`.

**Consequência**: o log do cliente fica poluído com mensagens sem categoria identificável, e não
dá para filtrar. O projeto já tem `LogSandboxCore`, `LogSandboxCharacter`, `LogSandboxUI` — é só
usar.

**Correção**: trocar por categoria própria. Mecânico e de baixo risco.

### 🟡 B5 — Metadados vazios nos `.uplugin`

`CreatedByURL`, `DocsURL`, `MarketplaceURL`, `SupportURL` estão em branco nos 11, e não há
`EngineVersion` nem declaração de plataformas suportadas.

**Correção**: preencher no `.uplugin` consolidado. `DocsURL` e `SupportURL` são exigidos na
prática — o revisor precisa saber para onde mandar o cliente.

### 🟡 B6 — Campo `SandboxVersion` não é padrão

Os `.uplugin` trazem um bloco `"SandboxVersion": { "Plugin", "API", "Assets", "Network",
"Serialization" }` que a Unreal não conhece. É ignorado, mas pode gerar aviso na validação.

**Decisão sua**: manter (é versionamento interno útil) ou mover para um `.h` de constantes.

---

## 3. Sequência recomendada

Cada etapa termina com **build verde + 446 specs**. Nenhuma avança sem isso.

| # | Etapa | Risco | Por que nesta ordem |
| :-: | :--- | :--- | :--- |
| 1 | **B1** — guardar os testes | Baixo | Independente de tudo. Se algo quebrar, o culpado é óbvio |
| 2 | **B4** — categorias de log | Baixo | Mecânico, não interage com o resto |
| 3 | **B2** — consolidar em um plugin | **Alto** | Move tudo. Fazer depois que 1 e 2 estabilizaram, com a suíte como rede |
| 4 | **B3** — conteúdo para dentro do plugin | Médio | Depende de B2: os caminhos finais só existem depois da consolidação |
| 5 | **B5/B6** — metadados | Nenhum | Último, quando o `.uplugin` final já existe |
| 6 | Exemplos de cada sistema | Médio | O que falta para ser produto, não só código |

> **B2 é o único de risco alto.** Move onze plugins de lugar e reescreve o `.uproject`. Se der
> errado, o editor não abre. Antes de começar: commit limpo, e um branch — é o caso em que a
> branch se paga.

---

## 4. O que já é diferencial competitivo

Vale saber o que **não** precisa mudar, e o que usar como argumento de venda:

- **446 specs automatizados e verdes.** A maioria dos plugins de marketplace não tem teste
  nenhum. Isso é verificável pelo comprador e sustenta a alegação de estabilidade.
- **11 módulos com dependências unidirecionais**, verificadas pelo compilador.
- **Replicação de rede** implementada e testada — inventário à prova de race condition,
  predição de movimento, rollback, compensação de lag, anti-cheat.
- **Composição orientada a dados** (PawnData + ComponentSet), o mesmo padrão do Lyra, **sem
  depender do Lyra**.

> [!WARNING] Um cuidado com a alegação de qualidade
> A auditoria de 05/09/2026 encontrou 166 asserções que não verificavam nada e 14 fases marcadas
> como homologadas cujo código nunca compilara. Isso foi corrigido e **medido** — mas o hábito
> de anunciar mais do que se verificou é justamente o que destrói reputação de plugin pago.
> Anuncie o número que a suíte imprime, com o comando para o comprador reproduzir.

---

## 5. O que falta e não é código

Para virar produto, além dos bloqueadores:

| Item | Situação |
| :--- | :--- |
| Documentação de uso para terceiros | Existe muita doc **interna**, em português, com histórico de auditoria. Comprador precisa de guia de integração, em inglês |
| Mapa/cena de demonstração | Não existe |
| Exemplo funcional de cada sistema anunciado | Parcial — sete tipos de asset com zero instâncias |
| Suporte a plataformas declarado | Não declarado |
| Licença e atribuição de assets de terceiros | A verificar — o conteúdo atual usa malhas do Game Animation Sample, que **não podem** ser redistribuídas |

> [!DANGER] O item de licença é o mais perigoso da lista
> `DA_HeroPawnData` referencia `SKM_UEFN_Mannequin` e `SandboxCharacter_Mover_ABP`, ambos
> conteúdo do Game Animation Sample da Epic. Empacotar isso num plugin pago é redistribuição de
> conteúdo de terceiro. Os assets de exemplo do plugin precisam usar malha própria ou o
> mannequin padrão que a Epic permite redistribuir — confirme a licença antes de empacotar.

---

## 6. Estimativa honesta

| Etapa | Esforço |
| :--- | :--- |
| B1 + B4 (higiene) | 2–4 horas, com suíte entre elas |
| B2 (consolidação) | 1 dia |
| B3 (conteúdo para dentro) | 1 dia, mais o que decidir sobre licença |
| B5 + B6 (metadados) | 1 hora |
| Exemplos de cada sistema | Semanas — é criação de conteúdo, não engenharia |
| Documentação em inglês para terceiros | Semanas |

**O código está muito mais perto de publicável do que o conteúdo.** As quatro primeiras etapas
somam poucos dias e resolvem tudo que é técnico. O que separa de um produto vendável é exemplo,
documentação e licenciamento de asset — e nada disso é C++.

Se quiser começar, **B1 é o passo certo**: baixo risco, independente, e remove o problema que um
revisor apontaria primeiro. Posso executar quando você disser.
