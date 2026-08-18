# Sandbox Framework Plugin Specification (SFPS v1.0.0)

Este documento consolidado define as especificações estritas de arquitetura de software para a governança e implementação do **Sandbox Framework v1.0.0**.

---

## 1. Topologia Final de Plugins (11 Plugins)

A distribuição de módulos é estruturada de forma modular, com dependências unidirecionais previsíveis:

```
Plugins/
├── Foundation/
│   ├── 01_SandboxCommon
│   ├── 02_SandboxInterfaces
│   ├── 03_SandboxAssets
│   └── 04_SandboxCore
├── Gameplay Base/
│   └── 05_SandboxCharacter (Serviço de base para extensões de gameplay: Character, Movement, Camera, Animation)
├── Gameplay Extensions/
│   ├── 06_SandboxCombat (Depende de 05_SandboxCharacter - Habilidades, Armas, Combate)
│   ├── 07_SandboxInteraction (Depende de 05_SandboxCharacter - Portas, Pickups, Objetos interativos)
│   └── 08_SandboxInventory (Depende de 05_SandboxCharacter - Itens, Equipamentos, Inventário)
├── Presentation/
│   ├── 09_SandboxUI
│   └── 10_SandboxDebug
└── Tools/
    └── 11_SandboxEditor
```

### Regra de Dependência Unidirecional de Gameplay
Os plugins Gameplay Extensions (06, 07, 08) podem depender diretamente de Gameplay Base (05), mas o inverso é estritamente proibido (05_SandboxCharacter nunca deve importar tipos ou cabeçalhos de Combat, Inventory ou Interaction). Isso preserva o desacoplamento e evita dependências circulares.
```

### Metadados de Compatibilidade (.uplugin)
Todo descritor `.uplugin` do framework deve conter a estrutura de compatibilidade de versões:
```json
"SandboxVersion": {
    "Plugin": "1.0.0",
    "API": 1,
    "Assets": 1,
    "Network": 1,
    "Serialization": 1
}
```

---

## 2. Abstrações e Modelos Genéricos Reutilizáveis

### A. Estrutura de Contexto Unificada (`FSBBehaviorContext`)
Para evitar quebras de assinatura em cascata caso novas dependências de infraestrutura sejam adicionadas no futuro, todos os comportamentos de gameplay utilizam um wrapper unificado de referências constantes:

```cpp
USTRUCT(BlueprintType)
struct FSBGameplayContext
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<ACharacter> Character = nullptr;

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<APawn> Pawn = nullptr;

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<AController> Controller = nullptr;

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<APlayerState> PlayerState = nullptr;

    UPROPERTY(BlueprintReadOnly)
    float DeltaSeconds = 0.0f;
};

USTRUCT(BlueprintType)
struct FSBFrameworkContext
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<UWorld> World = nullptr;

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<UGameInstance> GameInstance = nullptr;

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<USBAssetManager> AssetManager = nullptr;

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<USBEventSubsystem> EventSubsystem = nullptr;

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<USBSaveSubsystem> SaveSubsystem = nullptr;

    UPROPERTY(BlueprintReadOnly)
    uint8 NetMode = 0;
};

USTRUCT(BlueprintType)
struct FSBBehaviorContext
{
    GENERATED_BODY()

    // Referência forte ao contexto de gameplay do ator
    const FSBGameplayContext* GameplayContext = nullptr;

    // Referência forte ao contexto de infraestrutura global
    const FSBFrameworkContext* FrameworkContext = nullptr;

