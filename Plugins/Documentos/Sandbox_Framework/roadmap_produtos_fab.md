# 📦 Catálogo de Produtos para a Fab — o framework recortado em pacotes

> Companheiro de [[roadmap_publicacao_fab|Roadmap de Publicação na Fab]], que trata do processo. Este trata do **produto**: o que exatamente vira pacote, em que estado está e o que falta em cada um.
>
> Medido em 06/09/2026 sobre `D:\Unreal\GameAnimationSample`, agrupando os arquivos por sistema em vez de por plugin — porque um produto não é um plugin: `Atributos` mora em seis plugins, `Movimentação` em dois.

---

## 1. O eixo que decide tudo: lógica × mundo

Medi, componente a componente, quantas vezes cada um toca o mundo do jogo — trace, spawn, mover ator, dirigir o `CharacterMovementComponent`. Nos 46 componentes dos plugins 05 e 08:

| Componente | Linhas | Toques no mundo |
|---|---|---|
| `SBMovementComponent` | 580 | 27 |
| `SBSpacecraftComponent` | 216 | 8 |
| `SBBuildingComponent` | 174 | 5 |
| `SBZiplineComponent` | 143 | 4 |
| `SBGrappleComponent` | 156 | 3 |
| `SBMechComponent` | 242 | 1 |
| **os outros 40** | 100–1.168 | **0** |

`SBInventoryComponent` (1.168 linhas), `SBAttributeComponent` (659), `SBCraftingComponent` (462), `SBPowerGridComponent` (304): zero toques no mundo — **e está certo assim**. São sistemas de estado e simulação; a apresentação é do jogo. Vendem-se como framework.

`SBParkourComponent` (120 linhas): zero toques no mundo — **e isso é fatal**. O método `DetectObstacle()` *recebe por parâmetro* a posição da parede, a normal, a altura e a profundidade, e devolve um enum dizendo "Vault" ou "Mantle". Não traça, não move, não anima. Quem compra "Parkour System" está comprando exatamente o trace e o movimento que não existem aqui.

**A régua deste catálogo é essa:** para cada pacote, zero interação com o mundo é *arquitetura* ou é *o produto faltando*? A resposta muda o veredito completamente.

---

## 2. Tabela-resumo dos quinze pacotes

`impl` = linhas de implementação · `test` = linhas de teste · `specs` = casos de teste · `BP` = funções expostas a Blueprint · `rede` = arquivos com replicação · `mundo` = toques no mundo

| # | Pacote | impl | test | specs | BP | rede | mundo | Veredito |
|---|---|---|---|---|---|---|---|---|
| 1 | **Inventário, Itens & Crafting** | 3.938 | 2.885 | 57 | 24 | 12 | 0 ✅ | 🟢 produto |
| 2 | **Atributos, Efeitos & Habilidades** | 3.925 | 1.379 | 28 | 33 | 13 | 0 ✅ | 🟢 produto |
| 3 | **Performance & Debug Toolkit** | 5.179 | 1.947 | 54 | 72 | 0 | 0 ✅ | 🟢 produto |
| 4 | **Vitais de Sobrevivência** | 3.565 | 1.246 | 36 | 54 | 0 ⚠️ | 0 ✅ | 🟡 quase |
| 5 | **Redes Industriais / Fábrica** | 4.957 | 1.692 | 40 | 83 | 0 ⚠️ | 0 ⚠️ | 🟡 falta a camada de atores |
| 6 | **Combate** | 6.506 | 2.693 | 59 | 58 | 3 ⚠️ | — | 🟡 maior e mais exposto |
| 7 | **Construção** | 742 | 458 | 16 | 15 | 3 | 5 | 🟡 pequeno demais sozinho |
| 8 | **Save & Persistência** | 1.050 | 502 | 8 ⚠️ | 14 | 0 | 0 ✅ | 🟡 pouco testado |
| 9 | **Mundo & Clima** | 1.501 | 675 | 15 | 15 | 0 | — | 🟡 fragmentado |
| 10 | **Interação** | 906 | 667 | 6 | 2 ⚠️ | 0 | — | 🟡 vira anexo do nº 1 |
| 11 | **Câmera & Anim Layers** | 939 | 279 | 4 ⚠️ | 1 ⚠️ | 0 | 0 ⚠️ | 🔴 sem conteúdo, não existe |
| 12 | **UI / HUD** | 524 | 161 | 2 ⚠️ | 5 | 0 | — | 🔴 zero `.uasset` de widget |
| 13 | **Movimentação** | 3.295 | 1.667 | 40 | 37 | 0 ⚠️ | 27+0 | 🔴 sprint/crouch reais, o resto não |
| 14 | **Veículos & Mechas** | 2.093 | 736 | 20 | 37 | 0 | 9 ⚠️ | 🔴 simulação sem veículo |
| 15 | **Fundação (core)** | 3.909 | 1.820 | 36 | 16 | 0 | — | ⚫ não é produto: vai dentro de todos |

