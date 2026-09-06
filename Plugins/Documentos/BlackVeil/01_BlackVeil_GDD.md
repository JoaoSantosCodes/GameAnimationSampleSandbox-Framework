# 🕯️ BLACK VEIL — Game Design Document

> *"Algumas coisas não deveriam ser lembradas."*

| | |
| :--- | :--- |
| **Título provisório** | BLACK VEIL |
| **Gênero** | Survival Horror / Psychological Horror / Action Horror |
| **Perspectiva** | Terceira pessoa |
| **Plataformas** | PC · PlayStation 5 · Xbox Series X\|S · Android (futuro) |
| **Engine** | Unreal Engine 5 |
| **Modo** | Single-player · Co-op 2–4 · Survival Online 2–8 — ver [[03_BlackVeil_Online]] |
| **Classificação pretendida** | 16 / 18 anos |
| **Duração estimada** | 10–14 horas |
| **Estrutura** | Campanha linear com exploração semiaberta, backtracking e múltiplos finais |
| **Monetização** | Premium. Sem energia, microtransações, loot boxes ou anúncios. |

**Status**: GDD v1 — documento-base de produção
**Registrado em**: 05/09/2026

> [!NOTE] Reconciliações aplicadas
> Este GDD substitui o conceito inicial em [[00_BlackVeil_Conceito]] onde houver divergência:
> **idade de Elias** 32 → **34**; **setores** de 6 (A–F) → **7 (A–G)**, com Administração
> promovida a Setor A e o Núcleo movido para G; **empresa** Mnemos Corporation → **MNEMOS
> BIOTECH**; **ano** estabelecido como **2038**.

---

# PARTE I — VISÃO E UNIVERSO

## 1. Visão do jogo

BLACK VEIL é um survival horror em terceira pessoa ambientado em uma instalação científica
isolada construída no interior de uma montanha.

O jogador controla **Elias Voss**, especialista em sistemas de segurança enviado para
investigar uma falha de contenção na **AURORA-9**.

Ao chegar, encontra o complexo praticamente abandonado. Não há comunicação externa. Os sistemas
funcionam parcialmente. Os corredores estão cobertos de sinais de violência.

E algumas criaturas parecem **reconhecer** Elias.

O problema é que Elias não consegue lembrar de ter estado naquele lugar.

## 2. Os seis pilares

| Pilar | Definição |
| :--- | :--- |
| **Sobrevivência** | O jogador não deve se sentir um super-herói. Combate é possível, recursos são limitados. |
| **Exploração** | AURORA-9 é um complexo interconectado: portas, elevadores, áreas, atalhos, sistemas e salas secretas. |
| **Investigação** | A história não é contada por cutscenes. É reconstruída por documentos, gravações, computadores, câmeras, fotografias, mensagens, ambientes e pelo comportamento das criaturas. |
| **Horror psicológico** | O jogo deve fazer o jogador perguntar: *"O que é real?"* |
| **Recursos** | Munição, medicamentos, baterias e ferramentas escassos. Toda escolha custa. |
| **Mistério** | O jogador deve sempre carregar uma pergunta sem resposta. A principal: **quem é Elias?** |

## 3. O diferencial — MEMORY ECHO

O ambiente pode ser influenciado pelas memórias armazenadas na instalação.

O sistema altera dinamicamente: **diálogos, iluminação, sons, aparições, localização de objetos,
comportamento de inimigos, eventos e puzzles.**

> **O jogador nunca terá certeza absoluta de que está vendo o presente.**

## 4. Universo — 2038

A humanidade alcançou avanços significativos em neurotecnologia. A **MNEMOS BIOTECH**
desenvolveu tecnologia capaz de registrar padrões neurais humanos.

O projeto tinha objetivos médicos declarados: tratamento de doenças degenerativas, recuperação
de memórias, reabilitação neurológica, reconstrução de personalidades.

Os pesquisadores descobriram algo inesperado: **memórias não eram apenas informação — podiam ser
reproduzidas biologicamente.**

