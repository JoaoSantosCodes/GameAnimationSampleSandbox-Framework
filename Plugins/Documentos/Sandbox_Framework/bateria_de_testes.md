# Bateria de Testes Automatizados - Sandbox Framework

Este documento descreve detalhadamente todos os testes automatizados da suíte de controle de qualidade do Sandbox Framework. Ele fornece mapeamentos dos cenários, propósitos específicos, parâmetros de simulação e os comandos exatos de execução.

---

## 📈 Resumo da Suíte
*   **Total de Testes Atuais**: 44 Specs.
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

## 💻 Comando Executável dos Testes (Automation Run)

Para executar a bateria inteira em modo silencioso diretamente do terminal powershell:
```powershell
& "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "D:\Unreal\GameAnimationSample\GameAnimationSample.uproject" -NullRHI -NoSound -NoSplash -stdout -ExecCmds="Automation RunTest Sandbox; Quit" -log
```
O console deverá retornar **`EXIT CODE: 0`** para confirmar estabilidade absoluta do framework.
