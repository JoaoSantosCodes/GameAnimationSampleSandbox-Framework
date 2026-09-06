# Manual de Uso Simplificado - Sandbox Framework (v1.28.0)

Guia rápido e prático de referência para inicialização e uso das mecânicas do **Sandbox Framework** no workspace `GameAnimationSample`.

---

## 🚀 Setup Rápido
### Passo 1: Configurar os Componentes via Dados (PawnData)
O personagem não possui componentes hardcoded. Tudo é injetado dinamicamente:
1. Clique com o botão direito no Content Browser -> **Miscellaneous** -> **Data Asset**.
2. Escolha **`USBPawnData`** como classe e nomeie-o (ex: `DA_HeroPawnData`).
3. Abra-o e adicione os componentes desejados no array **`ComponentsToGrant`** (ex: `SBMovementComponent`, `SBInventoryComponent`, `SBCombatComponent`, `SBInteractionComponent`, `USBStatusEffectComponent`).
4. Crie um Blueprint derivado de `ASBCharacter` (ex: `BP_SBCharacter_Hero`) e configure a variável **`PawnData`** no painel Details apontando para o seu `DA_HeroPawnData`.

### Passo 2: Mapear Inputs Dinâmicos
1. Crie um Data Asset herdando de **`USBInputConfig`** (ex: `DA_InputConfig`).
2. Mapeie um **InputAction** a uma **GameplayTag** (ex: `Input.Action.Sprint` para a tecla Shift).
3. No seu `DA_HeroPawnData`, atribua este `DA_InputConfig` no campo correspondente.

### Passo 3: Configurar Atributos Públicos e Privados (Fase 25)
1. No `USBAttributeComponent` do seu Blueprint, ao registrar atributos, configure a flag **`bIsPrivate`** (bool).
2. Defina `bIsPrivate = True` para atributos sensíveis (ex: **Mana**, **Stamina**, **Ammo**). O servidor irá replicá-los sob a condição `COND_OwnerOnly`, evitando que outros clientes os interceptem na rede.
3. Deixe `bIsPrivate = False` para atributos compartilhados (ex: **Health**).

### Passo 4: Criar Widgets Reativos (UMG)
Em **Class Settings** dos seus Widgets, altere a classe pai para as backing classes do plugin `09_SandboxUI`:
*   `USBStatusHUDWidget` (Vida/Mana) -> Adicione progress bars chamadas **`PB_Health`** e **`PB_Mana`**.
*   `USBInteractionPromptWidget` (Interações) -> Adicione Text Block **`TXT_Prompt`** e ProgressBar **`PB_HoldProgress`**.
*   `USBAbilityBarWidget` (Hotbar Cooldown) -> Adicione Image **`IMG_CooldownMask`** e Text Block **`TXT_CooldownTime`**. Defina a tag do slot em **`WatchedAbilityTag`** Details do widget (ex: `Ability.Teleport`).
*   `USBInventoryGridWidget` (Inventário) -> Implemente o evento `BP_OnSlotUpdated` no gráfico.

### Passo 5: Persistir Dados (Save Game)
Qualquer Ator ou Componente que implemente a interface **`ISBSaveInterface`** é salvo automaticamente:
*   Para salvar o estado do jogo atual em runtime:
    ```cpp
    USBSaveSubsystem::Get(GetWorld())->SaveGame(TEXT("SlotName"));
    ```
*   Para carregar o estado salvo:
    ```cpp
    USBSaveSubsystem::Get(GetWorld())->LoadGame(TEXT("SlotName"));
    ```

### Passo 6: Anti-Cheat e Segurança (Fase 24)
* **Anti-Cheat de Velocidade**: O servidor valida a movimentação a partir da velocidade máxima teórica calculada por `GetCalculatedMaxSpeed()`. O CMC e o atributo de velocidade devem ser mantidos sincronizados (desvios em runtime geram avisos de desync a cada 5 segundos).
* **Wall-Shot Protection**: Disparos hitscan são invalidados pelo servidor se houver barreiras físicas estáticas entre o tórax do atirador (obtido dinamicamente usando metade do capsule half height) e o ponto de impacto.

### Passo 7: Proteção Anti-Cheat de Saves (Fase 39)
* Os saves são criptografados com validação de checksum MD5 automaticamente.
* Defina a chave privada no arquivo `Config/DefaultGame.ini` sob a chave `SaveEncryptionKey` da seção `[/Script/SandboxCommon.SBDeveloperSettings]`.
* Modificações não autorizadas ou corrupção do hash de assinatura abortam o carregamento síncrono.

### Passo 8: Som de Passos e Zonas Ambientais (Fases 40 & 41)
* **Passos**: Crie um `USBSurfaceEffectsDataAsset` definindo sons/VFX por material físico. Na animação, adicione a notificação **`USBAnimNotify_Footstep`** apontando para o socket do pé e o data asset.
* **Zonas**: Arraste um ator **`SBAmbientZoneTrigger`** para o nível, posicione-o sobre a área e configure o arquivo de som looping e tempos de fade-in e fade-out. O processamento é realizado de forma otimizada no cliente.

### Passo 9: Missões e Comércio (Fases 42 & 43)
* **Missões**: Crie um `USBQuestDataAsset`, defina seus objetivos (ex: `Quest.Objective.Footstep`) e recompensas. Adicione `USBQuestComponent` ao player. O progresso é incrementado de forma autoritativa no servidor por eventos publicados no `USBEventSubsystem`.
* **Comércio**: Adicione `USBMerchantComponent` a um ator do mapa, configure o estoque de `USBItemDefinition` e preços. Use `ServerBuyItem` e `ServerSellItem` para transacionar itens de forma segura, verificando saldo do atributo `Attribute.Coins` do cliente, espaço de inventário e proximidade.

---

## 🛠️ Comandos Úteis

### Compilar Projetos via Terminal
```powershell
dotnet "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" GameAnimationSampleEditor Win64 Development "D:\Unreal\GameAnimationSample\GameAnimationSample.uproject" -waitmutex
```

### Executar Testes de Automação
```powershell
& "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "D:\Unreal\GameAnimationSample\GameAnimationSample.uproject" -NullRHI -NoSound -NoSplash -stdout -ExecCmds="Automation RunTest Sandbox; Quit" -log
```