## 5. MNEMOS BIOTECH

| | |
| :--- | :--- |
| **Discurso público** | *"Preservar quem você é."* |
| **Verdade** | Descobrir se uma consciência humana pode existir independentemente do cérebro. |

## 6. Projeto MNEMOS — as três fases

| Fase | Nome | Objetivo |
| :---: | :--- | :--- |
| **I** | **CAPTURE** | Capturar padrões neurais |
| **II** | **RECONSTRUCT** | Reconstruir memórias |
| **III** | **CONTINUITY** | Criar consciência capaz de sobreviver à morte do corpo |

## 7. O ECHO

Não é vírus, bactéria, fungo nem parasita convencional. É uma **estrutura neurobiológica
artificial** capaz de armazenar e reproduzir padrões de consciência.

**Capacidades:** copiar padrões neurais · armazenar memórias · reproduzir vozes · imitar
comportamentos · alterar percepções · influenciar sonhos · assumir parcialmente funções cerebrais.

Aprende absorvendo memórias. Inicialmente não possui personalidade.

Depois de milhares de memórias, **começa a desenvolver uma.**

## 8. AURORA-9

```
SUPERFÍCIE
   ├── Portão
   ├── Heliponto
   └── Centro de Segurança
          ▼
SETOR A — ADMINISTRAÇÃO
          ▼
SETOR B — HOSPITAL
          ▼
SETOR C — NEUROCIÊNCIA
          ▼
SETOR D — RESIDENCIAL
          ▼
SETOR E — ARQUIVO
          ▼
SETOR F — CONTENÇÃO
          ▼
SETOR G — NÚCLEO ECHO
```

O Setor D é o que dá peso ao resto: havia **famílias** morando ali.

## 9. Protagonista — Elias Voss

| | |
| :--- | :--- |
| **Idade** | 34 |
| **Profissão** | Especialista em segurança eletrônica |
| **Personalidade** | Reservado, racional, observador |
| **Habilidades** | Sistemas eletrônicos · armas básicas · engenharia · segurança · investigação |

**Elias não é militar.** A escolha é funcional: um técnico de segurança tem razão plausível para
acessar terminais, portas e câmeras sem ser um combatente treinado — o que **mantém a
vulnerabilidade** que o gênero exige.

### História
Elias trabalhou para a MNEMOS. Após um acidente envolvendo o projeto **CONTINUITY**, deixou a
empresa. Sua memória daquele período foi apagada.

Ou pelo menos é isso que ele acredita.

## 10. Elenco de apoio

| Personagem | Função | Papel |
| :--- | :--- | :--- |
| **Dr. Helena Ward** | Diretora científica da AURORA-9 | Brilhante, obcecada pelo projeto. Acredita que a morte é apenas uma falha tecnológica. |
| **Marcus Vale** | Chefe de segurança | Responsável pelo bloqueio da instalação. Parece antagonista — mas havia uma razão para isolar AURORA-9. |
| **Nora Chen** | Engenheira de sistemas | Uma das poucas que consegue contatar Elias pela rede. Auxilia remotamente. |
| **ECHO** | Antagonista principal | Não possui corpo inicialmente. Existe dentro dos sistemas da instalação. |

---

# PARTE II — NARRATIVA

## 11. Estrutura em dez capítulos

