# Discursos de Desenvolvimento - Diário de Engenharia e Retrospectiva

Este documento consolida as discussões de design, as decisões de engenharia crítica tomadas durante a jornada de desenvolvimento do **Sandbox Framework** e as lições aprendidas ao caçar e corrigir bugs de arquitetura.

---

## 🏛️ Evolução Arquitetural e Decisões Críticas

O desenvolvimento do Sandbox Framework foi norteado por um objetivo: **Desacoplamento e Segurança em Rede**.

### 1. PawnData e Injeção de Componentes (SBComponentFactory)
*   **Problema**: A maioria dos projetos Unreal instancia componentes de gameplay (como Inventário ou Combate) diretamente no construtor do Character em C++ usando `CreateDefaultSubobject`. Isso gera acoplamento rígido: se decidirmos remover o combate para fazer um minijogo pacífico, a classe do personagem precisa ser re-compilada, e o executável carrega memória inútil.
*   **Solução**: Adotamos o padrão baseado em dados do Lyra. O personagem é apenas uma casca vazia. Suas propriedades físicas e lista de componentes são descritos em um Data Asset `USBPawnData`. O `USBComponentFactory` lê essa definição em tempo de execução e injeta dinamicamente os componentes na inicialização do ator.
*   **Lição**: O ciclo de vida do componente (`ISBComponentInterface`) precisa ter passos precisos para que componentes injetados tardiamente consigam encontrar seus vizinhos de forma determinística em rede.

### 2. Barramento de Eventos Assíncronos (Message Router)
*   **Problema**: A comunicação direta entre sistemas (ex: `Character->GetInventory()->OnItemAdded->CallHUDUpdate()`) cria teias de aranha de dependências. Se o HUD quebrar, o inventário quebra. Se o inventário for removido, o HUD não compila.
*   **Solução**: Implementamos um barramento de eventos desacoplado (`USBEventSubsystem`). Atores publicam payloads genéricos em tags específicas (ex: `Event.Attribute.Changed`). Qualquer sistema interessado assina a tag de forma assíncrona.
*   **Aprimoramento**:
    *   **Idempotência**: Protegemos a assinatura garantindo que o mesmo delegate não possa se inscrever repetidamente na mesma tag, prevenindo chamadas redundantes.
    *   **Auto-Unsubscribe**: Vinculamos a desinscrição recursiva no ciclo de vida de destruição dos widgets (`NativeDestruct`) e atores (`ResetState`/`Shutdown`), eliminando vazamentos de memória e referências pendentes a ponteiros coletados pelo Garbage Collector.

---

## 🐛 Caça aos Bugs Críticos e Lições de Depuração

Durante a homologação física dos plugins, resolvemos bugs complexos que não seriam detectados em testes de compilação simples.

### Case 1: Use-After-Free no Inventário (Fase 18)
*   **Sintoma**: Um crash aleatório de violação de acesso ocorria quando um item era removido do inventário de forma síncrona em rede.
*   **Causa**: O método `ServerRemoveItem()` excluía fisicamente o slot do array (`RemoveAt`) antes de disparar o evento de remoção. O evento, rodando síncronamente, tentava ler o ponteiro do item a partir da referência da struct deletada.
*   **Solução**: Reordenamos o fluxo com segurança:
    1.  Fazemos o cache local do ponteiro do item (`RemovedInstance`).
    2.  Removemos a entrada do array físico de slots (garantindo que qualquer query síncrona subsequente já veja o estado correto do inventário).
    3.  Publicamos o evento usando o ponteiro cacheado (que é mantido vivo no escopo local).

### Case 2: Ensure Crash no Carregamento Autônomo de Testes (Fase 19)
*   **Sintoma**: Testes de inventário salvos quebravam em execuções de console autônomos (`UnrealEditor-Cmd.exe`) com falhas de `Ensure condition failed` ao tentar registrar tags de eventos.
*   **Causa**: Em ambientes de linha de comando puros, as tags de gameplay declaradas em arquivos `.ini` do editor não são carregadas automaticamente no startup do módulo.
*   **Solução**: Adicionamos o registro nativo C++ síncrono das tags de eventos (`Event.Inventory.ItemAdded`, etc.) diretamente na inicialização do módulo (`StartupModule()` de `SandboxInventoryModule.cpp`), garantindo integridade das tags independente do estado do editor.

