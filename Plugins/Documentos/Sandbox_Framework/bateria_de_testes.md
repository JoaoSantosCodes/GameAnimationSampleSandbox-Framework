# Bateria de Testes Automatizados - Sandbox Framework

Este documento descreve detalhadamente todos os testes automatizados da suíte de controle de qualidade do Sandbox Framework. Ele fornece mapeamentos dos cenários, propósitos específicos, parâmetros de simulação e os comandos exatos de execução.

---

## 📈 Resumo da Suíte
*   **Total de Testes Atuais**: 88 Specs.
*   **Status atual**: **100% Verde (Passando)**.
*   **Framework de Teste**: Unreal Engine Automation Spec Framework (C++).

---

## 🔬 Detalhamento dos Cenários de Teste

### 1. Sistema de Inventário (`Sandbox.Inventory`)
*   **Cenário 1: Manipulação Básica de Itens**:
    *   *Objetivo*: Validar adição de itens (empilháveis e não-empilháveis), limites de capacidade máxima e rejeição sob mochila cheia.
*   **Cenário 2: Persistência (Save/Load)**:
    *   *Objetivo*: Certificar que a serialização do inventário no disco (`ISBSaveInterface`) preserva slots, tipos de itens e quantidades de empilhamento.
*   **Cenário 3: Integração de Equipar / Fragments**:
    *   *Objetivo*: Testar a concessão automática de comportamentos e atributos no momento em que um item equipado é inserido na hotbar.
*   **Cenário 4: Prevenção de Condição de Corrida (Loot Dispute)**:
    *   *Objetivo*: Validar que duas requisições simultâneas de coletas do mesmo baú no mesmo frame de rede não resultem em duplicação de itens.
*   **Cenário 5: Desequipamento e Ejeção Simétrica**:
    *   *Objetivo*: Validar a limpeza correta e sincronizada de tags de estado e comportamentos quando um item é desequipado.

---

### 2. Sistema de Combate e Habilidades (`Sandbox.Combat`)
*   **Cenário 1: Predição e Confirmação Jitter-Free do Consumo de Munição**:
    *   *Objetivo*: Validar que o cliente consome munição localmente por predição e reconcilia sem travamentos físicos (*jitter-free*) após confirmação do servidor.
*   **Cenário 2: Rejeição de Disparo por Falta de Munição e Rollback (Anti-Cheat)**:
    *   *Objetivo*: Validar que requisições de disparo sem munição suficiente no servidor são abortadas e forçam o cliente a reverter seu estado.
*   **Cenário 3: Swap de Armas via ExclusivityGroup**:
    *   *Objetivo*: Certificar que equipar uma arma ejeta automaticamente qualquer outra arma ativa do mesmo grupo de exclusividade.

---

### 3. Sistema de Compensação de Lag (`Sandbox.LagCompensation`)
*   **Cenário 1: Rebobinamento Físico e Interpolação**:
    *   *Objetivo*: Simular latência de ping artificial e confirmar que a colisão de trace hitscan calcula corretamente a posição retroativa interpolada e restaura o presente síncrono.

---

### 4. Sistema de Status Effects (`Sandbox.StatusEffects`)
*   **Cenário 1: Buffs Permanentes**:
    *   *Objetivo*: Aplicar e testar modificadores aditivos (+100 de velocidade) e tags de estado (invulnerabilidade).
*   **Cenário 2: Expiração de Debuffs**:
    *   *Objetivo*: Testar a limpeza automática de debuffs lentos após a expiração da duração configurada.
*   **Cenário 3: Danos Periódicos (DOTs)**:
    *   *Objetivo*: Validar ticks periódicos de envenenamento retirando vida a cada segundo de forma autoritativa.

---

### 5. Sincronização Estética de Equipamento (`Sandbox.Combat.Visuals`)
*   **Cenário 1: Ciclo de Sockets**:
    *   *Objetivo*: Validar spawn inicial no coldre (`spine_03Socket`), saque para a mão (`hand_rSocket`) ao iniciar o disparo e retorno ao coldre ao parar.

---

