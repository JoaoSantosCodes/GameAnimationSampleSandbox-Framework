# 🏪 Roadmap de Publicação na Fab — Plugins do Sandbox Framework

> Escrito em 06/09/2026 a partir de medição do repositório, não de estimativa. Todo número deste documento foi obtido contando arquivos e linhas em `D:\Unreal\GameAnimationSample` nesta data.

A loja da Epic hoje é a **Fab** (a Unreal Marketplace foi absorvida por ela em out/2024). O que segue vale para produto do tipo *code plugin*, que é o que existe aqui.

---

## 0. Recomendação em uma frase

**Não publique onze plugins. Publique um, depois decida se vale o segundo.** O framework tem profundidade real em cinco sistemas e uma cauda longa de trinta componentes de 100–200 linhas que são esqueletos funcionais, não produtos. Vender a cauda é comprar suporte para código que ninguém terminou.

---

## 1. O que existe hoje (medido)

| Plugin | Módulos | LOC | Testes (LOC) | Depende de |
|---|---|---|---|---|
| `01_SandboxCommon` | 1 | 6.922 | 156 (2%) | GameplayAbilities, 02, ModularGameplay |
| `02_SandboxInterfaces` | 1 | 505 | — | — |
| `03_SandboxAssets` | 1 | 304 | — | 01, 02 |
| `04_SandboxCore` | 1 | 11.411 | 2.942 (25%) | 01, 02, 03, EnhancedInput, **ModularGameplayActors** |
| `05_SandboxCharacter` | 1 | 17.903 | 5.383 (30%) | 01–04, **ModularGameplayActors** |
| `06_SandboxCombat` | 1 | 9.381 | 3.591 (38%) | 01, 02, 05, StateTree, SmartObjects, **ModularGameplayActors** |
| `07_SandboxInteraction` | 1 | 1.488 | 426 (28%) | 01–05, **ModularGameplayActors** |
| `08_SandboxInventory` | 1 | 14.236 | 5.670 (39%) | 01–05, PCG, **ModularGameplayActors** |
| `09_SandboxUI` | 1 | 1.000 | 132 (13%) | 01–04 |
| `10_SandboxDebug` | 1 | 168 | — | 02 |
| `11_SandboxEditor` | 1 | 26 | — | 01–08 |
| `UnrealMCP` | 1 | 5.697 | — | EditorScriptingUtilities |

**Total: ~69.000 LOC, 416 specs verdes, 1.894 asserções, 476 funções `BlueprintCallable`, 34 classes com replicação.**

> O número de specs foi corrigido de 444 para 416 em 06/09/2026: o filtro da suíte arrastava 30 testes da engine. Ver o registro do Portão B, na Fase B.

O que já está certo e não custa nada:

- **Grafo de dependências é um DAG limpo**, sem ciclos entre plugins.
- **Zero acoplamento com o projeto**: nenhum `#include` de `GameAnimationSample` dentro dos plugins.
- **Zero caminho `/Game/` hardcoded** no C++ — o framework não presume o conteúdo do projeto.
- **Zero código específico de plataforma** (`PLATFORM_WINDOWS`, `windows.h`): portável por construção.
- **Zero `TODO`/`FIXME`/`HACK`** no código.
- **As 166 asserções inertes de enum foram corrigidas** — resta zero no padrão auditado.

Nada disso é comum em framework de vendedor iniciante. A distância até a loja não está na arquitetura; está nos cinco itens abaixo.

---

## 2. Os cinco bloqueadores medidos

> [!WARNING] Nenhum produto é submetido antes destes cinco
> Não são polimento — são causa de reprovação na revisão ou de pedido de reembolso.

### B1 — Dependência de `ModularGameplayActors`, que é da Lyra, não da engine — ✅ RESOLVIDO em 06/09/2026

Verificado contra a engine em `D:\Unreal\Unreal Sistema\UE_5.8`: `ModularGameplay`, `GameplayAbilities`, `EnhancedInput`, `PCG`, `SmartObjects`, `StateTree` e `GameplayStateTree` **vêm com a engine**. `ModularGameplayActors` **não vem** — está no repositório porque foi copiado da Lyra. Cinco plugins declaram a dependência em `.Build.cs` (04, 05, 06, 07, 08).

