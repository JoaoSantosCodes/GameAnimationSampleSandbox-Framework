# Plano de Implementação - Marco 6: Inteligência Artificial e Combate Avançado

Este plano descreve o design, a arquitetura e as etapas de implementação para o **Marco 6** do Sandbox Framework, introduzindo Inteligência Artificial integrada ao componente de estados, multiplicadores de dano crítico/resistências, e tabelas de loot físico replicadas.

---

## 📐 Especificação das Novas Fases

### Fase 31: Inteligência Artificial Integrada com State Component (`05_SandboxCharacter` & `11_SandboxEditor`)
*   **Objetivo**: Integrar IAs inimigas ao vocabulário de tags de estado e criar comportamento de perseguição síncrona.
*   **Regras de Negócio**:
    *   **Compartilhamento de Tags**: O `USBStateComponent` deve ser acoplável a `AAIController` ou `APawn` inimigo para desabilitar comportamentos (ex: impedir locomoção ou ataque de IA se a tag `State.Character.Stunned` ou `State.Character.Frozen` estiver ativa).
    *   **Agro-Register**: Um subsistema simples de ódio/agro no componente de combate da IA registrando alvos agressores baseado em dano sofrido.
    *   **Perseguição**: Comportamento básico de IA que se move em direção ao alvo de maior agro e ataca quando dentro do alcance físico validado no servidor.
    *   **Testes unitários**: `SBAIBehaviorTests.cpp` cobrindo registro de agro, bloqueio de ataque por tags de controle de grupo (CC) e oclusão de visão.

### Fase 32: Dano Crítico, Resistências e Reações de Impacto Replicadas (`06_SandboxCombat`)
*   **Objetivo**: Implementar sensibilidade a pontos fracos, mitigação de dano por atributos e animações de reação a golpes.
*   **Regras de Negócio**:
    *   **Multiplicadores de Weakspot**: O `USBWeaponBehaviorHitscan` deve verificar o `BoneName` atingido no trace físico (ex: `head` aplica multiplicador `2.0x`, mapeado em Data Asset).
    *   **Resistência a Dano**: O `USBAttributeComponent` do receptor aplica mitigação baseada no atributo `Attribute.Defense` ou tags de resistências específicas (ex: `Attribute.Resistance.Fire`).
    *   **Hit Reactions**: Aplicação síncrona de tags de impacto (`State.Character.HitReacting`) forçando a execução de montagens de animação de impacto de forma predita e replicada no servidor.
    *   **Testes unitários**: `SBCriticalDamageTests.cpp` validando multiplicadores de osso, mitigação de defesa e aplicação de impacto.

### Fase 33: Tabela de Loot e Drop Físico Replicado (`08_SandboxInventory`)
*   **Objetivo**: Criar espólios dinâmicos que dropam fisicamente no cenário a partir de inimigos mortos.
*   **Regras de Negócio**:
    *   **Loot Table**: Criação do Data Asset `USBLootTableDataAsset` mapeando ID de item, chance percentual (raridade) e quantidade mínima/máxima.
    *   **Physical Drop**: Classe `ASBPhysicalLootDrop` herdando de `AActor` com malha dinâmica (`UStaticMeshComponent`), física de colisão ativa (`SimulatePhysics`) e componente interativo `USBInteractionComponent` para coleta.
    *   **Segurança**: O loot físico é spawnado de forma autoritativa no servidor e se torna coletável após tocar o solo para evitar cheats de vácuo.
    *   **Testes unitários**: `SBLootDropTests.cpp` cobrindo probabilidades de drop tables e replicação de coleta física de itens.

---

## 🛠️ Modificações Propostas por Arquivo

### [`SBCombatComponent.h`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Public/Components/SBCombatComponent.h) & [`SBCombatComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Components/SBCombatComponent.cpp)
*   Adicionar gerenciamento de alvos de agro (`AgroTable` mapeando `APawn*` para `float` de ódio acumulado).
*   Adicionar funções `AddAgro(APawn* Instigator, float Damage)` e `GetHighestAgroTarget()`.

### [`USBWeaponBehaviorHitscan.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Weapons/SBWeaponBehaviorHitscan.cpp)
*   Adicionar lógica para mapear e ler bone names atingidos (`HitResult.BoneName`) e aplicar multiplicação de dano baseado em um mapa de ossos configurado no Data Asset do comportamento de arma.

### [`SBAttributeComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Components/SBAttributeComponent.cpp)
*   Na função de aplicação de dano/modificação de vida, deduzir a quantidade com base no valor atual do atributo `Attribute.Defense`.

---

## 🔬 Plano de Verificação

### Testes Automatizados (C++)
*   Novos arquivos de teste:
    1.  [`SBAIBehaviorTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Tests/SBAIBehaviorTests.cpp)
    2.  [`SBCriticalDamageTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Tests/SBCriticalDamageTests.cpp)
    3.  [`SBLootDropTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Private/Tests/SBLootDropTests.cpp)
*   Todos os testes devem passar com sucesso absoluto (**EXIT CODE: 0**).