### 6. Sistema de UI & Eventos (`Sandbox.UI`)
*   **Cenário 1: Idempotência de Assinaturas**:
    *   *Objetivo*: Certificar que assinaturas duplicadas de delegates de HUD são limpas e não causam vazamentos de memória ou use-after-free.
*   **Cenário 2: Filtro Anti-Spill em Split-Screen**:
    *   *Objetivo*: Garantir que eventos de vida e cooldown são despachados estritamente para o HUD do jogador que disparou a ação (isolamento de viewports).

---

## 🔒 Cenários de Anti-Cheat (Fase 24) (Concluídos)

### 7. Anti-Cheat de Movimentação e Combate (`Sandbox.AntiCheat`)
*   **Cenário 1: Detecção de Speedhack**:
    *   *Objetivo*: Forçar o deslocamento do cliente a uma velocidade 10x superior à máxima permitida e certificar que o servidor detecta a anomalia e teleporta o personagem de volta.
*   **Cenário 2: Detecção de Warp/Teleporte**:
    *   *Objetivo*: Deslocar instantaneamente o ator cliente por 5000 unidades e validar o rollback de transform punitivo no servidor.
*   **Cenário 3: Bloqueio de Dano Através de Parede**:
    *   *Objetivo*: Criar uma barreira estática (obstáculo físico) entre o atacante e o alvo, executar disparo hitscan e verificar a rejeição autoritativa do dano.
*   **Cenário 4: Autorização de Realocação**:
    *   *Objetivo*: Teleportar o personagem e sinalizar a autorização pelo servidor, certificando que o detector de velocidade/warp aceita a nova localização sem aplicar rollback.
*   **Cenário 5: Velocidade Combinada (Sprint + Buff de Status)**:
    *   *Objetivo*: Ativar o comportamento de Sprint (multiplicador 1.5x) e aplicar um buff de velocidade de Status Effect (+100.f) simultaneamente, certificando que o limite dinâmico de velocidade do anti-cheat é a combinação proporcional de ambos (1050.f) e não há rollbacks falsos-positivos.

---

## ⚡ Otimização de Replicação de Atributos (Fase 25) (Concluídos)

### 8. Canais Replicados Públicos e Privados (`Sandbox.Attributes.ConditionalReplication`)
*   **Cenário 1: Classificação Pública/Privada**:
    *   *Objetivo*: Validar se tags contendo `Mana`, `Stamina` ou `Ammo` são marcadas de forma determinística como privadas, e `Health` como pública.
*   **Cenário 2: Roteamento de Canais**:
    *   *Objetivo*: Certificar que o registro de um atributo público o adiciona apenas no array de replicação `PublicAttributes` (replicado para todos) e o privado no `PrivateAttributes` (limitado a `COND_OwnerOnly`).

---

## ⚡ Novas Mecânicas de Gameplay e Combate (Fases 27 a 33) (Concluídos)

### 9. Sistema de Estamina (`Sandbox.Stamina`)
*   **Cenário 1: Consumo e Travas de Pulo**:
    *   *Objetivo*: Validar consumo e bloqueio do pulo se a estamina for menor que o custo.
*   **Cenário 2: Consumo e Regeneração de Sprint**:
    *   *Objetivo*: Testar consumo ao sprintar e regeneração correta de estamina após um delay.
*   **Cenário 3: Estado Exausto (`Exhausted`)**:
    *   *Objetivo*: Testar a ativação da tag `State.Character.Exhausted` ao atingir 0 e a liberação de ações apenas após atingir o limite seguro de recuperação.

### 10. Munição & Recarga (`Sandbox.Reloading`)
*   **Cenário 1: Consumo de Balas e Travamento**:
    *   *Objetivo*: Validar dedução de munição por disparo e travamento automático com 0 balas.
*   **Cenário 2: Recarga e Bloqueio síncrono**:
    *   *Objetivo*: Certificar que o disparo de armas é bloqueado enquanto a tag `State.Weapon.Reloading` estiver ativa.

### 11. Controle e Estados de IA (`Sandbox.AIBehavior`)
*   **Cenário 1: Tabela de Agro**:
    *   *Objetivo*: Validar o acúmulo de agro, mudança para o alvo de maior agro e limpeza automática de referências fracas de Pawns destruídos.