O comprador não tem esse plugin, e redistribuir código da Lyra dentro de um produto pago não é aposta que se faça sem confirmar a licença.

**A boa notícia é o tamanho do problema:** o uso real são **10 referências em 5 arquivos** — `SBGameMode.h`, `SBGameState.h`, `SBPlayerController.h`, `SBPlayerState.h` e `SBCharacter.h`, todas apenas herdando as classes-base. (A primeira medição dizia 4 arquivos: o grep não cobria o nome `AModularGameModeBase`.) Essas classes-base não fazem nada além de repassar chamadas ao `UGameFrameworkComponentManager`. Reimplementá-las dentro de `04_SandboxCore` é meio dia de trabalho e elimina a dependência de vez.

### B2 — 18.300 linhas de teste dentro dos módulos de runtime — ✅ RESOLVIDO em 06/09/2026

112 arquivos de teste vivem em `Private/Tests/` dos módulos de runtime, **sem nenhuma guarda de compilação** (`WITH_DEV_AUTOMATION_TESTS` aparece em zero deles). Em `06_SandboxCombat` e `08_SandboxInventory` isso é 38% e 39% do módulo.

Confirmei na `AutomationTest.h` da 5.8 que as macros têm definição nos dois lados do `#if WITH_AUTOMATION_WORKER` — ou seja, **isto compila em Shipping**, não quebra o build. O problema é outro: o comprador leva 18k linhas de teste dentro do produto dele, e revisor de loja lê isso como produto não preparado para distribuição.

**Correção aplicada:** 117 arquivos movidos para sete módulos próprios (`SandboxCoreTests`, `SandboxCharacterTests`, …), tipo **`UncookedOnly`** — não `DeveloperTool` como este documento dizia antes. `UncookedOnly` é a convenção dominante da própria engine para suíte de teste (27 módulos contra 10 na 5.8) e é a garantia mais forte: módulo `UncookedOnly` **nunca entra em build cozinhado**, então o jogo do comprador não carrega teste nenhum.

*O risco levantado aqui não se materializou:* os únicos headers privados incluídos por teste são os próprios helpers de teste (`SBCoreTestTypes.h`, `SBCharacterTestTypes.h`, `SBInventoryTestTypes.h`, `SBCombatTestHelper.h`, `SBInteractionTestTypes.h`), que se mudaram junto. Zero testes dependiam de header privado do produto.

*O que a mudança revelou de brinde:* duas classes que só existem para teste moravam no produto — `USBTestMovementComponent`, declarada **dentro do header público** `SBMovementComponent.h` e implementada no `.cpp` do produto, e `USBUITestMockWidget`, na pasta `Public/Tests/` do `09_SandboxUI`. Cada uma usada por um único teste. Foram para os módulos de teste.

### B3 — Metadados de vitrine são de rascunho — ✅ RESOLVIDO em 06/09/2026

Todo `.uplugin` diz `"CreatedBy": "Antigravity"` — e o do `UnrealMCP` diz literalmente `"Your Name"`. `DocsURL`, `SupportURL` e `CreatedByURL` estão vazios nos doze. Todos são `VersionName: 1.0.0`. O `UnrealMCP` ainda usa `WhitelistPlatforms`, campo renomeado para `PlatformAllowList` nas versões atuais.

Autoria, URL de documentação e canal de suporte são campos que a loja exige preenchidos e que o comprador usa para decidir se confia.

**Resolvido:** os doze `.uplugin` agora declaram `CreatedBy: João Santos`, `CreatedByURL`, `DocsURL` no repositório e `SupportURL` nas issues dele; o `WhitelistPlatforms` do `UnrealMCP` virou `PlatformAllowList`. `VersionName` continua `1.0.0` em todos — versionar de verdade só faz sentido quando cada produto tiver ciclo próprio.

### B4 — Nenhum arquivo tem cabeçalho de copyright — ✅ RESOLVIDO em 06/09/2026