### Case 3: O Bug do Cooldown Compartilhado (Fase 19)
*   **Sintoma**: Ativar uma habilidade acendia a máscara de cooldown em todos os slots da hotbar ao mesmo tempo.
*   **Causa**: O widget de habilidades escutava `Event.Ability.CooldownStarted` mas não validava qual habilidade havia disparado o evento.
*   **Solução (Fail-Closed)**:
    *   Adicionamos a propriedade C++ exposta `WatchedAbilityTag` por slot.
    *   Adicionamos o guard seguro na assinatura: `if (!WatchedAbilityTag.IsValid() || CoolPayload->AbilityTag != WatchedAbilityTag) return;`.
    *   Adicionamos um aviso explícito de diagnóstico no Output Log (`LogSandboxUI`) alertando o designer caso um slot no editor estivesse mal configurado (sem tag watched).

### Case 4: O Vazamento de Dados (Split-Screen Spill)
*   **Sintoma**: Jogando em tela dividida, a barra de vida do jogador 1 atualizava com o dano sofrido pelo jogador 2.
*   **Causa**: O barramento de eventos envia a notificação para todos os widgets instanciados na máquina local. Sem filtragem, o HUD do jogador 1 lia o payload correspondente ao pawn do jogador 2.
*   **Solução (Anti-Spill)**: Criamos o método helper `GetOwningPlayerPawn()` no `USBUserWidget` que interroga a viewport do jogador local correspondente e compara síncronamente: `if (Payload->TargetPawn != GetOwningPlayerPawn()) return;`.

### Case 5: Exploit de Proximidade e Validação RPC (Fase 20)
*   **Sintoma**: Clientes podiam interagir com baús/itens distantes enviando chamadas RPC diretas (`ServerStartInteract`) sem estarem próximos fisicamente.
*   **Causa**: As validações `ServerStartInteract_Validate` e `ServerCompleteInteract_Validate` retornavam `true` incondicionalmente, deixando a verificação de distância apenas para a implementação da lógica (que apenas cancelava a ação localmente ao invés de barrar e desconectar o cliente suspeito).
*   **Solução**: Implementamos verificações de distância 3D rigorosas e rate-limiting diretamente nas funções `_Validate`. O retorno de `false` nestas funções agora ejeta e desconecta imediatamente o cliente malicioso.

### Case 6: Rate Limiting Throttling e Consumo de Tentativas (Fase 20)
*   **Sintoma**: Testes de rate limit às vezes retornavam um número menor de sucessos do que o esperado.
*   **Causa**: Tentativas que falharam em validações lógicas subsequentes (como alvo nulo ou fora do alcance) ainda assim incrementavam o contador do rate-limiter, consumindo as tentativas permitidas na janela de 1 segundo.
*   **Solução**: Esta é a natureza correta de um rate-limiter ("Attempt Throttling"). Ele deve contar *tentativas de requisição* (inputs de rede) para proteger o servidor contra flooding de processamento. Atualizamos o Cenário 6 dos testes automatizados de interação para validar esse comportamento com precisão.

### Case 7: LWC (Large World Coordinates) e Conflito de Tipagem em Testes (Fase 21)
*   **Sintoma**: A suíte de testes de Lag Compensation falhava ao tentar compilar a asserção `TestEqual` sobre a coordenada X do ator (`GetActorLocation().X`).
*   **Causa**: Na Unreal Engine 5, as coordenadas de mundo usam LWC, o que significa que o componente `X` de `FVector` é do tipo `double` (64 bits). Ao passar a asserção contra um literal `float` (`100.f`), o compilador C++ encontrava ambiguidades na resolução da função sobrecarregada `TestEqual`.
*   **Solução**: Adaptamos as assinaturas dos testes de automação para usar literais de ponto flutuante de precisão dupla (`100.0` e `50.0`) em vez de simples floats (`100.f`), garantindo resolução de sobrecarga determinística.

### Case 8: FTickableGameObject no World Subsystem (Fase 21)
*   **Sintoma**: Compilação de subsistema de mundo falhava ao instanciar `GetStatId()` com `RETURN_QUICK_STAT_ID`.
*   **Causa**: O macro `RETURN_QUICK_STAT_ID` está depreciado ou requer definições extras em escopos locais específicos de subsistema em certas configurações do UBT.
*   **Solução**: Substituímos por `RETURN_QUICK_DECLARE_CYCLE_STAT(USBLagCompensationSubsystem, STATGROUP_Tickables);`, que é a diretiva nativa estável e performática para registros de tick de subsistemas na Unreal Engine 5.