*   **Cenário 2: Desabilitação de Movimento (Crowd Control)**:
    *   *Objetivo*: Certificar que a velocidade de locomoção cai a `0.0f` sob tags `State.Character.Stunned` e `State.Character.Frozen`.
*   **Cenário 3: Bloqueio Lógico de Ataques**:
    *   *Objetivo*: Validar que as habilidades de armas bloqueiam sua entrada na pilha ativa quando o Pawn estiver atordoado.

### 12. Sensibilidade e Dano Refinado (`Sandbox.CriticalDamage`)
*   **Cenário 1: Dano Crítico por Osso**:
    *   *Objetivo*: Validar a detecção de ossos específicos (head, neck_01) e aplicação do multiplicador crítico.
*   **Cenário 2: Resistências e Defesa**:
    *   *Objetivo*: Certificar que o atributo de defesa atenua danos através da fórmula diminishing returns.
*   **Cenário 3: Hit Reactions**:
    *   *Objetivo*: Validar a injeção síncrona da tag `State.Character.HitReacting` ao sofrer danos.

### 13. Sorteios e Drops Físicos (`Sandbox.LootDrop`)
*   **Cenário 1: Sorteio Dinâmico (Loot Tables)**:
    *   *Objetivo*: Validar sorteios síncronos de itens e stacks baseados em peso e raridades configuradas.
*   **Cenário 2: Drop Físico Replicado**:
    *   *Objetivo*: Certificar inicialização, prompts limpos de UI, validação de inventário do interator e travas de concorrência anti-race condition.

### 14. Sistema de Progressão e Experiência (`Sandbox.Experience`)
*   **Cenário 1: Inicialização e Ganho de XP**:
    *   *Objetivo*: Validar se o componente inicializa corretamente no nível 1 com 0 de XP, e se a adição de XP atualiza os pontos acumulados e dispara o evento correspondente.
*   **Cenário 2: Mecânica de Level Up Simples**:
    *   *Objetivo*: Testar a transição de nível ao cruzar o limite requerido e o cálculo de carry-over (XP restante).
*   **Cenário 3: Múltiplos Level Ups em Cadeia**:
    *   *Objetivo*: Simular ganho massivo de XP de uma só vez e confirmar que múltiplos level ups consecutivos ocorrem recursivamente, deduzindo os limites respectivos e mantendo o carry-over preciso.
*   **Cenário 4: Resolução de DataTable**:
    *   *Objetivo*: Validar a priorização de busca de limites de XP em DataTables configuradas pelo designer, com fallback automático para a fórmula matemática exponencial.

### 15. Bancada Física de Crafting (`Sandbox.CraftingStation`)
*   **Cenário 1: Configuração e Interface**:
    *   *Objetivo*: Certificar que a bancada inicializa corretamente no mundo, associando a `StationTag` correspondente para identificação.
*   **Cenário 2: Associação Dinâmica de Tags na Interação**:
    *   *Objetivo*: Validar que a interação síncrona com a bancada concede a `StationTag` ao `USBStateComponent` do jogador.
*   **Cenário 3: Desassociação Manual**:
    *   *Objetivo*: Testar a remoção limpa da tag do jogador e desregistro da lista de interatores ao invocar `StopInteracting`.
*   **Cenário 4: Monitoramento Dinâmico de Proximidade (Range Check)**:
    *   *Objetivo*: Confirmar que afastar o jogador no mundo além do raio `MaxInteractionDistance` limpa automaticamente sua tag de estado no próximo Tick do servidor.

### 16. Desmantelamento de Equipamentos (`Sandbox.Salvage`)
*   **Cenário 1: Rejeição de Itens Não-Salvageable**:
    *   *Objetivo*: Certificar que tentativas de desmantelar itens que não possuem o fragmento `USBItemFragment_Salvageable` retornem falha imediatamente no servidor.
*   **Cenário 2: Consumo Seguro e Drops Garantidos**:
    *   *Objetivo*: Validar que ao desmantelar um item válido, a quantidade consumida é removida do inventário e os subprodutos garantidos com 100% de probabilidade são gerados na quantidade correta.