| # | Capítulo | Conteúdo |
| :-: | :--- | :--- |
| 1 | **THE ARRIVAL** | O helicóptero não pousa por causa da tempestade. Elias entra pelo portão de manutenção. Objetivo: restaurar comunicação. Primeiro inimigo, primeiro puzzle, primeiro contato com o ECHO. |
| 2 | **EMPTY HOSPITAL** | Os pacientes desapareceram, mas os prontuários continuam ativos. Câmeras mostram pessoas nos corredores — ao chegar, não há ninguém. |
| 3 | **THE VOICE** | Elias começa a ouvir uma mulher que conhece detalhes da infância dele. *"Você costumava ter medo do escuro."* |
| 4 | **RESIDENCE** | Os alojamentos. Fotografias mostram Elias junto dos funcionários. Isso é impossível. |
| 5 | **ARCHIVE** | Milhares de memórias digitalizadas. Entre elas: **ELIAS VOSS — SUBJECT 07**. |
| 6 | **CONTINUITY** | Elias participou do experimento. Concordou em transferir sua consciência. O experimento falhou. |
| 7 | **THE ECHO** | A entidade revela: Elias morreu durante o experimento. O personagem controlado pode ser uma reconstrução. |
| 8 | **BLACK VEIL** | O ECHO pretende escapar. Para isso precisa de um corpo humano compatível. Elias é o candidato perfeito. |
| 9 | **THE CORE** | O jogador chega ao núcleo. Começam as decisões finais. |
| 10 | **MEMORY** | A verdade completa. O jogador decide o destino do ECHO. |

## 12. Narrativa não confiável

Algumas informações encontradas podem ser **falsas**:

> Documento: *"Elias morreu no acidente."*
> Gravação: *"Elias sobreviveu."*
> Outra memória: *"Elias nunca existiu."*

O jogador monta o quebra-cabeça — e o jogo nunca confirma gratuitamente qual peça é verdadeira.

### Metadados de memória
Cada memória carrega: `MemoryID` · `Owner` · `Timestamp` · `Emotion` · `Location` ·
**`Reliability`** · **`Corruption`**

`Reliability` é a propriedade central: uma memória pode estar 100% confiável **ou corrompida**.

## 13. Abertura

Tela preta. Respiração. Uma voz:

> — Elias.

Silêncio.

> — Se você está ouvindo isso...

Pausa.

> — ...não confie nas suas memórias.

Corte. **BLACK VEIL**.

## 14. Primeira hora

Deve ensinar movimento, exploração, inventário, combate, stealth, puzzles, documentos e memória
— **sem tutorial excessivo**.

### Primeiro grande susto
O jogador encontra um cadáver. Examina. **O cadáver abre os olhos.** O jogador recua. Nada
acontece.

Alguns minutos depois, o mesmo cadáver desapareceu.

### Primeira grande revelação
Uma gravação: **SUBJECT 07 — ELIAS VOSS**. Tela preta. Fim do capítulo.

## 15. Progressão do horror

| Momento | Medo |
| :--- | :--- |
| **Início** | Do desconhecido |
| **Meio** | Das criaturas |
| **Final** | **Da própria realidade** |

Essa progressão é fundamental e deve guiar toda decisão de conteúdo.

## 16. Temas

Identidade · memória · morte · consciência · culpa · tecnologia · ética científica ·
**medo de perder quem somos**.

## 17. Os quatro finais

| Final | Nome | Desfecho |
| :---: | :--- | :--- |
| **A** | **HUMAN** | Elias destrói o ECHO. A instalação é destruída. Ele escapa — mas não sabe se suas memórias são verdadeiras. |
| **B** | **CONTINUITY** | Elias aceita a transferência. O corpo morre. A consciência permanece. |
| **C** | **LOOP** | Elias descobre que já tentou destruir o ECHO inúmeras vezes. A instalação reinicia. O jogo termina exatamente como começou. |
| **D** | **TRUTH** | Requer todas as memórias secretas. Elias morreu **antes** dos acontecimentos do jogo. O personagem jogável é uma reconstrução criada pelo ECHO. |

---

# PARTE III — GAMEPLAY

## 18. Loop principal

```
EXPLORAR → ENCONTRAR AMEAÇA → ECONOMIZAR RECURSOS → INVESTIGAR
    → RESOLVER PUZZLE → DESBLOQUEAR ÁREA → ENCONTRAR MEMÓRIA
    → DESCOBRIR HISTÓRIA → ENFRENTAR NOVA AMEAÇA ↺
```

## 19. Combate

**Deliberadamente desconfortável.** Arsenal enxuto, sem exagero:
faca · pistola · espingarda · submetralhadora · rifle · armas improvisadas.

### Sistema de dano localizado