### Case 9: Inicialização de Role de Atores em Testes de Automação (Fase 22)
*   **Sintoma**: Os status effects eram aplicados nos testes unitários, mas nenhuma Gameplay Tag ou Modificador de Atributo era concedido, resultando em falhas nas asserções de verificação.
*   **Causa**: O método `ApplyStatusEffect` possui um guard de autoridade (`if (!GetOwner()->HasAuthority())`). Em mundos de teste criados programaticamente (`EWorldType::Game`), os atores gerados não possuem o papel de autoridade (`ROLE_Authority`) definido por padrão, fazendo com que o componente retornasse silenciosamente.
*   **Solução**: Adicionamos a chamada explícita `TestCharacter->SetRole(ROLE_Authority);` logo após o spawn do personagem no bloco `BeforeEach` dos testes de automação de status effects.

### Case 10: Resolução Sob Demanda de Componentes (Fase 22)
*   **Sintoma**: O componente de status effects acessava ponteiros nulos ao tentar aplicar tags ou modificadores durante os testes unitários.
*   **Causa**: No ciclo de vida normal do Unreal, o método `BeginPlay()` executa cacheamento estático de componentes vizinhos. Em testes unitários em que os componentes são instanciados e registrados via C++ dinâmico em ordem arbitrária, o `BeginPlay()` pode rodar antes que o vizinho esteja devidamente anexado.
*   **Solução**: Implementamos um padrão de carregamento preguiçoso (*lazy loading*) nos métodos de acesso (ex: `if (!CachedStateComponent) { CachedStateComponent = FindComponent(); }`), garantindo resiliência total mesmo sob registro tardio de componentes.

### Case 11: Falha no Root Component ao Anexar Atores Genéricos (Fase 23)
*   **Sintoma**: Nos testes automatizados de visual de armas, as asserções de anexação de coldre e mão falhavam porque a arma retornava sem pai anexado (`GetAttachParent()` retornava nulo).
*   **Causa**: Estávamos configurando a classe de teste como `AActor::StaticClass()`. Na Unreal Engine, a classe base `AActor` é apenas um container lógico e não possui um `RootComponent` por padrão. Chamar `AttachToComponent` em um ator sem root component falha silenciosamente.
*   **Solução**: Mudamos a classe de teste para `AStaticMeshActor::StaticClass()`, que possui nativamente um `UStaticMeshComponent` como root component.

### Case 12: Conflito de Mobilidade (Static vs. Movable) na Anexação (Fase 23)
*   **Sintoma**: Ao usar `AStaticMeshActor::StaticClass()`, os testes lançavam mensagens de aviso críticas no PIE informando que atores estáticos não podem ser anexados a pais com mobilidade móvel (como o esqueleto do personagem), abortando a operação.
*   **Causa**: Um ator estático possui restrições físicas e de renderização otimizadas que impedem que ele se mova com o personagem.
*   **Solução**: Injetamos uma configuração programática de mobilidade (`NewWeaponActor->GetRootComponent()->SetMobility(EComponentMobility::Movable)`) na rotina de spawn no servidor antes de executar a anexação, permitindo a anexação livre de qualquer classe de ator.

### Case 13: O Cooldown da Arma nos Testes Unitários (Fase 24)
*   **Sintoma**: Ao disparar o rifle duas vezes consecutivas nos testes automatizados de anti-cheat de combate, a segunda asserção de disparo falhava e o alvo não tomava dano.
*   **Causa**: O Data Asset de definição da arma possuía `FireRate = 0.2f` (200ms de cooldown). Como os testes executam instruções C++ sequencialmente no mesmo frame (0ms entre chamadas), o servidor rejeitava o segundo disparo por considerar que ele ocorreu antes da expiração do cooldown.
*   **Solução**: Ajustamos a propriedade `FireRate` para `0.0f` especificamente dentro da configuração do Rifle no teste unitário, eliminando o limite de cadência e permitindo disparos frame-perfect.

### Case 14: A Colisão de Visibilidade em Atores de Teste (Fase 24)
*   **Sintoma**: Os disparos de teste de rifle passavam através do alvo sem causar dano, mesmo sem obstáculos físicos intermediários.
*   **Causa**: O trace de hitscan de armas de fogo utiliza o canal de colisão `ECC_Visibility`. Atores `ASBCharacter` spawnados dinamicamente em mundos de testes C++ puros possuem a colisão padrão da cápsula definida como `Pawn`, a qual por padrão ignora/overlap o canal `Visibility` para evitar que a câmera do jogador colida com outros pawn.
*   **Solução**: Adicionamos chamadas explícitas nos testes de combate para forçar o bloqueio do canal `Visibility` na cápsula dos personagens: `Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block)`.