*   **Cenário 3: Multi-Salvage de Pilhas Sequenciais**:
    *   *Objetivo*: Validar que ao desmantelar uma pilha com múltiplas unidades simultaneamente, o consumo é total e as quantidades resultantes de drop escalam proporcionalmente à quantidade desmantelada.

### 17. Saturação de Cosméticos e Áudio (`Sandbox.Cosmetics.Limiter`)
*   **Cenário 1: Reprodução Inicial Autorizada**:
    *   *Objetivo*: Certificar que a primeira chamada de reprodução de áudio ou efeito visual em um local é imediatamente permitida.
*   **Cenário 2: Supressão por Cooldown Espacial (Grid 3D)**:
    *   *Objetivo*: Validar que chamadas consecutivas idênticas na mesma célula espacial de 1m dentro do intervalo `MinInterval` sejam rejeitadas.
*   **Cenário 3: Independência de Células Espaciais**:
    *   *Objetivo*: Garantir que chamadas idênticas em localizações distantes e no mesmo frame sejam ambas aceitas.
*   **Cenário 4: Exceção de Tipo de Asset**:
    *   *Objetivo*: Confirmar que tocar efeitos/sons diferentes no mesmo local e no mesmo frame seja permitido sem supressão.
*   **Cenário 5: Expiração de Cooldown**:
    *   *Objetivo*: Validar que após a expiração do intervalo configurado, o mesmo efeito volte a ser aceito na mesma posição.

### 18. Segurança de Save Game (`Sandbox.SaveSystem.Security`)
*   **Cenário 1: Salvamento/Carregamento Normal**:
    *   *Objetivo*: Certificar que salvamentos e carregamentos normais do subsistema criptografado e assinado funcionam perfeitamente sem erros.
*   **Cenário 2: Bloqueio de Payload Adulterado**:
    *   *Objetivo*: Validar que qualquer modificação forjada nos bytes criptografados do save game seja detectada e resulte na recusa de carregamento.
*   **Cenário 3: Bloqueio de Assinatura Inválida**:
    *   *Objetivo*: Garantir que modificar apenas a assinatura de validação do arquivo de save resulte na rejeição síncrona do carregamento (Anti-Save Scumming).

### 19. Efeitos Físicos de Superfície e Áudio Ambiental (`Sandbox.Audio.SurfaceAndAmbient`)
*   **Cenário 1: Resolução de Superfície e Mapeamento**:
    *   *Objetivo*: Confirmar que o `USBSurfaceEffectsDataAsset` retorna os sons corretos mapeados para superfícies físicas específicas (ex: grama).
*   **Cenário 2: Fallback de Superfície Padrão**:
    *   *Objetivo*: Garantir que consultas por materiais físicos não explicitamente configurados caiam corretamente no fallback configurado em `SurfaceType_Default`.
*   **Cenário 3: Transições de Zonas de Áudio Ambiental**:
    *   *Objetivo*: Simular a entrada e saída do jogador local no trigger de áudio `ASBAmbientZoneTrigger` e verificar que a alocação e ciclo de overlaps é tratada sem erros.
### 20. Sistema de Missões e Comércio (`Sandbox.QuestsAndMerchant`)
*   **Cenário 1: Progressão por Event Bus**:
    *   *Objetivo*: Validar aceitação, escuta síncrona do barramento de eventos (ex: passos) e progressão automatizada de objetivos até a conclusão.
*   **Cenário 2: Recompensas Desacopladas**:
    *   *Objetivo*: Certificar que reivindicar recompensas adiciona XP e publica eventos para a extensão de inventário conceder itens de forma segura.
*   **Cenário 3: Compras e Vendas com Segurança**:
    *   *Objetivo*: Validar a dedução de saldo de moedas (`Attribute.Coins`), verificação de proximidade física e restrições de inventário no servidor.

---

## 💻 Comando Executável dos Testes (Automation Run)

Para executar a bateria inteira em modo silencioso diretamente do terminal powershell:
```powershell
& "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "D:\Unreal\GameAnimationSample\GameAnimationSample.uproject" -NullRHI -NoSound -NoSplash -stdout -ExecCmds="Automation RunTest Sandbox; Quit" -log
```
O console deverá retornar **`EXIT CODE: 0`** para confirmar estabilidade absoluta do framework (88 de 88 specs verdes).
