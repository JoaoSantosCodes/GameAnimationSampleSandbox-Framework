# 🧬 BLACK VEIL — Camada Survival Sandbox

> Extensão do [[01_BlackVeil_GDD|GDD]]. Transforma BLACK VEIL de survival horror linear em
> **survival horror + survival sandbox**, com mundo persistente, construção e recursos.

**Registrado em**: 05/09/2026

---

> [!IMPORTANT] Esta direção resolve a pendência de arquitetura
> As duas versões anteriores deste conjunto de documentos alertavam para uma tensão entre
> BLACK VEIL (espaço fechado, recursos escassos) e o Sandbox Framework (automação industrial,
> mundo aberto, construção).
>
> **Com a camada survival, essa tensão deixa de existir.** A verificação em §20 mostra que
> praticamente todo sistema proposto aqui **já está implementado** no framework. O alerta
> registrado em [[00_BlackVeil_Conceito]] §11 e [[01_BlackVeil_GDD]] §"Pendência aberta" fica
> **superado por este documento**.

---

## 1. A regra que rege tudo

> **Não fazer "ARK com monstros".**
>
> A sobrevivência precisa ser **parte do horror**, não um sistema paralelo a ele.

Corolário prático: **evitar barras irritantes**. Todo sistema de sobrevivência deve existir para
gerar **decisão**, não para gerar manutenção.

O teste para aprovar qualquer mecânica desta camada:

> *"Ela cria um momento em que o jogador precisa escolher entre duas coisas que quer?"*

Se a resposta for não, ela é ruído.

---

## 2. Estatísticas de sobrevivência

Além de Vida, Munição e Inventário:

| Sistema | Papel no horror |
| :--- | :--- |
| 🍖 **Fome** | Obriga a sair do seguro |
| 💧 **Sede** | idem, com ciclo mais curto |
| 🌡️ **Temperatura** | Torna a superfície e a noite hostis |
| 😴 **Fadiga** | Pune o jogador que evita descansar por medo |
| 🩸 **Sangramento** | **Deixa rastro** — criaturas podem seguir |
| 🦴 **Fraturas** | Degradam capacidade, não só vida |
| 🦠 **Infecção / contaminação** | Ameaça de prazo, não de impacto |
| 🧠 **Exposição ao ECHO** | Sistema exclusivo — ver §13 |
| 🔋 **Energia** | Recurso de decisão estratégica — ver §14 |
| 🛠️ **Durabilidade** | Faz equipamento ser consumível, não permanente |

---

## 3. Mundo parcialmente aberto

A AURORA-9 deixa de ser um laboratório linear.

```
                  AURORA-9
                     │
          ┌──────────┴──────────┐
      SUPERFÍCIE             SUBSOLO
          │                     │
     Floresta             Setor Industrial
     Acampamentos         Hospital
     Instalações          Residencial
     abandonadas          Laboratórios
                          Arquivo
                          Contenção ECHO
                          NÚCLEO
```

> **A consequência de design mais importante:** o jogador **precisa sair do laboratório para
> sobreviver**. O lugar seguro não tem comida.

## 4. Superfície e ciclo dia/noite

A superfície foi abandonada. Área florestal extensa contendo: animais · criaturas contaminadas ·
instalações abandonadas · veículos · cabanas · torres · depósitos · túneis · rios · cavernas ·
**entradas alternativas para a instalação**.

| Período | Caráter |
| :--- | :--- |
| **Dia** | Exploração |
| **Noite** | Perigo — **algumas criaturas só aparecem à noite** |

## 5. Construção de abrigo

```
┌───────────────┐
│   PAREDE      │
│   🔥 FOGUEIRA │
│   📦 BAÚ      │
└───────────────┘
```

**Componentes:** paredes · portas · teto · janelas · barricadas · armários · baús · bancada ·
fogueira · gerador · iluminação · estação médica.

> **A regra que impede virar ARK:** quanto mais **barulho e energia** você gera, maior a chance
> de atrair criaturas.
>
> Construir uma base grande é **perigoso**. A progressão de construção tem custo crescente de
> risco, não só de recurso.

## 6. Crafting

| Categoria | Itens |
| :--- | :--- |
| **Sobrevivência** | Bandagem · comida · purificador de água · fogueira · filtro · lanterna improvisada |
| **Medicina** | Kit médico · antibiótico · estimulante · coagulante · tratamento contra contaminação |
| **Equipamentos** | Mochila · coldre · filtros · baterias · ferramentas · peças eletrônicas |
| **Defesa** | Barricadas · armadilhas · alarmes · minas improvisadas · geradores |