~275 arquivos começam direto em `#pragma once`. Nenhuma linha de copyright em nenhum. É exigência de praxe da revisão e é a única defesa se o código aparecer republicado. Correção mecânica, script de um parágrafo. **Não depende da decisão de marca** (§4), ao contrário do que este documento afirmava: o detentor do copyright é a pessoa ou empresa, não o nome do produto.

**Resolvido:** `// Copyright 2026 João Santos. All Rights Reserved.` em **598 arquivos** — não os ~275 estimados aqui, porque aquele número contava só os `.h` e o cabeçalho vai também nos `.cpp` e nos `.Build.cs`. A fórmula ficou em inglês, que é a convenção da loja e da própria engine, apesar dos comentários do código estarem em português.

### B5 — Os plugins não têm conteúdo nenhum — 🟡 primeiro conteúdo entrou em 06/09/2026

Todos declaram `CanContainContent: true` e **todos têm zero `.uasset`**. Os 30 assets de demonstração vivem em `Content/SandboxFramework/` do projeto, e o `DefaultEngine.ini` aponta mapa e GameMode padrão para lá.

Plugin de código pode ser vendido sem conteúdo, mas *este* não pode: são 476 funções Blueprint e um sistema de fragmentos de item. Sem um mapa que abra e funcione, o comprador não descobre por onde começar e pede reembolso. Conteúdo de demonstração é trabalho de produto, não sobra do que já existe.

**Primeiro passo dado:** `/08_SandboxInventory/Demo/` tem seis assets — quatro definições de item com nove fragmentos entre elas, uma receita que consome 2× Sucata numa forja e devolve uma Tocha, e uma loot table de três entradas ponderadas. É o primeiro conteúdo que mora **dentro de um plugin** em vez de `/Game`. Autorado por comando, não à mão.

*Achado ao autorar:* o vocabulário de tags de item é praticamente vazio — **uma** tag `Item.*` registrada (`Item.Type.Material`) contra onze tipos de fragmento. Um produto de inventário precisa entregar um conjunto inicial de tags que faça sentido; hoje o comprador começa do zero.

**Mapa e Blueprints entraram em 07/09/2026.** `/08_SandboxInventory/Demo/L_InventoryDemo` tem chão, luz, `PlayerStart` e quatro Blueprints de exemplo ligados aos Data Assets: um nó de recurso que dropa pela loot table, uma forja que conhece a receita da tocha, um baú e um drop físico. Tudo dentro do plugin.

O MCP não cria nem salva nível, então o mapa é montado por commandlet Python — script versionado em `scripts/montar_nivel_demo.py`, e o `PythonScriptPlugin` ficou habilitado só para o alvo Editor.

*O que ainda falta no B5:* uma UI de inventário utilizável (o `SBInventoryGridWidget` é C++ sem widget `.uasset`), e um passo a passo de primeiro uso. O mapa abre; o comprador ainda não tem o que clicar.

### Fora da lista, mas a decidir: a origem do `UnrealMCP`

O plugin é derivado do projeto aberto `chongdashu/unreal-mcp`. Não há arquivo de licença nem de atribuição dentro de `Plugins/UnrealMCP/`. Antes de qualquer plano que envolva vendê-lo, é preciso ler a licença de origem e cumprir a atribuição. Lembrando o que já está registrado: **o lado Python não é versionado** — um produto MCP teria que incluí-lo, e hoje ele existe só na sua máquina.

---

## 3. Recorte de produto recomendado

> [!INFO] O recorte detalhado está em [[roadmap_produtos_fab]]
> Esta seção dá a forma geral. O catálogo mede os quinze pacotes um a um, com o estado de cada um e o que falta — e corrige o julgamento que fiz aqui sobre os sistemas industriais.

Onze produtos significam onze páginas, onze conjuntos de documentação, onze filas de suporte e onze revisões a cada versão nova da engine. E como 04–08 arrastam 01, 02 e 03 juntos, vender separado obriga a embutir a fundação em cada um ou a exigir que o comprador compre outro produto antes.

**Três produtos, nesta ordem, com um portão de decisão entre eles:**

### P0 — Piloto de processo (o objetivo não é receita)

