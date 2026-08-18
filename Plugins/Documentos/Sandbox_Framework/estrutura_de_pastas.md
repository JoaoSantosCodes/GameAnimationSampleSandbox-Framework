# Guia de Estrutura de Pastas - Sandbox Framework

Este documento descreve detalhadamente a organização física das pastas e arquivos dos projetos **V1** e **GameAnimationSample**, servindo como guia de mapeamento para o Manual de Uso.

---

## 📂 1. Raiz do Workspace e Projetos

O Sandbox Framework opera em dois ambientes físicos estruturados da seguinte forma:

### A. Projeto Standalone (`D:\Unreal\V1`)
Projeto C++ limpo utilizado como ambiente primário de homologação e testes automatizados.
```
D:\Unreal\V1/
├── Config/                     # Configurações do motor (Engine, Input, GameplayTags)
├── Content/                    # Assets globais e blueprints do jogo
│   └── SandboxFramework/       # Assets do Framework criados no editor
│       ├── Abilities/          # Blueprints lógicos de habilidades (ex: teleporte)
│       ├── Blueprints/         # Atores de gameplay (BP_SBCharacter_Hero, BP_BauDeTeste, SandboxCharacter_Hero)
│       ├── Data/               # Configurações de Data Assets (DA_PawnData_Hero, DA_CameraMode, DA_MovementConfig)
│       ├── Input/              # Enhanced Input Actions (IA_SB_Sprint, IA_SB_Interact, IA_SB_Crouch) e Contextos (IMC_HeroDefault)
│       └── UI/                 # Widget Blueprints derivados das backing classes C++ (WBP_StatusHUD, etc.)
├── Plugins/                    # Todos os plugins lógicos e de terceiros (ver Seção 2)
├── Source/                     # Módulo principal C++ de inicialização (V1)
│   ├── V1/                     # Cabeçalhos, builds e código C++ do módulo
│   ├── V1.Target.cs            # Alvo de compilação standalone do jogo
│   └── V1Editor.Target.cs      # Alvo de compilação do editor
├── V1.sln                      # Arquivo de solução do Visual Studio
└── V1.uproject                 # Descritor do projeto Unreal
```

### B. Projeto de Animação Integrado (`D:\Unreal\GameAnimationSample`)
Projeto de demonstração de animações convertido para C++ híbrido e integrado com os plugins do Sandbox.
```
D:\Unreal\GameAnimationSample/
├── Config/                     # Configurações de animações, mover e tags
├── Content/                    # Malhas 3D, mapas e animações da Epic Games
│   └── SandboxFramework/       # Assets do Framework integrados no projeto de animação
│       ├── Abilities/          # Blueprints lógicos de habilidades (ex: teleporte)
│       ├── Blueprints/         # Atores de gameplay (BP_SBCharacter_Hero, BP_BauDeTeste, SandboxCharacter_Hero)
│       ├── Data/               # Configurações de Data Assets (DA_PawnData_Hero, DA_CameraMode, DA_MovementConfig)
│       ├── Input/              # Enhanced Input Actions (IA_SB_Sprint, IA_SB_Interact, IA_SB_Crouch) e Contextos (IMC_HeroDefault)
│       └── UI/                 # Widget Blueprints derivados das backing classes C++ (WBP_StatusHUD, etc.)
├── Plugins/                    # Pasta de plugins contendo o Sandbox portado
├── Source/                     # Módulo principal C++ de jogo (GameAnimationSample)
│   ├── GameAnimationSample/    # Cabeçalhos, builds e implementação do módulo
│   ├── GameAnimationSample.Target.cs
│   └── GameAnimationSampleEditor.Target.cs
├── GameAnimationSample.sln     # Solução gerada do Visual Studio
└── GameAnimationSample.uproject
```

---

## 🧱 2. Topologia de Pastas de um Plugin Sandbox (Padrão)

Cada um dos 11 plugins lógicos do Sandbox segue uma arquitetura modular rígida baseada em C++ e conteúdo:
```
[Nome_Do_Plugin]/
├── Content/                    # Assets locais (.uasset, .umap, arquivos INI locais)
├── Resources/                  # Recursos de interface (ícones do plugin para o editor)
├── Source/                     # Raiz dos códigos-fonte C++ do plugin
│   └── [Módulo]/               # Módulo C++ correspondente
│       ├── Private/            # Implementações privadas (.cpp)
│       │   └── Tests/          # Testes de automação locais (.spec.cpp, .cpp)
│       ├── Public/             # Cabeçalhos públicos expostos (.h)
│       └── [Módulo].Build.cs   # Regras de compilação e dependências de módulos
└── [Nome_Do_Plugin].uplugin    # Metadados de compatibilidade e plugins necessários
```