## 7. Inventário por peso e espaço

```
INVENTÁRIO
────────────────────
Mochila 18 / 24 kg

🔫 Pistola      🔧 Ferramentas
🔪 Faca         🥫 Alimento
💊 Medkit       🔋 Bateria
💧 Água         🧪 Amostra ECHO
────────────────────
```

As decisões que isso força:

> *"Levo mais munição ou mais comida?"*
> *"Levo o gerador ou o equipamento médico?"*

## 8. Criaturas vivendo no mundo

Deixam de aparecer apenas quando a história manda.

| Criatura | Comportamento no mundo |
| :--- | :--- |
| **Hollow** | Comum. Floresta, hospital, estacionamento, túneis. |
| **Mimic** | Raro. Pode passar horas sem aparecer. Observa o jogador, imita NPCs e vozes, **segue**, e ataca quando ele está vulnerável. |
| **Witness** | Extremamente perigoso. Aprende rotas, esconderijos, padrões e comportamento do jogador. |

> **O mundo deixa de ser previsível** — e essa imprevisibilidade é gerada por comportamento, não
> por spawn aleatório.

## 9. Animais e fauna alterada

Animais normais: cervos · lobos · javalis · aves · pequenos animais.

Alguns foram expostos ao ECHO.

### ECHO Wolf
Não é um lobo gigante. **Imita sons humanos.**

O jogador atravessa a floresta e ouve:

> — Socorro...

Ele pensa que encontrou alguém.

## 10. Clima dinâmico

Chuva · tempestade · neblina · frio · calor · noite · vento — **todos interferindo no gameplay**.

| Clima | Vantagem | Custo |
| :--- | :--- | :--- |
| **Chuva** | Reduz percepção auditiva dos inimigos | Aumenta consumo de equipamentos elétricos |
| **Neblina** | — | Reduz visão · aumenta encontros surpresa |
| **Tempestade** | — | Desliga energia · quebra equipamentos · **abre portas automáticas** · ativa sistemas antigos |

Note que a tempestade é a única que **muda o estado do mundo** — é o clima que o Horror Director
deve usar para eventos narrativos.

## 11. Ferimentos localizados

Não existe apenas `100 HP → 0 HP`.

```
       CABEÇA
          │
   🟥 ── CORPO ── 🟥
          │
       🟥   🟥
```

| Ferimento | Consequência |
| :--- | :--- |
| Braço ferido | Piora precisão |
| Perna ferida | Reduz velocidade |
| Costelas quebradas | Reduz stamina |
| **Sangramento** | **Deixa rastro** |

> A consequência assustadora: **o sangue permite que certas criaturas encontrem você.**
> O ferimento deixa de ser um número e vira uma coleira.

## 12. Energia — o sistema de decisão

A AURORA-9 tem rede elétrica antiga. O jogador restaura setores, mas a energia é **limitada**.

```
GERADOR CENTRAL
      ├── Hospital
      ├── Laboratório
      ├── Elevadores
      ├── Segurança
      └── Residencial
```

| Escolha | Ganha | Perde |
| :--- | :--- | :--- |
| **Energia no Hospital** | Acesso a equipamentos médicos | Segurança desligada |
| **Energia na Segurança** | Portas e sistemas de defesa | Hospital no escuro |

Este é provavelmente o sistema mais forte da camada survival: transforma infraestrutura em
dilema, e o dilema é **reversível** — o jogador pode religar depois, ao custo de atravessar o
mapa de novo.

## 13. ECHO Exposure — o sistema exclusivo

| Exposição | Efeito |
| :---: | :--- |
| **10%** | Pequenos ruídos |
| **25%** | Vozes |
| **40%** | Objetos mudam de posição |
| **60%** | NPCs aparecem onde não deveriam |
| **80%** | O mapa apresenta alterações |
| **100%** | O ECHO interfere diretamente na realidade do jogador |

> **A vantagem que inverte o sistema:** exposição alta permite acessar **memórias que jogadores
> com exposição baixa não conseguem perceber**.
>
> Ficar contaminado é ruim — mas revela a verdade.

Este é o único sistema em que "jogar mal" é uma estratégia legítima. É o que impede a camada
survival de ser apenas manutenção, e o que a costura à narrativa.

## 14. Veículos