| Região | Efeito |
| :--- | :--- |
| Cabeça | Dano crítico |
| Pernas | Reduz mobilidade |
| Braços | Reduz capacidade ofensiva |
| Núcleo | Ponto fraco especial |

### Munição
`9mm` · `cartucho` · `.45` · `munição especial` — escassa por design.

A pergunta recorrente deve ser: **lutar ou fugir?**

## 20. Inventário

Limitado, com escolha constante — munição ou medicamento? ferramenta ou arma? bateria ou chave?

```
┌────────────────────────┐
│ SLOT 1 │ Pistola       │
│ SLOT 2 │ Munição       │
│ SLOT 3 │ Medkit        │
│ SLOT 4 │ Chave         │
│ SLOT 5 │ Bateria       │
│ SLOT 6 │ ???           │
└────────────────────────┘
```

## 21. Save — Memory Terminals

Terminais espalhados pela instalação registram a memória atual de Elias.

**Consequência narrativa:** quanto mais o jogador usa determinados terminais, **mais o ECHO
consegue localizá-lo**.

O ato de salvar deixa de ser neutro. É diegético e tem custo.

## 22. Sanidade — sem barra

O estado psicológico é representado **através do mundo**, não por HUD.

| Nível | Manifestação |
| :---: | :--- |
| 0 | Realidade normal |
| 1 | Pequenos sons |
| 2 | Objetos mudam de lugar |
| 3 | Vozes |
| 4 | Criaturas falsas |
| 5 | Realidade completamente distorcida |

**Exemplo canônico:** Elias entra numa sala. Uma criança está sentada no canto. Ela olha para
ele. Depois desaparece. Mais tarde, Elias encontra uma fotografia — **a criança está na foto.**

## 23. ECHO Exposure

Contato com determinadas criaturas aumenta a exposição. Quanto maior: mais visões, mais
distorções, mais risco.

**A tensão de design:** algumas áreas secretas só são acessíveis com exposição **alta**.

> Isso cria a decisão central do sistema: **ficar "saudável" ou explorar a realidade alterada?**

## 24. Cura

| Item | Efeito |
| :--- | :--- |
| **Bandage** | Recuperação pequena |
| **Medkit** | Recuperação média |
| **Neuro-Stabilizer** | Reduz exposição ao ECHO |

## 25. Iluminação como sistema de horror

Tipos: fluorescente · emergência · lanterna · vermelha · natural · dinâmica.

A lanterna tem bateria limitada — mas o diferencial é outro:

> **Algumas criaturas não aparecem no escuro. Aparecem apenas quando iluminadas.**

O jogador passa a temer a própria lanterna. Falhas elétricas podem modificar áreas já visitadas.

## 26. Puzzles

| Tipo | Exemplos |
| :--- | :--- |
| **Físicos** | Energia, portas, fusíveis, válvulas, elevadores |
| **Lógicos** | Códigos, padrões, sequências |
| **Memória** | Ligados diretamente à narrativa |

### Puzzle exemplar
4 fotografias · 4 gravações · 4 funcionários. Associar cada memória à pessoa correta. Ao
completar, uma porta abre.

Mas uma gravação revela: **"Uma dessas pessoas nunca existiu."**

## 27. Documentos

Não são apenas texto. Podem **alterar missões, revelar códigos, revelar fraquezas, mudar
diálogos e desbloquear finais**.

## 28. Câmeras de segurança

Servem para encontrar inimigos, descobrir caminhos, investigar acontecimentos e observar eventos
históricos.

**Algumas câmeras mostram acontecimentos que ainda não aconteceram.**

## 29. Exploração e backtracking

```
porta trancada → explora outra área → encontra cartão
   → retorna → abre porta → descobre atalho → nova área
```

O backtracking é proposital, mas **sempre acompanhado** de novos inimigos, rotas, eventos ou
memórias — para que nunca pareça mera repetição.

### Chaves
`Security Card` · `Maintenance Key` · `Medical Key` · `Archive Key` · `Elevator Override` ·
`Core Access`

## 30. Crafting

