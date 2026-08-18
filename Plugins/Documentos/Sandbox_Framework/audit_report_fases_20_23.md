# Relatório de Auditoria de Código - Fases 20 a 23

Este documento apresenta a análise de auditoria linha a linha realizada nas implementações das **Fases 20 a 23** do Sandbox Framework. Identificamos lacunas de segurança, gargalos de performance e inconsistências lógicas sutis de rede que demandam atenção ou refinamentos futuros.

---

## 🔒 1. Fase 20: RPC Rate-Limiting & Security

### 🔍 Código Auditado
*   [`SBRPCRateLimiter.h`](file:///D:/Unreal/GameAnimationSample/Plugins/04_SandboxCore/Source/SandboxCore/Public/Subsystems/SBRPCRateLimiter.h)
*   Integração nos componentes `SBAbilityComponent`, `SBMovementComponent`, `SBCombatComponent` e `SBInteractionComponent`.

### ⚡ Achados e Observações

#### A. Limitação e Resetação no Primeiro Frame do Jogo
*   **Linha 14**: `float LastCallTime = 0.0f;`
*   **Linha 35**: `if (CurrentTime - LastCallTime >= 1.0f)`
*   **Comportamento**: No frame `T=0.0f` do jogo, a janela não é resetada porque `0.0f - 0.0f < 1.0f`. O limitador inicia incrementando a partir de 0 normalmente. Isso não impede o funcionamento, mas significa que a primeira janela tem duração marginalmente menor. É um comportamento aceitável.

#### B. Ausência de Wrappers de RPC no Componente de Inventário
*   **Análise**: O [`SBInventoryComponent`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Public/Components/SBInventoryComponent.h) expõe apenas funções `BlueprintAuthorityOnly` (executadas apenas no Servidor). Ele **não possui** Server RPCs nativos (ex: `ServerRequestEquipItem` ou `ServerRequestDropItem`) expostos para receber requisições de interfaces visuais executadas no Cliente.
*   **Impacto**: Embora os testes passem porque rodam em ambiente autoritativo local, na prática, quando o designer montar uma UI de inventário (onde o jogador clica para Equipar/Descartar), ele precisará criar Server RPCs customizados no PlayerController para envelopar e chamar as funções do componente.
*   **Recomendação**: Adicionar no backlog a criação de Server RPCs nativos rate-limited diretamente no `USBInventoryComponent` para equipar, desequipar e descartar itens, mantendo o padrão de encapsulamento do framework.

---

## ⚡ 2. Fase 21: Compensação de Lag (Lag Compensation)

### 🔍 Código Auditado
*   [`SBLagCompensationSubsystem.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/04_SandboxCore/Source/SandboxCore/Private/Subsystems/SBLagCompensationSubsystem.cpp)

### ⚡ Achados e Observações

#### A. Gargalo de Performance: TActorIterator no Tick (Grave em Escala)
*   **Linhas 47-53**:
    ```cpp
    for (TActorIterator<ACharacter> It(World); It; ++It)
    ```
*   **Comportamento**: A cada tick do motor, o subsistema realiza uma varredura completa de todos os atores do nível buscando por personagens para gravar suas posições.
*   **Impacto**: Em mapas grandes (Open World) ou com grande quantidade de NPCs, a varredura linear por `TActorIterator` gera um consumo de CPU insustentável.
*   **Recomendação**: Substituir a busca linear por um modelo de **Registro Dinâmico**. Personagens devem se registrar no subsistema no seu `BeginPlay` (e se desregistrar no `EndPlay`), permitindo ao subsistema manter uma lista enxuta e rápida (`TSet<TWeakObjectPtr<ACharacter>>`), eliminando o uso do iterador de atores.

#### B. Gargalo de Performance: Rebobinamento de Todos os Atores
*   **Linha 72**: A função `RewindPositions` rebobina a posição de **todos** os personagens cadastrados no histórico da partida.
*   **Impacto**: Se 64 jogadores estiverem no mapa, todos os 64 serão teleportados fisicamente no servidor para validar o hitscan de um único tiro de um único jogador.
*   **Recomendação**: Filtrar os atores que precisam ser rebobinados com base em proximidade espacial (ex: rebobinar apenas atores dentro de uma caixa delimitadora em torno do traço do disparo), reduzindo custos desnecessários de física e colisões na Unreal.

---

## 🧪 3. Fase 22: Status Effects

### 🔍 Código Auditado
*   [`SBStatusEffectComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Components/SBStatusEffectComponent.cpp)

### ⚡ Achados e Observações

#### A. Bug de Drift Temporal e Perda de Ticks sob Server Lag (Clássico de MMO)
*   **Linha 214-225**:
    ```cpp
    if (CurrentTime - Entry.LastPeriodTriggerTime >= Entry.Period)
    {
        // ... aplica alteração ...
        Entry.LastPeriodTriggerTime = CurrentTime; // <-- BUG DE DRIFT
        ActiveEffects.MarkItemDirty(Entry);
    }
    ```
*   **Comportamento**: Se o período de ticks do veneno for de `1.0s` e o servidor sofrer lag (gerando um frame de tick de `1.2s` de intervalo), `LastPeriodTriggerTime` é atualizado diretamente para `CurrentTime`.
*   **Impacto**: O acumulado de atrasos de frames empurra a próxima ativação para frente, gerando um desvio temporal. Em efeitos de longa duração, o jogador pode perder um ou dois ticks de dano por completo devido a esse drift.
*   **Recomendação**: Atualizar a variável adicionando incrementalmente o período de tick em vez de usar o tempo absoluto corrente:
    ```cpp
    Entry.LastPeriodTriggerTime += Entry.Period;
    ```
    Isso preserva o ritmo exato das ativações ao longo do tempo de vida do efeito, independentemente de oscilações na taxa de frames do servidor.

---

## 🎭 4. Fase 23: Sincronização Estética de Equipamento

### 🔍 Código Auditado
*   [`SBCombatComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Components/SBCombatComponent.cpp)

### ⚡ Achados e Observações

#### A. Vulnerabilidade de Replicação de Atores Visuais de Armas
*   **Linhas 468-474**:
    ```cpp
    AActor* NewWeaponActor = GetWorld()->SpawnActor<AActor>(DefAsset->WeaponActorClass, ...);
    ```
*   **Comportamento**: O ator visual da arma é instanciado no servidor. A replicação do ator depende de que a flag `bReplicates` esteja ativada na classe do asset configurada pelo designer (`DefAsset->WeaponActorClass`).
*   **Impacto**: Se o designer esquecer de ativar "Replicates" no Blueprint da arma visual, o ator não será spawnado nem anexado nos clientes remotos (o jogador parecerá desarmado na tela dos outros).
*   **Recomendação**: Adicionar uma salvaguarda em C++ forçando a replicação logo após o spawn para blindar a lógica contra erros humanos de configuração do Data Asset:
    ```cpp
    if (NewWeaponActor)
    {
        NewWeaponActor->SetReplicates(true);
        // ...
    }
    ```

---

## 📊 Matriz de Gravidade dos Achados

| Achado | Componente | Descrição | Gravidade | Esforço de Correção | Status (2026-08-18) |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Gargalo no Tick** | `USBLagCompensationSubsystem` | Uso de `TActorIterator` a cada frame para histórico | **Média-Alta** (Performance) | Baixo | ⏳ Pendente |
| **Drift de Ticks** | `USBStatusEffectComponent` | Perda de ticks sob lag/baixo FPS (LastPeriodTriggerTime) | **Média** (Consistência) | Mínimo | ✅ **Corrigido** — código usa `+= Entry.Period` (linha 376) |
| **Vulnerabilidade Visual**| `USBCombatComponent` | Falha visual se Blueprint do ator não estiver replicado | **Baixa-Média** (Robustez) | Mínimo | ⏳ Pendente |
| **Falta de RPCs** | `USBInventoryComponent` | Sem RPCs de equipar/soltar no cliente | **Baixa** (Integração) | Médio | ⏳ Pendente |