Caminhonete · SUV · veículo militar · quadriciclo · van.

Exigem **combustível · bateria · manutenção**. E podem quebrar.

> Você volta para a base durante uma tempestade. O carro para. Silêncio.
> Você precisa sair para procurar uma peça.
> E então escuta uma voz vindo da floresta.

## 15. NPCs e sobreviventes

Espalhados pela AURORA-9. Podem ajudar · trocar itens · dar missões · acompanhar · **morrer** ·
**desaparecer**.

> **A mecânica perfeita para BLACK VEIL:** nem todo sobrevivente é realmente humano.

O Mimic pode assumir a identidade de um NPC. Você encontra "Nora". Ela sabe tudo sobre você.
Mas há algo errado.

**Confiar ou não?** — e a decisão tem custo real, porque um companheiro carrega itens e abre
caminhos.

## 16. Base principal — progressão

| Nível | Conteúdo |
| :---: | :--- |
| 1 | Abrigo improvisado |
| 2 | Bancada + armazenamento |
| 3 | Gerador + laboratório |
| 4 | Enfermaria + oficina |
| 5 | Base avançada |

> **O problema:** o ECHO pode descobrir sua localização. A base **sofre ataques**.

Progressão que aumenta capacidade e exposição ao mesmo tempo — coerente com §5.

## 17. Bosses como ameaças de mundo

Os bosses da campanha continuam existindo, mas ganham **versões dinâmicas no mundo**:

| Boss | Manifestação no survival |
| :--- | :--- |
| **The Warden** | Aparece durante um apagão |
| **The Choir** | **Invade sua base** |
| **The Mimic** | Se infiltra entre seus sobreviventes |

> Deixam de ser "uma luta em uma sala" e passam a ser **ameaças do mundo**.

## 18. Estrutura em três camadas

```
              BLACK VEIL
       ┌──────────┼──────────┐
    CAMPANHA   SURVIVAL   EXPLORAÇÃO
    História    Base       Mundo
    Bosses      Craft      Recursos
    Mistério    Fome       Criaturas
    Puzzles     Sede       Eventos
       └──────────┼──────────┘
                  │
               ECHO
```

O ECHO é o que unifica as três camadas — é ameaça narrativa, sistema de sobrevivência
(exposição) e gerador de eventos de mundo simultaneamente.

## 19. Modos de jogo

| Modo | Escopo |
| :--- | :--- |
| **Story Mode** | Campanha cinematográfica, focada na narrativa |
| **Survival Mode** | Mapa persistente, recursos, construção, crafting, eventos, criaturas, NPCs e bosses |
| **Co-op Survival** *(posterior)* | 2–4 jogadores sobrevivendo juntos na AURORA-9 |

---

# 20. 🎯 MAPEAMENTO PARA O SANDBOX FRAMEWORK

Verificação feita no código real em 05/09/2026. **A cobertura é quase total.**

## Sistemas que JÁ EXISTEM implementados