---

## 3. Fichas dos pacotes

### 🟢 1. Inventário, Itens & Crafting — *o primeiro produto*

**Entra:** `SBInventoryComponent` (1.168), `SBCraftingComponent` (462), os onze `SBItemFragment_*`, `SBItemDefinition`, `SBItemInstance`, `SBLootTableDataAsset`, `SBContainerChest`, `SBPhysicalLootDrop`, `SBResourceNode`, `SBMerchantComponent`, spawner de loot por PCG. Anexar o pacote **Interação** (nº 10) inteiro.

**Por que é o primeiro:** é o mais fundo e o mais testado do repositório — 57 specs, 2.885 linhas de teste, incluindo disputa de loot simultâneo, peso, auto-equipar, durabilidade de armadura e save. É replicado de verdade (12 arquivos com `DOREPLIFETIME`). E a arquitetura de *fragmentos de item* é o que o comprador de inventário procura.

**Zero toques no mundo aqui é arquitetura, não buraco.**

**O que falta:**
- Mapa de demonstração com baú, drop físico, nó de recurso e bancada — hoje há zero `.uasset` no plugin.
- Uma UI de inventário utilizável. `SBInventoryGridWidget` existe em C++ com 2 funções Blueprint e nenhum widget `.uasset`. Sem grade arrastável, o comprador acha que faltou metade.
- Desamarrar de `05_SandboxCharacter` (o inventário conversa com atributos) ou embutir a fatia necessária.
- Fundação `01`–`04` embutida, sem `ModularGameplayActors`.

**Esforço:** M (semanas) — a maior parte é conteúdo e documentação, não C++.

---

### 🟢 2. Atributos, Efeitos & Habilidades — *sem GAS, e isso é o argumento de venda*

**Entra:** `SBAttributeComponent` (659), `SBAbilityComponent` (545), `SBStatusEffectComponent` (393), `SBGameplayEffectComponent` (279), `SBAfflictionComponent` (224), `SBQuestComponent` (343), `SBExperienceComponent` (127), `SBStatusEffectDefinition`.

**O achado:** o framework tem **zero referências a `UAbilitySystemComponent`** — verificado no repositório inteiro. Isto não estende o GAS: é uma alternativa a ele, com replicação própria (`NetDeltaSerialize`, 13 arquivos replicados) e sem a cerimônia de ASC/AttributeSet.

Isso é uma posição de mercado real — muita gente quer atributos replicados sem GAS — **desde que dito na primeira linha da página**. Escondido, vira a reclamação nº 1: "não integra com meus outros plugins".

*De quebra:* `01_SandboxCommon` declara dependência de `GameplayAbilities` e não usa nada dela. Uma dependência a menos, de graça.

**O que falta:** demo mostrando dano, regeneração, buff temporário e efeito de status; documentação da comparação honesta com o GAS; decidir se `Quest` e `Experience` entram (são finos: 343 e 127 linhas) ou saem.

**Esforço:** M.

---

### 🟢 3. Performance & Debug Toolkit — *o candidato a piloto*

**Entra:** `SBDynamicTickThrottling`, `SBSpatialPartitionSubsystem`, `SBCacheOptimizedBuffer`, `SBLockFreeEventSubsystem`, `SBPerformanceProfilerSubsystem`, `SBCosmeticSaturationSubsystem`, `SBFaultToleranceSubsystem`, `SBWorldIntegrityAuditor`, `SBStressTestSubsystem`, `SBVisualDebuggerSubsystem`, `SBStateMatrix`, `SBLiveConfig`, categoria de GameplayDebugger.

**Por que este pode furar a fila:** 5.179 linhas, 54 specs, **72 funções Blueprint** — e é o único pacote que **não precisa de arte, animação nem personagem para demonstrar**. Um mapa com mil atores, um gráfico de tick e um botão de stress test já é a demo completa. Todos os outros produtos esbarram no problema de conteúdo (§B5 do outro documento); este não.

Também é o mais fácil de recortar: depende de `01` e `04`, não de `05`.

**O que falta:** confirmar que os subsistemas saem de `04_SandboxCore` sem arrastar GameMode e Input; mapa de demonstração; um número honesto de "quanto isso economiza" medido, porque produto de performance é comprado por número.

**Esforço:** P/M — o menor caminho até uma página publicada com algo de verdade dentro.

---

### 🟡 4. Vitais de Sobrevivência