---

## 🗂️ 3. Árvore Detalhada dos 11 Plugins do Sandbox

Os plugins do framework estão localizados em `Plugins/` e dividem-se em 4 camadas de arquitetura.

### Camada 1: Fundação (Base Comum e Regras)
Esta camada é a base de todo o framework. Módulos superiores herdam dela, mas ela nunca depende de módulos de gameplay.

#### 1. [`01_SandboxCommon`](file:///D:/Unreal/V1/Plugins/01_SandboxCommon)
*   **Propósito**: Agregadores comuns de atributos, enums e estruturas genéricas.
```
├── Source/SandboxCommon/
│   ├── Public/
│   │   ├── Types/
│   │   │   └── SBCommonTypes.h   # Declara FSBAttribute, FSBAttributeModifier
│   │   └── Utilities/
│   │       └── SBLogCategories.h # Categorias de log centralizadas (LogSandboxUI, etc.)
│   └── Private/
│       └── Utilities/
│           └── SBLogCategories.cpp
```

#### 2. [`02_SandboxInterfaces`](file:///D:/Unreal/V1/Plugins/02_SandboxInterfaces)
*   **Propósito**: Interfaces C++ puras que viabilizam o desacoplamento de chamadas.
```
├── Source/SandboxInterfaces/
│   └── Public/
│       └── Interfaces/
│           ├── SBComponentInterface.h # Ciclo de vida modular
│           ├── SBSaveInterface.h      # Persistência sob demanda
│           ├── ISBResettable.h        # Cleanup de buffers de atores
│           └── SBDebugInterface.h     # Telemetria para o Gameplay Debugger
```

#### 3. [`03_SandboxAssets`](file:///D:/Unreal/V1/Plugins/03_SandboxAssets)
*   **Propósito**: Mapeamento assíncrono de assets e dados do pawn.
```
├── Source/SandboxAssets/
│   ├── Public/
│   │   ├── SBAssetManager.h      # Gerenciador de carregamento por PrimaryAssetId
│   │   └── USBPawnData.h         # Ficha de especificações e componentes do herói
│   └── Private/
│       ├── SBAssetManager.cpp
│       └── USBPawnData.cpp
```

#### 4. [`04_SandboxCore`](file:///D:/Unreal/V1/Plugins/04_SandboxCore)
*   **Propósito**: Message Router (subsistema de eventos assíncronos) e configurações de inputs.
```
├── Source/SandboxCore/
│   ├── Public/
│   │   ├── Subsystems/
│   │   │   ├── SBEventSubsystem.h   # Barramento de eventos Broadcast/Stateful
│   │   │   └── SBEventPayloads.h    # Classes de dados de eventos (USBAttributeChangedPayload, etc.)
│   │   └── Input/
│   │       ├── SBInputConfig.h      # Mapeia GameplayTags para InputActions
│   │       └── SBInputSubsystem.h
│   └── Private/
│       ├── Subsystems/
│       │   └── SBEventSubsystem.cpp
│       └── Input/
│           └── SBInputSubsystem.cpp
```

---

### Camada 2: Gameplay Base (Locomoção)
Esta camada lida com o controle físico do personagem sob rede.

#### 5. [`05_SandboxCharacter`](file:///D:/Unreal/V1/Plugins/05_SandboxCharacter)
*   **Propósito**: Personagem base modular, câmera dinâmica e locomoção preditiva com rollback.
```
├── Source/SandboxCharacter/
│   ├── Public/
│   │   ├── Character/
│   │   │   └── SBCharacter.h         # Ator herói desacoplado
│   │   ├── Components/
│   │   │   ├── SBComponentFactory.h  # Injetor de componentes via PawnData
│   │   │   ├── SBMovementComponent.h  # Movimento predito na fila PrePhysics
│   │   │   └── SBStateComponent.h     # Central de GameplayTags de estado
│   │   └── Movement/
│   │       └── SBMovementBehavior.h   # Classe abstrata para Sprint/Crouch
│   └── Private/
│       ├── Character/
│       │   └── SBCharacter.cpp
│       ├── Components/
│       │   ├── SBComponentFactory.cpp
│       │   ├── SBMovementComponent.cpp
│       │   └── SBStateComponent.cpp
│       └── Tests/
│           └── SBMovementTests.cpp    # Testes locais de locomoção
```

---

### Camada 3: Gameplay Extensions (Sistemas Opcionais)
Módulos secundários que injetam recursos específicos. Dependem da Camada 2, mas são isolados entre si.