| Necessidade de BLACK VEIL | Componente do framework | Plugin |
| :--- | :--- | :--- |
| Fome, sede, nutrição | `USBMetaBolicNutritionComponent` | 05_SandboxCharacter |
| Temperatura corporal | `USBThermalRegulationComponent` | 05_SandboxCharacter |
| Fraturas, sangramento, trauma | `USBTraumaInjuryComponent` | 05_SandboxCharacter |
| Infecção, patógenos, febre | `USBImmuneSystemComponent` | 05_SandboxCharacter |
| Contaminação com efeitos progressivos | `USBRadiationExposureComponent` | 05_SandboxCharacter |
| Atmosfera tóxica, filtros | `USBAtmosphericSafetyComponent` | 05_SandboxCharacter |
| Enfermaria, cirurgia, próteses | `USBSurgeryProstheticsComponent` | 05_SandboxCharacter |
| Doenças e afligimentos | `USBAfflictionComponent` | 05_SandboxCharacter |
| Domesticação de animais | `USBDomesticationComponent` | 05_SandboxCharacter |
| Veículos terrestres | `USBVehicleComponent` | 05_SandboxCharacter |
| Natação, oxigênio | `USBSwimComponent` | 05_SandboxCharacter |
| **Construção de abrigo** | `USBBuildingComponent` + `ASBBuildingPiece` | 08_SandboxInventory |
| **Integridade estrutural / colapso** | `USBStructuralIntegrityComponent` | 08_SandboxInventory |
| **Crafting** | `USBCraftingComponent` + `ASBCraftingStation` | 08_SandboxInventory |
| **Rede elétrica e geradores** | `USBPowerGridComponent` | 08_SandboxInventory |
| Baús e armazenamento | `ASBContainerChest` | 08_SandboxInventory |
| Nós de recurso no mundo | `ASBResourceNode` | 08_SandboxInventory |
| Loot físico no chão | `ASBPhysicalLootDrop` | 08_SandboxInventory |
| Plantio / cultivo | `USBDynamicCropComponent` | 08_SandboxInventory |
| NPCs comerciantes | `USBMerchantComponent` | 08_SandboxInventory |
| Inventário com peso | `USBInventoryComponent` | 08_SandboxInventory |
| **Clima dinâmico e ciclo dia/noite** | `USBWeatherSubsystem` | 04_SandboxCore |
| **Mundo vivo fora da tela** | `USBSandboxBackgroundSimSubsystem` | 04_SandboxCore |
| **Persistência de mundo por GUID** | `USBSandboxPersistenceSubsystem` | 04_SandboxCore |
| Regiões e descoberta de áreas | `USBRegionSubsystem` | 04_SandboxCore |
| Transição entre áreas | `USBPortalSubsystem` | 04_SandboxCore |
| Save criptografado e assinado | `USBSaveSubsystemConcrete` | 04_SandboxCore |
| Stealth e percepção | `USBStealthComponent` | 06_SandboxCombat |
| Cobertura | `USBCoverComponent` | 06_SandboxCombat |
| IA com StateTree e Smart Objects | (06_SandboxCombat) | 06_SandboxCombat |
| Ligar/desligar módulos inteiros | `USBSandboxFeatureSubsystem` | 04_SandboxCore |

## O que NÃO existe — o trabalho real

| Sistema | Situação | Observação |
| :--- | :--- | :--- |
| **`MemorySystem`** | ❌ Não existe | Coração do jogo. Precisa ser construído do zero. |
| **`HorrorDirector`** | ❌ Não existe | Nada equivalente no framework. |
| **`ExposureSystem` (ECHO)** | ⚠️ Molde existe | `USBRadiationExposureComponent` já modela exposição progressiva com efeitos por faixa. É o ponto de partida — mas a **vantagem** da exposição alta (acessar memórias ocultas) é lógica nova. |
| **IA do Mimic** | ❌ Não existe | Copiar aparência/voz/comportamento de NPC observado. |
| **IA do Witness** | ❌ Não existe | Percepção baseada em **memória de ações recentes**, não em visão/audição. Inverte o modelo de percepção existente. |
| **Consciência coletiva do Choir** | ❌ Não existe | Transferência de aprendizado entre instâncias de IA. |
| **Narrativa não confiável** | ❌ Não existe | `Reliability` / `Corruption` em memórias. |
| **Câmeras que mostram o futuro** | ❌ Não existe | — |

## Conclusão do mapeamento

> As 134 fases do framework construíram **quase exatamente a fundação que a camada survival de
> BLACK VEIL precisa**. O que falta é o que torna o jogo *dele mesmo*: memória, horror dirigido
> e as três IAs de assinatura.

Isso responde à decisão que estava pendente:

- ❌ ~~BLACK VEIL usa um subconjunto e desativa o resto~~
- ❌ ~~O framework se reorienta e descarta o que foi feito~~
- ✅ **BLACK VEIL é o produto que dá finalidade ao framework existente.** O que sobra
  (espaçonaves 6-DOF, mechas, aeronaves, elevador espacial, esteiras industriais, trens) é
  desativado via `USBSandboxFeatureSubsystem`, sem ser removido.

### Ressalva honesta
"Existe implementado" não é o mesmo que "pronto para o jogo". Esses componentes foram
construídos para um sandbox de automação industrial e têm suítes de teste, mas **nunca foram
ajustados para ritmo de horror**. Fome que drena em minutos serve a um survival; a mesma curva
pode destruir a tensão de uma cena de perseguição.

O trabalho real nesses sistemas é **tuning e integração**, não implementação — o que é uma
ordem de magnitude mais barato.

---

## 21. Próximo passo

Com o mapeamento resolvido, o próximo documento continua sendo o **Documento de Produção do
Vertical Slice** ([[01_BlackVeil_GDD]] §54) — agora com uma vantagem: é possível especificar
quais componentes do framework cada sala usa, em vez de descrever sistemas a construir.