Um produto pequeno, publicado **de graça**, para atravessar o funil da Fab uma vez: conta de vendedor, cadastro fiscal, empacotamento, revisão, correção do que a revisão apontar, atualização de versão. Aprender isso num produto de 5k linhas custa uma semana; aprender no produto principal custa a reputação da primeira página.

Candidato natural é o `UnrealMCP` — editor-only, uma dependência só, categoria em alta — **desde que a licença de origem permita e a atribuição seja feita**. Se não permitir, o piloto vira `10_SandboxDebug` + `09_SandboxUI` como pacote de utilidades, ou pula-se direto para P1 sabendo que o primeiro contato com a revisão será no produto que importa.

### P1 — O produto de verdade: sobrevivência (inventário, itens, crafting, interação)

É onde está a profundidade medida: `SBInventoryComponent` 1.168 linhas, `SBAttributeComponent` 659, `SBCraftingComponent` 462, mais os onze `SBItemFragment_*` — arquitetura de fragmentos no estilo da Lyra, que é exatamente o que quem procura inventário quer comprar. É também a parte mais testada do repositório (39% e 28%).

Escopo: `08_SandboxInventory` + `07_SandboxInteraction` + a fatia de `05_SandboxCharacter` que atributos e efeitos exigem, com `01`–`04` embutidos como fundação do produto.

### P2 — Personagem e movimento (só se P1 vender)

`05_SandboxCharacter` tem `SBMovementComponent` (580), `SBAbilityComponent` (545), câmeras, anim layers e comportamentos de movimento. Mas tem também a cauda: parkour 120 linhas, glider 143, zipline 143, mount 133, footIK 101. Antes de virar produto, cada um desses precisa ser **promovido, congelado ou apagado** (§5).

### O que provavelmente nunca deve ser produto

Os sistemas industriais e exóticos — power grid, esteiras, tubulações, ferrovia, elevador espacial, mecha, espaçonave, cirurgia, domesticação, radiação. São 150–250 linhas cada. Impressionam numa lista de recursos e decepcionam na primeira hora de uso. Ou dois deles crescem até virar um "Factory Kit" de verdade, ou ficam onde estão: infraestrutura interna do BLACK VEIL.

---

## 4. A decisão que fica mais cara a cada semana: o nome

"Sandbox Framework", prefixo `SB`, módulos `SandboxCore`/`SandboxCharacter`. É genérico ao ponto de não ser buscável na loja, não ser defensável como marca e colidir com dezenas de projetos.

Renomear hoje é um `sed` em 275 arquivos e uma recompilação. Renomear depois de vender é impossível: o nome do módulo entra nos `.uasset` do comprador e mudá-lo quebra todo projeto que usou o produto. **Esta decisão vem antes do cabeçalho de copyright (B4) e antes de qualquer submissão.**

---

## 5. O que este roadmap resolve no planejamento do dia a dia

Foi por isso que você pediu o roadmap, e é o efeito mais útil dele: **publicar impõe uma definição de "pronto" que o desenvolvimento interno não impõe.**

Hoje um sistema é dado como pronto quando os specs ficam verdes. Um sistema publicável só está pronto quando: compila em projeto vazio, tem mapa de demonstração, tem página de documentação, sobrevive a um estranho usando errado, e você aceita mantê-lo na próxima versão da engine.

Aplicada à cauda longa dos trinta componentes de 100–200 linhas, essa régua força a classificação que o repositório vem adiando:

- **Promover** — o BLACK VEIL precisa, então terminar de verdade e testar;
- **Congelar** — não é do jogo e não é produto: para de receber trabalho, fica documentado como experimental;
- **Apagar** — nem jogo nem produto: sai do build e para de custar compilação, revisão e falsa sensação de cobertura.

Sem essa régua, todo componente novo entra no framework e nunca sai.

---

## 6. Fases e portões

Cada fase tem um portão verificável. Sem o portão fechado, a fase seguinte não começa.

### Fase A — Decisões e conta *(não escreve código)*