#### 6. [`06_SandboxCombat`](file:///D:/Unreal/V1/Plugins/06_SandboxCombat)
*   **Propósito**: Habilidades genéricas, armas hitscan/projéteis e barra de vida lógica.
```
├── Source/SandboxCombat/
│   ├── Public/
│   │   ├── Components/
│   │   │   └── SBCombatComponent.h   # Ativador de habilidades e armas
│   │   └── Behaviors/
│   │       ├── SBAbility.h           # Classe de habilidades lógicas
│   │       └── SBWeaponBehavior.h    # Classes de disparo hitscan/projétil
│   └── Private/
│       ├── Components/
│       │   └── SBCombatComponent.cpp
│       └── Tests/
│           └── SBCombatTests.cpp     # Testes de combate e projéteis
```

#### 7. [`07_SandboxInteraction`](file:///D:/Unreal/V1/Plugins/07_SandboxInteraction)
*   **Propósito**: Interações físicas com suporte a foco (Trace) e holds preditos.
```
├── Source/SandboxInteraction/
│   ├── Public/
│   │   └── Components/
│   │       └── SBInteractionComponent.h # Ticks limitados a 60Hz síncronos
│   └── Private/
│       ├── Components/
│       │   └── SBInteractionComponent.cpp
│       └── Tests/
│           └── SBInteractionTests.cpp   # Testes de interrupção e foco
```

#### 8. [`08_SandboxInventory`](file:///D:/Unreal/V1/Plugins/08_SandboxInventory)
*   **Propósito**: Mochila de itens replicada, slots de equipamentos e guards de race condition.
```
├── Source/SandboxInventory/
│   ├── Public/
│   │   ├── Components/
│   │   │   └── SBInventoryComponent.h   # Mochila lógica
│   │   └── Items/
│   │       ├── SBItemDefinition.h      # Definição imutável de itens
│   │       └── SBItemInstance.h        # Instância mutável replicada
│   └── Private/
│       ├── Components/
│       │   └── SBInventoryComponent.cpp
│       └── Tests/
│           ├── SBInventoryTests.cpp
│           └── SBInventorySaveTests.cpp # Testes de serialização de inventário
```

---

### Camada 4: Apresentação e Depuração (UI & Debug)
Camada de front-end do jogo.

#### 9. [`09_SandboxUI`](file:///D:/Unreal/V1/Plugins/09_SandboxUI)
*   **Propósito**: Ciclo de vida e camadas de widgets reativos baseados em backing classes.
```
├── Source/SandboxUI/
│   ├── Public/
│   │   ├── HUD/
│   │   │   └── SBHUD.h                # HUD padrão que instancia a tela do viewport
│   │   ├── Subsystems/
│   │   │   └── SBUIManager.h          # Gerenciador de camadas (HUD, Menus, Popups)
│   │   └── Widgets/
│   │       ├── SBUserWidget.h         # Classe base com auto-unsubscribe
│   │       ├── SBStatusHUDWidget.h    # Backing class para Vida/Mana (BindWidget)
│   │       ├── SBInteractionPromptWidget.h # Backing class para interação
│   │       ├── SBAbilityBarWidget.h   # Backing class para cooldown local
│   │       └── SBInventoryGridWidget.h # Backing class para atualização de slots
│   └── Private/
│       ├── Widgets/
│       │   ├── SBUserWidget.cpp
│       │   ├── SBStatusHUDWidget.cpp
│       │   ├── SBInteractionPromptWidget.cpp
│       │   ├── SBAbilityBarWidget.cpp
│       │   └── SBInventoryGridWidget.cpp
│       └── Tests/
│           └── SBUITests.cpp          # Testes de auto-unsubscribe e anti-spill
```

#### 10. [`10_SandboxDebug`](file:///D:/Unreal/V1/Plugins/10_SandboxDebug)
*   **Propósito**: Painéis de telemetria do Gameplay Debugger nativo da Unreal.
```
├── Source/SandboxDebug/
│   ├── Public/
│   │   └── GameplayDebuggerCategory_Sandbox.h # Depuração visual em tempo real
│   └── Private/
│       └── GameplayDebuggerCategory_Sandbox.cpp
```

#### 11. [`11_SandboxEditor`](file:///D:/Unreal/V1/Plugins/11_SandboxEditor)
*   **Propósito**: Validadores estáticos e customizações do editor sem vazamentos de runtime.
```
├── Source/SandboxEditor/
│   └── Private/
│       └── SandboxEditorModule.cpp
```