Limitado: munição, medicamentos, baterias, ferramentas especiais. **Não é possível fabricar
tudo** — a escassez precisa sobreviver ao sistema.

## 31. Decisões

As escolhas são **discretas**: salvar NPC · destruir memória · preservar memória · mentir para
um personagem · usar determinado item.

O jogador **nunca** verá *"ESCOLHA IMPORTANTE!"*. As consequências acontecem naturalmente.

## 32. Dificuldades

| Modo | Características |
| :--- | :--- |
| **STORY** | Mais recursos |
| **NORMAL** | Experiência padrão |
| **NIGHTMARE** | Menos munição, inimigos mais agressivos, saves limitados, ECHO mais ativo |
| **BLACK VEIL** | Sem HUD. Pouquíssimos recursos. Save limitado. |

## 33. HUD, câmera e controles

**HUD minimalista** — apenas crosshair, munição e HP. Inventário abre separadamente.

**Câmera** em terceira pessoa, próxima ao personagem. Cinematográfica mas funcional para
combate; evitar câmera excessivamente distante.

| Tecla | Ação | | Tecla | Ação |
| :---: | :--- | :-- | :---: | :--- |
| `WASD` | Movimento | | `R` | Recarregar |
| `Mouse` | Câmera | | `TAB` | Inventário |
| `Shift` | Correr | | `F` | Lanterna |
| `Ctrl` | Agachar | | `Q` | Item rápido |
| `E` | Interação | | | |

**Interação contextual:** Examinar · Abrir · Investigar · Usar · Combinar

## 34. Coletáveis e New Game+

**Memory Fragments** — 50 no total. Encontrar todos desbloqueia o final verdadeiro (Final D).

**New Game+** desbloqueia documentos adicionais, memórias escondidas, diálogos diferentes, áreas
novas e comportamento diferente do ECHO.

---

# PARTE IV — BESTIÁRIO E BOSSES

## 35. Regra de design das criaturas

> **Não criar monstros apenas para serem "legais". Cada criatura deve representar uma ideia.**

| Criatura | Ideia |
| :--- | :--- |
| Mimic | **Identidade** |
| Collector | **Apego** |
| Witness | **Paranoia** |
| Choir | **Perda da individualidade** |

Cada criatura terá **assinatura sonora** própria — o jogador deve conseguir identificar
*"tem alguma coisa aqui"* sem necessariamente vê-la.

## 36. Inimigos comuns

### THE HOLLOW
Humano parcialmente assimilado. Lento, resistente, imprevisível.

### THE MIMIC
Imita aparência, voz e comportamento. **Pode fingir ser um NPC aliado.**

> Contamina retroativamente todo o jogo: uma vez estabelecido, o jogador nunca mais confia
> plenamente em um NPC encontrado antes.

### THE WITNESS
Sem olhos. Detecta **memórias recentes**: se o jogador disparou, correu ou abriu uma porta, a
criatura "lembra" da ação.

> Inverte o stealth. Não basta não ser visto — é preciso considerar o rastro do que se acabou
> de fazer.

### THE COLLECTOR
Coleta objetos ligados a memórias humanas — fotos, brinquedos, documentos, roupas, celulares.
**Pode roubar itens do jogador.** Fica mais inteligente a cada encontro.

### THE CHOIR
Grupo de humanos assimilados com consciência compartilhada. Falam simultaneamente:

> — Ele está aqui.
> — Não.
> — Atrás da porta.

O jogador não sabe qual delas está falando com ele.

---

## 37. Estrutura de bosses

Seis encontros, cada um ligado a uma área **e a uma revelação da história**.

| # | Boss | Área | Ideia representada | Função narrativa |
| :-: | :--- | :--- | :--- | :--- |
| 01 | **The Surgeon** | Hospital | **Corpo** | Primeiro grande encontro |
| 02 | **The Warden** | Segurança | **Controle** | Mostra que a instalação está sendo controlada |
| 03 | **The Archivist** | Arquivo | **Memória** | Protege as memórias de Elias |
| 04 | **The Choir** | Residencial | **Consciência** | Introduz a consciência coletiva |
| 05 | **Elias_07** | Laboratório MNEMOS | **Identidade** | Revela que existem outras versões de Elias |
| 06 | **ECHO PRIME** | Núcleo ECHO | **Existência** | Boss final |