1. Marca e prefixo definidos (§4).
2. Recorte de produto confirmado (§3).
3. Conta de vendedor na Fab criada; cadastro fiscal e de repasse iniciado — **este item tem espera externa de semanas e não depende de você depois de enviado; comece por ele.**

**Portão A:** nome escolhido, conta criada, formulários enviados.

### Fase B — Higiene de distribuição *(aplica-se a todos os produtos)*

1. ~~B1: reimplementar as classes-base e remover `ModularGameplayActors` dos cinco `.Build.cs`.~~ ✅ 06/09/2026 — `SBModularActors.h/.cpp` em `04_SandboxCore`; build verde e suíte verde depois da troca (446 no log bruto, 416 do framework).
2. ~~B2: extrair os testes para módulos próprios por plugin.~~ ✅ 06/09/2026 — 117 arquivos em sete módulos `UncookedOnly`.
3. ~~B3: preencher os doze `.uplugin`; trocar `WhitelistPlatforms` por `PlatformAllowList`.~~ ✅ 06/09/2026
4. ~~B4: cabeçalho de copyright.~~ ✅ 06/09/2026 — 598 arquivos.
5. Definir a matriz de versões da engine a suportar — cada versão a mais multiplica build, teste e revisão; comece por **uma**.

**Portão B:** ~~um projeto novo e vazio recebe os plugins candidatos, compila em Development **e** Shipping, e a suíte roda verde, sem nada do resto deste repositório.~~ ✅ **fechado em 06/09/2026** — o registro completo está logo abaixo.

### Portão B — executado em 06/09/2026 ✅

Montei um projeto C++ vazio (`D:\Unreal\PortaoB`): um módulo de jogo de quatro linhas, dois `Target.cs`, um `Config` de duas linhas, os onze plugins copiados **sem `Intermediate` nem `Binaries`**, e os sete plugins de engine que os próprios `.uplugin` declaram. Nada mais — nenhum asset, nenhum `DefaultGameplayTags.ini`, nenhum `DefaultEngine.ini` do projeto original.

| Prova | Resultado |
|---|---|
| Editor Win64 Development | ✅ `exit code 0` — 382 ações, compilado do zero (o UBT criou o makefile sem cache algum) |
| Jogo Win64 **Shipping** | ✅ `exit code 0` — `PortaoB-Win64-Shipping.exe`, 163 MB, monolítico |
| Suíte dentro do projeto vazio | ✅ `TEST COMPLETE. EXIT CODE: 0` — **437 sucessos, 0 falhas** |
| Acoplamento com o projeto original | ✅ nenhuma referência em código; os únicos casamentos com "GameAnimationSample" eram as URLs que acabaram de ser gravadas nos `.uplugin` |

**O framework é autocontido.** Deixa de ser hipótese e passa a ser fato medido.

**O Shipping provou o B2 por execução:** 202 ações contra as 382 do Editor, e **zero** arquivo `*Tests.cpp` ou módulo `*Tests` no build. A regra está no próprio UBT (`ModuleDescriptor.cs:792`): `UncookedOnly` só compila quando o alvo não exige dado cozinhado, enquanto `DeveloperTool` responde a `bBuildDeveloperTools` e entraria num Development de jogo.

#### O que a diferença de contagem revelou — e é a descoberta que mais importa

O projeto principal fecha 446 specs; o projeto vazio fechou 437. Os nove que faltavam **não são do framework**:

```
AI.ToolsetRegistry.Sandbox.Library.*        (8)
AI.Toolsets.EditorToolset.*                 (1)
```

São testes dos plugins de engine `EditorToolset` e `ModelContextProtocol`, habilitados no `GameAnimationSample` e não no projeto vazio. Entram na contagem porque o comando `Automation RunTest Sandbox` casa por **substring**, e a palavra "Sandbox" aparece no caminho deles.

Puxando o fio, há mais 21 no mesmo caso — e esses rodam nos dois projetos:

```
Plugins.FileSandboxCore.*                   (21)
```

`FileSandboxCore` é plugin **da engine**, em `Engine/Plugins/Developer/Sandbox/FileSandbox`. Nada a ver com este framework.