    // Ponteiro genérico opcional para dados específicos de domínio (ex: FSBMovementContext)
    const void* FeatureContext = nullptr;
};
```

### B. Divisão de Estados de Behaviors
1. **Definition**: Propriedades imutáveis carregadas via Data Asset (ex: `USBMovementBehaviorDefinition`).
2. **Instance**: O UObject de lógica operacional ativado em runtime (ex: `USBMovementBehavior`). É de posse direta do `BehaviorRegistry`.
3. **Runtime Data**: Struct com variáveis dinâmicas e mutáveis que sofrem alteração a cada tick (ex: `FSBMovementRuntimeData`). O behavior apenas referencia este dado transiente.

### C. Priorização e Políticas do Modificador
Toda modificação física ou de atributos é regulada pela estrutura `FSBModifierEntry`:
```cpp
USTRUCT(BlueprintType)
struct FSBModifierEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag SourceTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Value = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESBModifierOperation Operation = ESBModifierOperation::Additive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority = 0; // Prioridade maior executa depois (ex: Overrides)
};
```

---

## 3. Gameplay Message Router & UI Event Bus

O barramento de eventos processa mensagens em duas categorias de tempo de vida:

1. **Instant Message (`BroadcastMessage<T>`)**: Mensagens transientes distribuídas imediatamente aos escutas ativos no momento do disparo (ex: `Event.Weapon.Fire`).
2. **Stateful Message (`PublishState<T>`)**: Registra o último valor enviado (ex: `State.Character.Sprinting`). 
   - Novos componentes que entram em jogo (Late Join / Respawn) leem o estado imediatamente.
   - **Gerenciamento de Cache**: O cache de estado é vinculado ao ciclo de vida do ator emissor. Quando um ator é destruído ou resetado (`ISBResettable`), suas mensagens de estado registradas no barramento devem ser invalidadas.

### UI Event Bus (09_SandboxUI)
O sistema de interface estende o barramento para dar suporte a atualizações assíncronas e reativas baseadas em delegates dinâmicos Blueprint:
- **Inscrições Idempotentes**: O barramento valida se um delegate já está inscrito para uma tag específica antes de adicioná-lo, prevenindo duplicações.
- **Filtro Anti-Spill em Split-Screen**: Todo widget visual deve validar o Pawn de destino do evento contra `GetOwningPlayerPawn()` do seu controle local. Eventos para Pawns pertencentes a outras viewports são descartados síncronamente na entrada do handler do widget.
- **Gerenciamento de Ciclo de Vida**: No `NativeDestruct()` dos widgets herdados de `USBUserWidget`, todas as inscrições ativas associadas à instância do widget são removidas síncronamente via delegate específico, evitando referências pendentes na memória ou chamadas pós-destruição (*Use-After-Free*).
- **Interpolação Cosmética Local**: Mudanças de cooldown devem ser calculadas e decrescidas localmente em `NativeTick` a partir de uma mensagem inicial única de início (`CooldownStarted`), eliminando a necessidade de polling de rede a cada frame.

---

## 4. Matriz Consolidada de Interfaces de Fundação (02_SandboxInterfaces)

Todas as classes e componentes devem se adequar aos seguintes contratos:

- **`ISBInitializable`**: Métodos para inicialização e encerramento ordenados de dependências (`Initialize`, `Shutdown`).
- **`ISBTickable`**: Padroniza ticks em classes que não herdam de `UActorComponent`. Executa por padrão no tick group `TG_PrePhysics` para assegurar que modificações lógicas influenciem a simulação física do mesmo frame.
- **`ISBComponentInterface`**: Define os ganchos do ciclo de vida modular (`OnComponentCreated`, `OnPreInitialize`, `OnInitialize`, `OnPostInitialize`, `OnReady`, `OnShutdown`).
- **`ISBSaveInterface`**: Ganchos de persistência e salvamento de estado do componente.
- **`ISBResettable`**: Implementa `ResetState()` utilizado para limpar estados físicos do personagem durante pooling de atores ou respawn.
- **`ISBReplicable`**: Métodos para empacotamento de propriedades dinâmicas e sincronização de dados transitórios via rede (RPC/Serialization wrappers).
- **`ISBDebugInterface`**: Retorno de strings estruturadas de telemetria e depuração visual.