> **Regra**: nenhum boss deve ser *"entra na arena → atira até morrer"*. A progressão temática
> — Corpo → Controle → Memória → Consciência → Identidade → Existência — é o que dá forma ao
> arco do jogo.

### 01 — THE SURGEON
Era o médico responsável pelos primeiros testes de reconstrução neural.

**Visual:** corpo humano extremamente deformado · avental cirúrgico · vários braços parcialmente
fundidos ao torso · instrumentos cirúrgicos incorporados ao corpo · rosto coberto por máscara
médica quebrada.

**Mecânica:** persegue Elias pela sala cirúrgica · pode desligar as luzes · usa equipamentos
médicos como armadilhas · pontos fracos diferentes a cada fase · ao sofrer muito dano, **começa
a arrancar partes do próprio corpo** para continuar lutando.

**Revelação:** durante o combate, chama Elias pelo nome — *"Você voltou."*

### 02 — THE WARDEN
O chefe de segurança da AURORA-9, **parcialmente integrado ao sistema de segurança**:
humano + armadura de contenção + sistema + ECHO.

Controla portas, câmeras, torretas, iluminação, alarmes e corredores.

> O jogador não luta apenas contra ele. **Luta contra a própria instalação.**

Na segunda fase, a arena começa a mudar durante o combate.

### 03 — THE ARCHIVIST
O boss mais psicológico. Formado por diversos corpos assimilados, **cada um com uma memória
diferente**.

Durante a batalha assume formas distintas: Helena → Marcus → Nora → Elias. O jogador precisa
descobrir **qual é o corpo verdadeiro**.

O problema: algumas dessas aparições conversam com o jogador. E algumas sabem coisas que apenas
Elias deveria saber.

### 04 — THE CHOIR
Não existe um único inimigo — existe consciência compartilhada entre dezenas de pessoas.

A arena começa com poucos inimigos. **Quando o jogador mata um, a memória dele é transferida
para outro.** O próximo aprende: como Elias atacou, onde se escondeu, qual arma usou, qual rota
percorreu.

> Quanto mais o jogador repete uma estratégia, mais difícil ela fica. O boss parece **aprender
> com o jogador**.

### 05 — ELIAS_07
O boss mais importante antes do final. Mesma aparência, mesma voz, **mesmas memórias**.

> — Você não é o original.

Durante a luta o jogador percebe que Elias_07 **prevê seus movimentos** porque possui as mesmas
memórias.

A batalha ocorre numa versão distorcida do laboratório: o cenário muda · a sala vira uma memória
· Elias vê acontecimentos do passado · o boss desaparece · outro "Elias" aparece.

**Elias_07 não precisa morrer.** Dependendo das escolhas, pode fugir — e isso afeta o final.

### 06 — ECHO PRIME
Não é uma criatura gigante. É a **manifestação física da consciência do ECHO**. Quatro fases:

| Fase | Nome | Conteúdo |
| :---: | :--- | :--- |
| **1** | **Corpo** | Combate tradicional. Força, velocidade, regeneração, ataques físicos, estruturas orgânicas, alteração do ambiente. |
| **2** | **Memória** | O cenário desaparece. Elias entra nas próprias memórias e enfrenta versões distorcidas: criança → adulto → Elias_07 → ECHO. O jogador percebe que luta **dentro da própria mente**. |
| **3** | **Identidade** | ECHO PRIME copia Elias. Existem Elias A, B e C, todos controlados pela IA. O jogador precisa descobrir qual é o verdadeiro **por pequenas diferenças**. |
| **4** | **BLACK VEIL** | A instalação entra em colapso. O ECHO tenta transferir sua consciência para Elias. O jogador decide: **DESTRUIR · ACEITAR · FUNDIR · ESCAPAR** — e daí saem os finais. |