> [!IMPORTANT] A suíte do framework tem **416 specs**, não 444 nem 446
> Os números que circularam na documentação vinham inflados por **30 testes da engine** que o filtro captura só porque contêm a palavra "Sandbox". A contagem honesta é a dos testes sob o prefixo `Sandbox.`: **416, todos verdes** — Character (108), Inventory (101), Combat (57), Core (53) e a cauda.
>
> Ao medir, contar `Result={Success}` **filtrando por `Path={Sandbox.`** — nunca o total do log, e nunca o `EXIT CODE`, que é 0 até com suíte vazia.

#### Nota de instalação que saiu do primeiro erro

A primeira tentativa abortou com `The following action paths are longer than 260 characters`, antes de qualquer problema de código — o projeto estava num caminho fundo demais. Vale como nota na página do produto: **instalar em caminho curto**. Quem puser o projeto em `C:\Users\...\Documents\Unreal Projects\...` bate no mesmo limite do Windows, e a mensagem não deixa óbvio que a causa é o caminho.

#### O que este portão **não** provou

Compila e os testes passam — não que alguém consiga *usar*. Não há mapa, nem Blueprint de exemplo, nem um asset sequer: o projeto inteiro copiado tem **3,5 MB e zero `.uasset`**. É o B5 intacto, e é o próximo gargalo real.

---

### Fase C — Piloto P0 ponta a ponta

Empacotar, submeter, corrigir o que a revisão apontar, publicar e **anotar cada exigência que apareceu**. Essa lista vira a checklist real da Fase D — mais confiável que qualquer documento escrito de fora.

**Portão C:** produto no ar e uma checklist de revisão escrita a partir da experiência.

### Fase D — P1: conteúdo e documentação

1. Mapa de demonstração dentro do `Content/` do plugin, com os assets migrados de `Content/SandboxFramework/`.
2. Blueprints de exemplo por sistema; itens e receitas de amostra — o MCP agora cria Data Asset por comando, o que torna isso produção e não trabalho manual.
3. Documentação de instalação e de primeiro uso; página de suporte.
4. Vídeo e imagens da vitrine.

**Portão D:** alguém que nunca viu o framework instala o produto num projeto vazio e tem inventário e crafting funcionando **em dez minutos, sem te perguntar nada**. Se precisar perguntar, é a documentação que está incompleta.

### Fase E — Submissão do P1

**Portão E:** aprovado e publicado.

### Fase F — Manutenção *(permanente, e é o custo que ninguém calcula)*

Cada versão da engine é um ciclo de recompilação, reteste e ressubmissão, por produto e por versão suportada. Some fila de suporte e correções. **Antes da Fase E, decida quantas horas por mês você aceita gastar nisso** — é o número que determina se P2 existe.

---

## 7. Riscos honestos

- **Tempo tirado do BLACK VEIL.** Publicar plugin não é subproduto de fazer o jogo; é um segundo produto com prazos próprios. O caminho que serve aos dois é só produtizar o que o jogo já obrigou a endurecer — nunca abrir frente nova para a loja.
- **Suporte de 69k linhas.** Todo recurso listado na página vira pergunta de comprador. É outro argumento para vender pouco e fundo.
- **Cauda longa exposta.** Se a página listar trinta sistemas e cinco forem esqueletos de 150 linhas, a primeira avaliação ruim é sobre isso — e avaliação inicial não se apaga.
- **Regras da loja mudam.** As exigências estruturais citadas aqui (autocontido, compila do código-fonte, autoria e suporte preenchidos, sem IP de terceiro) são estáveis; detalhes de processo, taxas e prazos precisam ser lidos nas diretrizes vigentes da Fab no dia da submissão, não neste documento.

---

## 8. O que fazer primeiro

Três coisas, nesta ordem, e nenhuma delas depende de escrever recurso novo:

1. **Abrir a conta de vendedor e enviar o cadastro fiscal** — é a única tarefa com espera externa.
2. **Decidir o nome** — fica mais caro a cada arquivo criado.
3. **Fechar o Portão B com o `08_SandboxInventory`** — remover a dependência da Lyra, extrair os testes e provar em projeto vazio. Isso responde, com fato e não com estimativa, se existe produto aqui.