**Entra:** `SBMetaBolicNutritionComponent` (251), `SBThermalRegulationComponent` (194), `SBRadiationExposureComponent` (187), `SBImmuneSystemComponent` (235), `SBTraumaInjuryComponent` (352), `SBSurgeryProstheticsComponent` (193), `SBAtmosphericSafetyComponent` (188), `SBDomesticationComponent` (195), `SBDynamicCropComponent` (216).

Fome, sede, temperatura, radiação, infecção, trauma: **lógica pura é a forma certa** para isso. 36 specs, 54 funções Blueprint. Como add-on do nº 2 (atributos), fecha um "Survival Systems" coerente.

**O que falta:** nenhum é replicado — para survival multiplayer isso é lacuna, e o comprador vai perguntar. Ou replica, ou a página diz "single-player / autoridade do servidor por conta do integrador". Cirurgia/próteses e domesticação são específicos demais: candidatos a ficar de fora do pacote.

**Esforço:** M.

---

### 🟡 5. Redes Industriais / Fábrica

**Entra:** `SBPowerGridComponent` (304), `SBConveyorNetworkComponent` (303), `SBPipeNetworkComponent` (307), `SBRailNetworkComponent` (297), `SBIndustrialProcessorComponent` (282), `SBMachineryComponent` (281), `SBResourceExtractorComponent` (264), `SBLogicCircuitComponent` (262), `SBCargoDroneNetworkComponent` (276), `SBSpaceElevatorComponent` (269), `SBStructuralIntegrityComponent` (297).

> [!NOTE] Correção ao documento anterior
> Em [[roadmap_publicacao_fab]] eu tratei estes sistemas como cauda descartável olhando componente a componente (150–300 linhas cada). Medidos **como conjunto**, são 4.957 linhas, 40 specs e 83 funções Blueprint — o maior número de superfície Blueprint de todos os pacotes. A avaliação anterior estava apertada demais.

**O que realmente falta não é lógica, é a camada de atores.** Zero toques no mundo aqui significa: nenhuma esteira que mova item visível, nenhum cabo que ligue dois pontos, nenhum mesh. O comprador de um "Factory Kit" espera colocar a esteira no mapa e ver a caixa andar.

**O que falta:** atores + meshes de placeholder para esteira, cano, gerador, trilho; mapa de demonstração de uma fábrica pequena funcionando; replicação. É o pacote com maior distância entre "código pronto" e "produto".

**Esforço:** G (mês+), quase todo em conteúdo.

---

### 🟡 6. Combate

**Entra:** 6.506 linhas — o maior bucket. `SBCombatComponent`, comportamentos de arma (hitscan, projétil, recarga), combo, cobertura, defesa, desmembramento, execução, hit trace, lock-on, motion warp, poise, stealth, `SBAIController` e tarefas de StateTree.

**Riscos concretos:** só 3 arquivos replicados num pacote de combate; arrasta `StateTree`, `GameplayStateTree` e `SmartObjects`; e combate é a categoria mais lotada e mais criticada da loja — sem animação, VFX e som próprios, a demo parece vazia por mais correto que o código esteja.

**Veredito:** não é o primeiro produto, apesar de ser o maior. Reavaliar depois do nº 1.

---

### 🟡 7. Construção · 8. Save & Persistência · 9. Mundo & Clima · 10. Interação

Quatro pacotes bons demais para jogar fora e pequenos demais para vender sozinhos:

- **Construção** (742, 16 specs, replicado, 5 toques no mundo) — o `SBBuildingComponent` é um dos raros que traça e spawna de verdade. Anexo natural do nº 1.
- **Save & Persistência** (1.050) — só **8 specs** para o sistema que, se falhar, corrompe o save do jogador. É o pacote onde a cobertura precisa subir antes de qualquer coisa. Vai embutido nos outros.
- **Mundo & Clima** (1.501) — clima, regiões, portais, simulação em segundo plano. Fragmentado; talvez um "World Streaming & Weather" depois.
- **Interação** (906, 667 de teste) — só 2 funções Blueprint em 906 linhas: a superfície pública está pequena demais para um sistema de interação. Anexo do nº 1.

---

### 🔴 11. Câmera & Anim Layers · 12. UI/HUD

`SBCameraComponent` (248) com **1 função Blueprint** e 4 specs; `SBAnimLayerManagerComponent` (175) sem uma única animação; `09_SandboxUI` com 524 linhas e **zero widget `.uasset`**.

Câmera, animação e UI são os três domínios onde o comprador julga pelo que vê nos primeiros dez segundos. Sem conteúdo, não há o que ver. **Não vender.** Servem como infraestrutura interna e como parte da demo dos outros pacotes.

---

### 🔴 13. Movimentação — *o pacote que você citou, e o que a medição diz dele*