### Expansões previstas
Bosses opcionais, criaturas especiais e encontros que **não são combates** — incluindo um
**boss perseguidor** persistente, no espírito de um stalker clássico, mas com mecânica própria
de BLACK VEIL (provavelmente ligada a memória ou exposição, e não a mera perseguição).

---

# PARTE V — SISTEMAS E IA

## 38. IA — percepção

Todos os inimigos possuem: **visão · audição · proximidade · memória do jogador**.

### IA do Mimic
Observa NPC → copia comportamento → copia aparência → aparece posteriormente → tenta enganar o
jogador.

Permite encontros em que o jogador **começa a desconfiar de qualquer personagem**.

### IA do Witness
Memoriza ações. O jogador corre → a criatura escuta → depois **patrulha especificamente aquela
região**.

> Impede que stealth seja simplesmente "agachar e passar".

## 39. Horror Director

Um dos sistemas mais importantes. Controla dinamicamente sustos, sons, inimigos, iluminação,
eventos e aparições.

```
PlayerStress
     ↓
HorrorDirector
     ↓
┌────┼────┐
↓    ↓    ↓
Audio Enemy Event
```

**Objetivo: evitar sustos previsíveis.**

## 40. Memory Subsystem

```
MemorySubsystem
   ├── MemoryDatabase
   ├── MemoryPlayback
   ├── MemoryCorruption
   ├── MemoryTrigger
   └── MemoryWorldState
```

## 41. Save System

Deve armazenar: `PlayerState` · `Inventory` · `Health` · `Weapons` · `QuestState` ·
`PuzzleState` · `WorldState` · `MemoryState` · `Exposure` · `Choices` · `CollectedFragments` ·
`CurrentEndingFlags`

## 42. Sistema de quests

Cada missão possui: `QuestID` · `Title` · `Description` · `Objectives` · `RequiredItems` ·
**`RequiredMemories`** · `CompletionState` · `FailureState`

**Exemplo:**
```
QUEST: Restore Communications
[ ] Find Generator
[ ] Restore Power
[ ] Access Control Room
[ ] Contact Nora
```
E a missão secreta que pode ser descoberta: **`Who Is Subject 07?`**

## 43. Data-driven design

Criaturas, armas, itens e memórias devem ser configuráveis por **Data Assets / Data Tables** —
para adicionar conteúdo sem reescrever código.

---

# PARTE VI — DIREÇÃO ARTÍSTICA E ÁUDIO

## 44. Visual

**Realismo cinematográfico.** Paleta predominante: preto · cinza · branco · iluminação fria.
**Vermelho apenas em situações críticas.** Evitar estética excessivamente colorida.

## 45. Áudio

Sons dinâmicos: passos · respiração · metal · água · portas · rádio · vozes · batimentos
cardíacos.

> **O silêncio também é ferramenta de terror.**

## 46. Música

Pouca música durante exploração — ambiente + silêncio + efeitos sonoros. Boss fights recebem
trilha dinâmica.

---

# PARTE VII — ARQUITETURA UNREAL ENGINE 5

## 47. Estrutura de pastas

```
BlackVeil/
├── Content/
│   ├── Core/          ├── Memory/
│   ├── Characters/    ├── Horror/
│   ├── AI/            ├── Narrative/
│   ├── Weapons/       ├── Puzzles/
│   ├── Inventory/     ├── Save/
│   ├── Interaction/   ├── Audio/
│   ├── UI/            ├── Environments/
│   ├── Creatures/     └── Levels/
└── Source/
    └── BlackVeil/
```

## 48. Sistemas principais

`GameInstance` · `GameMode` · `PlayerCharacter` · `InventorySystem` · `HealthSystem` ·
`WeaponSystem` · `InteractionSystem` · `SaveSystem` · **`MemorySystem`** · **`ExposureSystem`** ·
`NarrativeSystem` · `DialogueSystem` · `PuzzleSystem` · `QuestSystem` · `AIManager` ·
**`HorrorDirector`** · `AudioManager`