### Case 15: Conflito de Replicação nos Testes Unitários de Atributos Privados (Fase 25)
*   **Sintoma**: Os testes de rede e combate herdados quebravam ao compilar informando que o método `OnRep_ReplicatedAttributes()` não era membro de `USBAttributeComponent`.
*   **Causa**: Com a reformulação de replicação de canais, dividimos o array único `ReplicatedAttributes` em `PublicAttributes` e `PrivateAttributes`, removendo o membro antigo. Contudo, testes automatizados legados faziam chamadas diretas a `OnRep_ReplicatedAttributes()` para simular pacotes de rede do cliente de forma síncrona.
*   **Solução**: Implementamos de volta o método `OnRep_ReplicatedAttributes()` no componente de atributos como um método público de compatibilidade que dispara sequencialmente ambos os novos RepNotifies (`OnRep_PublicAttributes()` e `OnRep_PrivateAttributes()`), assegurando 100% de retrocompatibilidade com a suíte de testes existente.

---

## 💡 Princípios de Engenharia de Destaque

1.  **Fail-Closed por Padrão**: Se um dado está incorreto ou ausente, o sistema deve recusar a execução e alertar explicitamente no log (Output Log estruturado), em vez de prosseguir com fallbacks silenciosos que criam bugs visuais difíceis de rastrear.
2.  **Snapshot & Mutação**: Ao alterar coleções indexadas que disparam eventos síncronos, capture instantâneos (*snapshots*) dos ponteiros antes de aplicar a mutação no array.
3.  **Ticking Inteligente**: Classes de lógica física e inputs devem residir em `TG_PrePhysics` para influenciar a física no mesmo frame; componentes de UI que interpolam valores de cooldown devem fazê-lo estritamente de forma local no client-side para poupar banda de rede.
4.  **Flooding Attempt Restriction**: O rate limiting deve sempre ser o primeiro check em un RPC `_Validate` para mitigar o custo computacional de validações complexas (como traces físicos 3D) sob ataque de spam.
5.  **Unilateral Delay Backtracking**: O tempo de rebobinamento físico no servidor para fins de compensação de lag deve basear-se no atraso de ida simples (One-Way Delay = Ping / 2) mais o atraso de interpolação do cliente para corresponder fielmente à tela do jogador.
6.  **Fail-Safe Time Bounds**: Todo cálculo baseado em tempo retroativo relatado pelo cliente ou estimado pelo servidor deve ser rigidamente limitado no servidor (`PingSeconds` clampado entre `0.0f` e `0.5f`) para prevenir exploits de manipulação do tempo físico de colisão.
7.  **Server-Only Fast Array Status Replication**: Lógicas críticas de status effects (cálculo de ticks periódicos e modificadores aditivos/multiplicativos) devem rodar 100% no servidor. A replicação para os clientes deve ser delegada a estruturas serializadoras eficientes (`FFastArraySerializer`), mitigando vulnerabilidades de manipulação de memória local.
8.  **Programmatic Attachment Mobility Enforcement**: Para garantir resiliência contra erros de configuração artística em editores visuais, o sistema de anexação em esqueleto deve forçar programaticamente a mobilidade do ator anexado para `Movable` antes de vincular transformações físicas.
9.  **Anti-Wall Clipping Verification**: Em mecânicas de disparos hitscan em rede, o servidor deve sempre realizar uma consulta de linha de visão a partir da origem física do corpo do atacante até o ponto de impacto. Isso previne abusos de clipping de câmera onde o cliente consegue ver e atirar através de geometrias estáticas de colisão.
10. **Redundant Server-Side Movement Constraints**: A detecção autoritativa de movimentação ilícita deve basear-se na velocidade máxima dinâmica configurada pela engine adicionando limiares de segurança (jitter e latência de rede), forçando o teleporte imediato (*rollback*) do jogador infrator para sua última localização válida registrada.
11. **Conditional OwnerOnly Bandwidth Optimization**: Atributos e recursos de uso estritamente pessoal (como Mana, Stamina e munição) devem ser replicados utilizando a condição `COND_OwnerOnly` em vez de replicados globalmente. Isso reduz severamente o consumo de banda de rede em zonas povoadas e impede exploits de wallhacks de interface inimiga.