**O que é real:** `SBMovementComponent` (580 linhas, 27 toques no mundo) dirige o `CharacterMovementComponent` de verdade — sprint altera `MaxWalkSpeed`, crouch chama `Character->Crouch()`/`UnCrouch()` com validação de teto pela engine. A pilha de comportamentos com `SBMovementModifierAggregator` e `SBMovementConfigDataAsset` é arquitetura decente e testada (40 specs).

**O que não é:**

| Componente | Linhas | Toques no mundo | O que o comprador espera |
|---|---|---|---|
| `SBParkourComponent` | 120 | 0 | trace de parede, motion warp, animação de vault |
| `SBGliderComponent` | 143 | 0 | voo, física, controle |
| `SBSwimComponent` | 158 | 0 | detecção de água, movimento aquático |
| `SBMountComponent` | 133 | 0 | montar, dirigir a montaria |
| `SBFootIKComponent` | 101 | 0 | trace de pé, IK no esqueleto |
| `SBZiplineComponent` | 143 | 4 | deslizar no cabo |
| `SBGrappleComponent` | 156 | 3 | disparar, prender, puxar |

São máquinas de estado que decidem *que* ação caberia e emitem eventos — sem percepção e sem movimento. E o `SBMovementComponent` não tem replicação nenhuma, num gênero em que rede é a primeira pergunta.

**Veredito:** vendido hoje como "Advanced Movement System", isto vira reembolso e avaliação de uma estrela na primeira semana. Há duas saídas honestas:

1. **Vender o que é** — um "Movement Behavior Stack": arquitetura de comportamentos e modificadores com sprint e crouch de exemplo. Produto pequeno, honesto, categoria pouco disputada. Esforço P/M.
2. **Terminar de verdade** — trace, motion warping, animação e replicação para dois ou três traversals. Isso é mês(es) e é jogo, não plugin. **Só faz sentido se o BLACK VEIL precisar.**

O que não dá é listar sete traversals na página.

---

### 🔴 14. Veículos & Mechas

`SBVehicleComponent` (320) com zero toques no mundo, `SBMechComponent` (242) com 1, aeronave e embarcação com zero, espaçonave com 8. Mesmo diagnóstico do nº 13, agravado: veículo é comprado *pela* física. Sem ChaosVehicles por baixo e sem mesh, não há produto. **Não vender.** Fica como sistema interno do jogo.

---

### ⚫ 15. Fundação (core)

`SBGameMode`, `SBGameState`, `SBPlayerController`, `SBCharacter`, `SBEventSubsystem`, `SBComponentFactory`, `SBInputSubsystem`, tags, settings, behavior stack. 3.909 linhas que **todo pacote acima carrega junto**.

Não é produto: é o que precisa estar limpo (sem `ModularGameplayActors`, com testes fora do runtime) antes que qualquer pacote possa ser empacotado. É o Portão B do outro documento, e é o gargalo comum dos quinze.

---

## 4. Sequência recomendada

```
Fundação limpa (Portão B)  ─┬─► 3. Performance & Debug ──► piloto publicado
                            │
                            └─► 1. Inventário+Crafting+Interação ──► produto principal
                                        │
                                        ├─► 7. Construção (anexo)
                                        └─► 2. Atributos & Efeitos
                                                    │
                                                    └─► 4. Vitais (add-on)

                                 depois, e só se o anterior vender:
                                 6. Combate · 5. Fábrica · 9. Mundo

                                 nunca como estão hoje:
                                 11 · 12 · 13 · 14
```

**Por que o Performance & Debug antes do Inventário:** não é o produto mais valioso, é o que atravessa a revisão da Fab primeiro, porque é o único que não depende de conteúdo artístico para demonstrar. O objetivo dele é aprender o funil numa página que não importa tanto.

**Por que o Inventário é o produto principal:** é o mais testado (57 specs), o mais replicado (12 arquivos) e o mais fundo (1.168 linhas no componente central) — e o único cujo diferencial arquitetural (fragmentos de item) o comprador reconhece de imediato.

---

## 5. Os três números que reaparecem em todo pacote

1. **Conteúdo: zero.** Nenhum plugin tem um `.uasset`. Todo pacote acima tem "mapa de demonstração" na lista do que falta, e em quase todos isso é a maior parte do trabalho restante.
2. **Replicação: irregular.** Inventário e atributos são replicados; movimento, vitais, fábrica e performance não são. A página de cada produto precisa dizer a verdade sobre isso antes que o comprador descubra.
3. **Fundação embutida: 3.909 linhas em todos.** Enquanto ela não fechar o Portão B, nenhum dos quinze pode ser empacotado — e por isso a fundação é a única tarefa que vale a pena fazer antes de escolher qual produto sai primeiro.