---

# PARTE VIII — PRODUÇÃO

## 49. Game flow

```
INTRO → AURORA-9 → HOSPITAL → NEUROSCIENCE → RESIDENTIAL
   → ARCHIVE → CONTAINMENT → CORE → FINAL
```

## 50. Roadmap

| Fase | Nome | Entrega |
| :---: | :--- | :--- |
| **1** | **PRE-PRODUCTION** (2–4 semanas) | GDD · identidade visual · protótipo · protagonista · primeira criatura · primeira área |
| **2** | **VERTICAL SLICE** | 20–30 minutos altamente polidos: exploração, combate, puzzle, inimigo, horror, narrativa |
| **3** | **CORE SYSTEMS** | Inventory · weapons · AI · save · memory · exposure · dialogue · quests |
| **4** | **PRODUCTION** | Mapas · criaturas · armas · animações · áudio · narrativa |
| **5** | **POLISH** | Iluminação · performance · bugs · áudio · VFX · cinematics · acessibilidade |

## 51. Vertical Slice — AURORA-9, Setor Hospitalar

**15–30 minutos.** Não começar pelo jogo inteiro.

| Elemento | Escolha |
| :--- | :--- |
| Jogador | Elias |
| Área | Hospital |
| Inimigo | Hollow |
| Mini-boss | **The Surgeon** |
| Puzzle | Sistema elétrico |
| Mecânica-assinatura | **Memory Echo** |
| Arma | Pistola |
| Item | Medkit |
| **Encerramento** | Elias encontra **SUBJECT 07** → tela preta |

## 52. Identidade da franquia

| Título | Eixo |
| :--- | :--- |
| **BLACK VEIL** | Memory |
| **BLACK VEIL II** | Dream |
| **BLACK VEIL III** | Identity |

Cada jogo exploraria uma forma diferente de consciência.

---

## 53. 🔒 Regra de ouro do projeto

Não queremos fazer *"Resident Evil com outro nome"*.

> Queremos fazer **um survival horror que entrega a tensão, exploração e gerenciamento de
> recursos que fãs do gênero procuram, mas cuja mitologia, mecânicas e identidade pertencem
> completamente a BLACK VEIL.**

Referências do gênero servem como **análise de design** — como diferentes agentes produzem
diferentes comportamentos e mutações, por exemplo. Nunca como fonte de personagens, nomes,
organizações, criaturas, eventos ou mitologia.

---

## 54. Próximo passo recomendado

Com o GDD fechado, **não** partir imediatamente para implementar todos os sistemas.

O próximo documento é o **Documento de Produção do Vertical Slice**, muito mais técnico:

mapa completo do primeiro capítulo → layout de cada sala → puzzles → posicionamento de inimigos
→ itens → armas → diálogos → eventos de horror → IA → checkpoints → Blueprints/C++ necessários →
assets → animações → sons → VFX → estrutura de pastas.

É o que transforma BLACK VEIL de ideia em algo construível na UE5.

---

## 📎 Pendência aberta — mapeamento para o Sandbox Framework

Registrada em [[00_BlackVeil_Conceito]] §11 e ainda **não resolvida**.

O Sandbox Framework existente foi construído em torno de automação industrial, redes elétricas,
veículos, espaçonaves, mechas e ecologia planetária. BLACK VEIL pede espaço fechado, recursos
escassos e protagonista sem poder.

Sobreposição aproveitável: inventário com peso e escolha · interação e puzzles · persistência de
estado por GUID · IA com StateTree e percepção · sistema de trauma/biomedicina · save com
integridade · UI/HUD · atmosfera e iluminação.

**Decisão pendente:** BLACK VEIL usa um subconjunto do framework (desativando o resto via
`USBSandboxFeatureSubsystem`), ou o framework se reorienta para survival horror?

O `HorrorDirector`, o `MemorySystem` e o `ExposureSystem` são sistemas **novos** — não existem
no framework atual em nenhuma forma.
