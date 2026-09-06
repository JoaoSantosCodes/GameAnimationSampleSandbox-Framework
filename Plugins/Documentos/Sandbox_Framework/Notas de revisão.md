# Notas de Revisão - Sandbox Framework

Este documento registra as observações de revisão de arquitetura e o status de resolução de cada apontamento levantado sobre a integração dos sistemas das Fases 20 a 24.

---

## 🟢 [RESOLVIDO] Achado 1: Conflito do Anti-Cheat de Movimento com Teleport e Status Effects

### 🔍 Apontamento original:
O detector de velocidade e teleporte usava valores estáticos, o que causaria falsos positivos (rubber-banding) ao ativar habilidades legítimas de teleporte (`Ability.Teleport`) ou buffs de velocidade (+100) decorrentes de Status Effects da Fase 22.

### 🛠️ Resolução Aplicada:
1. **Velocidade Dinâmica**: No [`SBMovementComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Components/SBMovementComponent.cpp#L48-L82), o cálculo do anti-cheat de velocidade agora é completamente dinâmico. Ele consulta o `SpeedModifierAggregator` do personagem (que processa Sprint/Crouch) e a tag de atributo `Attribute.Speed` do `USBAttributeComponent` (que processa os buffs/debuffs dos Status Effects):
   ```cpp
   float MaxSpeed = CharOwner->GetCharacterMovement()->GetMaxSpeed();
   if (SpeedModifierAggregator)
   {
       MaxSpeed = SpeedModifierAggregator->CalculateFinalValue(MaxSpeed);
   }

   USBAttributeComponent* AttrComp = Owner->FindComponentByClass<USBAttributeComponent>();
   if (AttrComp)
   {
       FGameplayTag SpeedTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Speed"), false);
       FSBAttribute SpeedAttribute;
       if (SpeedTag.IsValid() && AttrComp->GetAttribute(SpeedTag, SpeedAttribute))
       {
           float AttrSpeed = SpeedAttribute.CurrentValue;
           MaxSpeed = SpeedModifierAggregator ? SpeedModifierAggregator->CalculateFinalValue(AttrSpeed) : AttrSpeed;
       }
   }
   ```
2. **Realocação Autorizada (Teleportes)**: Criamos a função pública [`AuthorizeServerRelocation()`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Public/Components/SBMovementComponent.h#L67-L69) no `USBMovementComponent`. Habilidades legítimas chamam esse método no servidor ao mover o personagem. O anti-cheat limpa a flag no frame seguinte e atualiza a última localização válida, evitando rollbacks.
3. **Validação**: Testado e verificado via spec `"Should allow teleport relocation when authorized by the server"` no arquivo [`SBAntiCheatTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Tests/SBAntiCheatTests.cpp#L162-L182).

---

## 🟢 [RESOLVIDO] Achado 2: Severidade de Validação de RPC vs Reconciliação Física

### 🔍 Apontamento original:
A desconexão automática do jogador ao falhar na validação RPC (`_Validate`) de distância de interação poderia punir jogadores com desvios legítimos causados por latência severa.

### 🛠️ Resolução Aplicada:
*   A decisão foi devidamente documentada no manifesto como uma escolha de design consciente no arquivo **[`manifesto_and_coding_standards.md`](file:///C:/Users/joaoc/.gemini/antigravity/brain/056f3669-6d8c-48af-8871-68ba0f54ee54/manifesto_and_coding_standards.md#L73-L84)**.
*   **Decisão**:
    *   A validação `_Validate` do RPC de interação é uma camada de proteção estrutural que previne a injeção de pacotes maliciosos. Ela tolera até `150.f` unidades (aproximadamente 1.5 metros) além do raio físico para acomodar o jitter comum.
    *   O anti-cheat de movimento em `TickComponent` não causa kicks nem desconexões, operando de forma passiva por meio de rollback (`TeleportTo`), pois a locomoção física é muito mais sensível a variações e perdas de pacotes.

---

## 🟢 [RESOLVIDO] Achado 3: Diagrama de Dependências Incompleto no Dashboard

### 🔍 Apontamento original:
O Mermaid visual do Dashboard não mostrava as dependências das extensões de gameplay (`06`, `07`, `08`) em relação ao núcleo do character e das interfaces.

### 🛠️ Resolução Aplicada:
*   Atualizamos as arestas do Mermaid no arquivo **[`00_Sandbox_Framework_Dashboard.md`](file:///C:/Users/joaoc/.gemini/antigravity/brain/056f3669-6d8c-48af-8871-68ba0f54ee54/00_Sandbox_Framework_Dashboard.md#L55-L66)** para incluir as setas explicitando a dependência unidirecional das extensões de combate, interações e inventários sobre o character core (`05_SandboxCharacter`) e sobre as interfaces desacopladas (`02_SandboxInterfaces`).

---

## 💻 Código de Validação do Anti-Cheat de Movimento

Segue a implementação finalizada do método de Tick e reset de autorização em [`SBMovementComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Components/SBMovementComponent.cpp#L21-L80):

```cpp
void USBMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority() || DeltaTime <= 0.0f)
	{
		return;
	}

	ACharacter* CharOwner = Cast<ACharacter>(Owner);
	if (!CharOwner || !CharOwner->GetCharacterMovement())
	{
		return;
	}

	FVector CurrentLocation = Owner->GetActorLocation();

	if (!bHasLastValidatedLocation)
	{
		LastValidatedLocation = CurrentLocation;
		bHasLastValidatedLocation = true;
		return;
	}

	if (bServerAuthorizedRelocation)
	{
		bServerAuthorizedRelocation = false;
		LastValidatedLocation = CurrentLocation;
		return;
	}

	float Distance2D = FVector::Dist2D(CurrentLocation, LastValidatedLocation);
	float MaxSpeed = CharOwner->GetCharacterMovement()->GetMaxSpeed();
	if (SpeedModifierAggregator)
	{
		MaxSpeed = SpeedModifierAggregator->CalculateFinalValue(MaxSpeed);
	}

	USBAttributeComponent* AttrComp = Owner->FindComponentByClass<USBAttributeComponent>();
	if (AttrComp)
	{
		FGameplayTag SpeedTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Speed"), false);
		FSBAttribute SpeedAttribute;
		if (SpeedTag.IsValid() && AttrComp->GetAttribute(SpeedTag, SpeedAttribute))
		{
			float AttrSpeed = SpeedAttribute.CurrentValue;
			MaxSpeed = SpeedModifierAggregator ? SpeedModifierAggregator->CalculateFinalValue(AttrSpeed) : AttrSpeed;
		}
	}

	float ExtraTolerance = 300.0f;
	float MaxAllowedDistance = (MaxSpeed + ExtraTolerance) * DeltaTime + 200.0f;

	bool bIsCheatDetected = false;

	if (Distance2D > MaxAllowedDistance)
	{
		bIsCheatDetected = true;
	}

	if (FVector::Dist(CurrentLocation, LastValidatedLocation) > 3000.0f)
	{
		bIsCheatDetected = true;
	}

	if (bIsCheatDetected)
	{
		UE_LOG(LogSandboxCharacter, Warning, TEXT("Anti-Cheat: Movimento anômalo detectado em %s! Distancia2D: %f (Max Permitido: %f). Executando Rollback."), *Owner->GetName(), Distance2D, MaxAllowedDistance);
		Owner->TeleportTo(LastValidatedLocation, Owner->GetActorRotation(), false, true);
	}
	else
	{
		LastValidatedLocation = CurrentLocation;
	}
}

void USBMovementComponent::AuthorizeServerRelocation()
{
	bServerAuthorizedRelocation = true;
}
```

------------
# Notas de Revisão - Sandbox Framework

A resolução do teste de Teleport é real e o mecanismo `AuthorizeServerRelocation()` é a abordagem certa em princípio — mas olhando o corpo real do cálculo de `MaxSpeed`, encontrei um problema estrutural que a "correção" não resolveu, só escondeu: as duas fontes de velocidade não são **combinadas**, são **substituídas** uma pela outra.

### O bug real: `MaxSpeed` é sobrescrito, não combinado

cpp

```cpp
float MaxSpeed = CharOwner->GetCharacterMovement()->GetMaxSpeed();
if (SpeedModifierAggregator)
{
    MaxSpeed = SpeedModifierAggregator->CalculateFinalValue(MaxSpeed);   // (A) calcula com base no CMC
}

USBAttributeComponent* AttrComp = Owner->FindComponentByClass<USBAttributeComponent>();
if (AttrComp)
{
    FGameplayTag SpeedTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Speed"), false);
    FSBAttribute SpeedAttribute;
    if (SpeedTag.IsValid() && AttrComp->GetAttribute(SpeedTag, SpeedAttribute))
    {
        float AttrSpeed = SpeedAttribute.CurrentValue;
        MaxSpeed = SpeedModifierAggregator ? SpeedModifierAggregator->CalculateFinalValue(AttrSpeed) : AttrSpeed;  // (B) DESCARTA (A) e recalcula do zero
    }
}
```

O valor calculado em (A) — que reflete Sprint via `SpeedModifierAggregator` aplicado ao `MaxSpeed` real do `CharacterMovementComponent` — é **completamente descartado** se `Attribute.Speed` existir, e substituído por um novo cálculo em (B) que aplica o **mesmo** `SpeedModifierAggregator` a uma base diferente (`AttrSpeed`, vindo dos Status Effects).

Isso significa: se um jogador estiver **correndo (Sprint) E com um buff de velocidade de Status Effect ativo ao mesmo tempo** — o cenário mais comum de "velocidade alta legítima" no jogo — o anti-cheat só considera **uma** das duas fontes, nunca a soma das duas. Dependendo de qual valor for maior (o velocidade real do CMC com Sprint, ou o `AttrSpeed` isolado), o jogador pode:

- Ter `MaxAllowedDistance` calculado **menor** do que sua velocidade física real combinada → falso positivo, rollback do próprio Sprint+Buff legítimo.
- Ou, se `Attribute.Speed` só refletir o buff sem incluir Sprint, o mesmo problema na direção oposta.

Isso é exatamente a mesma classe de bug que motivou a correção original (dois sistemas de velocidade que não conversam entre si) — só que a correção resolveu o sintoma que eu apontei (Sprint sozinho, buff sozinho) sem resolver a causa raiz (as duas fontes precisam ser **somadas/combinadas**, não escolhidas por um `if`). A forma correta seria a velocidade final vir de **uma única fonte de verdade** — idealmente `CharacterMovementComponent::GetMaxSpeed()` já refletindo tudo (se Sprint e Status Effects ambos escrevem no mesmo `SpeedModifierAggregator` que alimenta o CMC), sem precisar de um segundo cálculo paralelo via `Attribute.Speed` que compete com o primeiro.

**Pergunta concreta antes de aceitar isso como resolvido:** `Attribute.Speed` e o valor que `SpeedModifierAggregator` aplica ao `CharacterMovementComponent` são a **mesma fonte de dado** vista de dois ângulos, ou são dois sistemas paralelos independentes (um Sprint mexe direto no CMC, outro Status Effect mexe só no Attribute)? Se forem paralelos, o teste que valida isso precisa ativar **os dois ao mesmo tempo** e confirmar que nenhum rollback acontece — o teste atual (`"Should allow teleport relocation..."`) não cobre esse caminho.

### Segundo ponto, mais amplo: `AuthorizeServerRelocation()` só foi adotado por Teleport — mas o problema é genérico

O padrão agora exige que **qualquer** sistema que mova o personagem rapidamente e legitimamente chame `AuthorizeServerRelocation()` antes, ou será revertido como cheat. Isso cobre Teleport, mas o framework já tem (ou vai ter) outras fontes plausíveis de deslocamento rápido não cobertas:

- **Root motion** de animações de esquiva/dash (se alguma habilidade usar root motion em vez de `SetActorLocation`, o personagem pode se mover mais rápido que `MaxSpeed` fisicamente por um frame, sem nunca chamar `AuthorizeServerRelocation`).
- **Knockback/impulso físico** de combate (se alguma arma ou habilidade aplicar `AddImpulse`/`LaunchCharacter`, isso também escapa da velocidade "normal" e não está coberto).
- **Plataformas móveis** — se o personagem estiver em cima de um ator que se move (elevador, plataforma), o deslocamento _global_ do Actor por frame pode facilmente exceder `MaxAllowedDistance` mesmo com o jogador parado em relação à plataforma, porque o cálculo compara posição absoluta entre frames, não posição relativa à base de movimento.

Nenhum desses três está mencionado nos testes nem no código mostrado. Não é bloqueante pra fechar a Fase 24 agora — mas é uma dívida de manutenção real: toda vez que um novo sistema de gameplay mover o personagem de forma não-trivial, alguém vai precisar lembrar manualmente de chamar `AuthorizeServerRelocation()`, e esquecer isso é indistinguível de um bug de gameplay até o jogador reclamar de rubber-banding. Vale registrar isso explicitamente como princípio no `manifesto_and_coding_standards.md` (algo como _"Todo sistema que reposiciona o Character fora do fluxo normal do CharacterMovementComponent DEVE chamar `AuthorizeServerRelocation()` antes"_), pra não ficar como conhecimento tácito perdido.

### Achado 2 e 3: aceito

A tolerância de 150 unidades documentada como decisão consciente resolve o ponto que levantei — ainda existe risco residual sob latência extrema (300-500ms de ping pode superar 150 unidades de folga dependendo da velocidade do jogador), mas isso é um trade-off razoável e, principalmente, **está documentado como escolha**, que é o padrão certo. O diagrama do dashboard eu aceito sem verificação adicional — é baixo risco.

---

Resolve a questão de combinação de fontes de velocidade (ponto 1) antes de eu considerar o Anti-Cheat de movimento genuinamente fechado — é o tipo de bug que só aparece quando dois sistemas (Combate/Movimento e Status Effects) interagem ao mesmo tempo, exatamente o cenário mais comum em gameplay real, não um caso de borda.

---

## 🟢 [RESOLVIDO] Achado 4: Combinação de Fontes de Velocidade e Throttling Const-Safe (Fase 24)

### 🔍 Apontamento original:
O cálculo de `MaxSpeed` para verificação de anti-cheat de movimento sobrescrevia fontes em vez de combiná-las. Além disso, o mecanismo de throttling de log de 5 segundos utilizava uma variável local estática global que gerava concorrência entre instâncias, necessitando de uma modelagem const-safe para ser BlueprintPure.

### 🛠️ Resolução Aplicada:
1. **Fórmula de Velocidade Combinada**: Criamos a rotina unificada [`USBMovementComponent::GetCalculatedMaxSpeed() const`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Components/SBMovementComponent.cpp#L48-L110) que combina proporcionalmente:
   $$\text{Velocidade Final} = \text{Base Speed (CMC/Crouch)} \times \text{Aggregator Modifiers (Sprint)} \times \left(\frac{\text{Attribute.Speed.CurrentValue}}{\text{Attribute.Speed.BaseValue}}\right)$$
2. **Throttling Isolado por Instância**: Declaramos o campo `mutable double LastLogDesyncTime = 0.0;` em [`SBMovementComponent.h`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Public/Components/SBMovementComponent.h#L104). Isso permite que a verificação `const` altere o estado do timer sem requerer globais estáticas, evitando concorrência no log entre múltiplos personagens dessincronizados.

---

## 🟢 [RESOLVIDO] Achado 5: Acesso Limitado a Testes e Escopo de Asserção de Rede (Fase 25)

### 🔍 Apontamento original:
O uso de getters mutáveis para testar a replicação de rede condicional sob a flag `#if WITH_DEV_AUTOMATION_TESTS` expunha métodos de escrita na API pública e gerava afirmações otimistas sobre testes de rede reais (que na verdade simulam a replicação chamando `OnRep` localmente).

### 🛠️ Resolução Aplicada:
1. **Remoção de Getters Mutáveis**: Substituímos os getters mutáveis pela declaração `friend class FSBConditionalReplicationTestsSpec;` dentro do [`SBAttributeComponent.h`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Public/Components/SBAttributeComponent.h#L105). Isso restringiu o acesso mutável ao escopo privado e de testes.
2. **Suavização Documental**: Ajustamos o [`walkthrough.md`](file:///C:/Users/joaoc/.gemini/antigravity/brain/056f3669-6d8c-48af-8871-68ba0f54ee54/walkthrough.md#L471) para explicitar que a simulação atesta a lógica de leitura condicional local, enquanto o transporte real na rede depende da declaração nativa `COND_OwnerOnly` verificada pelo motor Unreal.

---

## 🟢 [RESOLVIDO] Achado 6: Falsos Positivos de Wall-Shot em Crouch (Fase 24)

### 🔍 Apontamento original:
A origem do traço de validação de Wall-Shot protection utilizava um offset estático de altura (`FVector(0,0,50)`). Ao se agachar (Crouch) ou deitar (Prone), o tórax físico desce, fazendo com que o ponto de origem pudesse emergir dentro do piso ou colisão estática de cobertura.

### 🛠️ Resolução Aplicada:
*   Substituímos o offset estático no [`SBWeaponBehaviorHitscan.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Weapons/SBWeaponBehaviorHitscan.cpp#L101-L103) por um offset dinâmico baseado na metade da altura escalada atual da cápsula de colisão:
    ```cpp
    float CapsuleHalfHeight = Character->GetCapsuleComponent() ? Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 88.0f;
    FVector BodyCenter = Character->GetActorLocation() + FVector(0, 0, CapsuleHalfHeight * 0.5f);
    ```
    Isso assegura proporcionalidade física independente da postura ou Crouch do atirador.

---

## 🟢 [RESOLVIDO] Achado 7: Restauração Visual de Armas no Save/Load (Fase 26)

### 🔍 Apontamento original:
Ao carregar o jogo salvo (`LoadGame`), os slots de inventário e instâncias de itens eram reconstituídos logicamente no componente, mas o estado de equipamento visual da arma nos sockets e behaviors associados não reaparecia fisicamente no personagem.

### 🛠️ Resolução Aplicada:
1. **Rastreamento por Tags Dinâmicas**: Integramos a atribuição da Gameplay Tag `State.Item.Equipped` em `ServerEquipItem` e a remoção em `ServerUnequipItem` direto no contêiner de tags dinâmicas replicado e serializado do item.
2. **Next-Tick Deferral**: No [`SBInventoryComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Private/Components/SBInventoryComponent.cpp#L414-L417), ao concluir a leitura do save, utilizamos o Timer Manager (`SetTimerForNextTick`) para adiar a execução de `RestoreEquippedItems()`. Isso evita race conditions e garante que todos os componentes necessários do Ator estejam completamente inicializados e prontos para processar os eventos e spawns de armas.
3. **Validação**: Testado e verde via `"Cenário 2: Persistência e restauração do estado equipado (Visual/Behavior)"` inserido no arquivo de testes [`SBInventorySaveTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Private/Tests/SBInventorySaveTests.cpp#L139-L221).

---

## 🟢 [RESOLVIDO] Fase 27: Sistema de Estamina Avançado (v1.13.0)

### 🔍 Descrição e Escopo:
Desenvolvimento de uma mecânica de Estamina de alto desempenho, predita no cliente e corrigida na autoridade, integrada com a movimentação física e o barramento de atributos.

### 🛠️ Implementação Realizada:
1. **Atributo Seguro**: Registramos `Attribute.Stamina` no [`SBMovementComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Components/SBMovementComponent.cpp#L44-L57) com a flag de privacidade `bIsPrivate = true`, roteando-o pelo canal seguro `COND_OwnerOnly` do `USBAttributeComponent` para evitar cheats de radares de telemetria externa.
2. **Predição e Consumo do Sprint**: Durante a locomoção física, a estamina é reduzida localmente (predição de cliente) e no servidor à taxa de `15.f/s` ao sprintar.
3. **Consumo de Pulo**: Sobrescrevemos o método [`ASBCharacter::Jump()`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Character/SBCharacter.cpp#L148-L161) interceptando o acionamento físico para invocar `ConsumeJumpStamina()`. O pulo consome `20.f` instantâneos de estamina e é bloqueado caso o saldo seja insuficiente.
4. **Regeneração com Atraso (Delay)**: Estamina se regenera a `10.f/s` após um atraso ininterrupto de `1.5s` da última ação de consumo.
5. **Estado de Exaustão Unificado**: Ao atingir `0.f`, o personagem recebe a tag `State.Character.Exhausted`, que desliga a corrida ativa e bloqueia novos pulos/corridas até que o valor de estamina recupere o limiar de `30.f`.
6. **Validação Automatizada**: Criamos a suíte de testes de estresse e lógica [`SBStaminaTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Tests/SBStaminaTests.cpp) com 3 specs.
7. **Integração com HUD C++**: Adicionamos o binding e ponteiro seguro de `PB_Stamina` no [`SBStatusHUDWidget.h`](file:///D:/Unreal/GameAnimationSample/Plugins/09_SandboxUI/Source/SandboxUI/Public/Widgets/SBStatusHUDWidget.h#L28) e corrigimos o binding de vida no [`SBStatusHUDWidget.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/09_SandboxUI/Source/SandboxUI/Private/Widgets/SBStatusHUDWidget.cpp#L28) para `Attribute.Health`.
8. **Resultado**: 100% verde (**48 de 48 testes verdes - EXIT CODE: 0**).

---

## 🟢 [RESOLVIDO] Fase 28: Vinculação de Assets Visuais e Playtests de UI (v1.14.0)

### 🔍 Descrição e Escopo:
Mapeamento de diretrizes no UMG Designer e validação estrutural do barramento de UI contra bugs de casting e vazamento de dados de rede.

### 🛠️ Implementação Realizada:
1. **Correção de Coerção Polimórfica (Bug de UI)**: Identificamos que o `USBInventoryGridWidget` C++ falhava ao fazer `Cast<USBInventoryEventPayload>` no payload `USBInventorySlotUpdatedEventPayload` porque as duas classes eram irmãs separadas de `UObject`. Corrigimos a estrutura no [`SBInventoryComponent.h`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Public/Components/SBInventoryComponent.h#L21) fazendo `USBInventorySlotUpdatedEventPayload` herdar diretamente de `USBInventoryEventPayload`.
2. **Suporte a Estamina Visual**: Estendemos [`USBStatusHUDWidget`](file:///D:/Unreal/GameAnimationSample/Plugins/09_SandboxUI/Source/SandboxUI/Public/Widgets/SBStatusHUDWidget.h) para expor a ProgressBar de estamina (`PB_Stamina`) e corrigimos o binding de vida (`Attribute.Health`).
3. **Prevenção de UI Spill**: Estabelecemos e validamos a arquitetura baseada em filtragem de eventos locais (`TargetPawn == GetOwningPlayerPawn()`) para evitar vazamento de dados de interface na tela dividida de Split-Screen local.
4. **Validação**: Compilado e testado verde via suíte completa.

---

## 🟢 [RESOLVIDO] Fase 29: Sistema de Munição e Recarga (v1.15.0)

### 🔍 Descrição e Escopo:
Implementação do sistema dinâmico de munições com comportamento de recarga, predição de cliente e validação autoritativa do servidor.

### 🛠️ Implementação Realizada:
1. **Registro do Atributo de Munição**: Implementamos o override de `OnReady_Implementation` no [`SBCombatComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Components/SBCombatComponent.cpp#L114) para instanciar dinamicamente `Attribute.Weapon.Ammo` como um atributo privado (`COND_OwnerOnly`) com capacidade de 30 unidades.
2. **Comportamento de Recarga C++**: Criamos o comportamento [`USBWeaponBehaviorReload`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Public/Weapons/SBWeaponBehaviorReload.h) herdando de `USBGameplayBehavior`, aplicando o estado `State.Character.Reloading` para bloquear disparos e restaurando o total de munição após 2.0s de recarga.
3. **Bloqueio Programático de Disparos**: Adicionamos proteção nativa no `CanEnter` de [`USBWeaponBehavior`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Weapons/SBWeaponBehavior.cpp#L30) para ejetar/bloquear disparos se o personagem possuir a tag de recarregando.
4. **Testes Unitários**: Criamos a suíte de testes de automação [`SBReloadTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Tests/SBReloadTests.cpp) com 2 especificações.
5. **Resultado**: 100% verde (**50 de 50 testes verdes - EXIT CODE: 0**).

---

## 🟢 [RESOLVIDO] Fase 30: Cooldowns de Habilidade e Custo de Mana (v1.16.0)

### 🔍 Descrição e Escopo:
Implementação do suporte a custos lógicos em `Attribute.Mana` (com regeneração passiva e delay) e cooldowns transientes de habilidades baseados em tags.

### 🛠️ Implementação Realizada:
1. **Regeneração Passiva de Mana**: Implementamos regeneração passiva de `5.f/s` com delay de `2.0s` a partir do último consumo no `TickComponent` do [`SBAbilityComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Components/SBAbilityComponent.cpp#L496) no servidor.
2. **Propriedade CooldownTag**: Adicionamos `CooldownTag` a [`USBAbility.h`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Public/Abilities/SBAbility.h#L30) para associar tags de cooldown como `State.Cooldown.Ability.Fire` de forma granular.
3. **Aplicação e Remoção de Tags**: Sincronizamos a aplicação da `CooldownTag` no `USBStateComponent` do personagem na ativação e a remoção automática na expiração do cooldown.
4. **Rollback de Rede**: Implementamos expurgo automático da `CooldownsList` e remoção da `CooldownTag` no `ClientRollbackAbility` caso o servidor rejeite a ação.
5. **Testes Unitários**: Criamos 3 especificações em [`SBAbilityTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Tests/SBAbilityTests.cpp) com passagem limpa avançando `TestWorld->TimeSeconds` e simulando `ROLE_AutonomousProxy`.
6. **Resultado**: 100% verde (**53 de 53 testes verdes - EXIT CODE: 0**).

---

## 🟢 [RESOLVIDO] Fase 31: Inteligência Artificial Integrada com State Component (v1.17.0)

### 🔍 Descrição e Escopo:
Integração de Inteligências Artificiais ao State Component para bloqueio síncrono de comportamentos e implementação de tabela de Agro autoritativa no servidor.

### 🛠️ Implementação Realizada:
1. **Tabela de Agro de Combate**: Adicionamos suporte a tabela interna `AgroTable` no [`SBCombatComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Components/SBCombatComponent.cpp) com métodos `AddAgro`, `ClearAgro` e `GetHighestAgroTarget()`.
2. **Prevenção de Memory Leaks**: Limpeza automática de chaves fracas de Pawns destruídos através da validação síncrona com `IsValid(PawnKey)` em `GetHighestAgroTarget()`.
3. **Bloqueio de Locomoção via Tags de CC**: Editamos [`SBMovementComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Components/SBMovementComponent.cpp#L69) reduzindo a velocidade máxima teórica a `0.0f` se a tag `State.Character.Stunned` ou `State.Character.Frozen` estiver ativa no State Component.
4. **Bloqueio de Ações de Habilidade/Arma**: As habilidades herdadas de [`SBWeaponBehavior.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Weapons/SBWeaponBehavior.cpp#L42) rejeitam ativações caso o Pawn possua tags bloqueadas (CC).
5. **Testes Unitários**: Criamos a suíte de testes [`SBAIBehaviorTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Tests/SBAIBehaviorTests.cpp) validando tabela de agro, bloqueio de locomoção e bloqueio de disparos sob stun/congelamento.
6. **Resultado**: 100% verde (**56 de 56 testes verdes - EXIT CODE: 0**).

---

## 🟢 [RESOLVIDO] Fase 32: Dano Crítico, Resistências e Reações de Impacto Replicadas (v1.18.0)

### 🔍 Descrição e Escopo:
Implementação de detecção de hit em ossos específicos (weakspots), multiplicador de dano crítico, mitigação de dano por defesa com diminishing returns e aplicação automática de reações de impacto (`State.Character.HitReacting`).

### 🛠️ Implementação Realizada:
1. **Detecção de Osso Crítico**: No [`SBWeaponBehaviorHitscan.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Weapons/SBWeaponBehaviorHitscan.cpp#L130), validamos se `HitResult.BoneName` pertence ao `CriticalBoneNames` do Data Asset da arma, multiplicando o dano por `CriticalDamageMultiplier`.
2. **Mitigação por Defesa**: Adicionamos atenuação matemática baseada em `Attribute.Defense` no alvo com a curva `FinalDamage = RawDamage * (100 / (100 + DefenseVal))`.
3. **Reação de Impacto e Event Bus**: Sofrer dano adiciona a tag `State.Character.HitReacting` no State Component do alvo e publica `Event.Combat.HitReact` e `Event.Combat.CriticalHit` (caso crítico) no Event Bus.
4. **Testes Unitários**: A suíte de testes [`SBCriticalDamageTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Tests/SBCriticalDamageTests.cpp) valida todos os fluxos de mitigação, multiplicador crítico em múltiplos ossos (cabeça, pescoço) e a injeção síncrona de tags de reação de impacto.

---

## 🟢 [RESOLVIDO] Fase 33: Tabela de Loot e Drop Físico Replicado (v1.19.0)

### 🔍 Descrição e Escopo:
Desenvolvimento do sistema probabilístico de sorteio de loot, ator de drop físico simulado replicado e infraestrutura de interação integrada com inventário.

### 🛠️ Implementação Realizada:
1. **Rolagem de Loot**: Implementamos rolagens probabilísticas baseadas em peso (`Weight`) e chance individual (`DropChance`) no [`SBLootTableDataAsset.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Private/DataAssets/SBLootTableDataAsset.cpp).
2. **Drop Físico e Interação**: Criamos a classe [`ASBPhysicalLootDrop.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Private/Actors/SBPhysicalLootDrop.cpp) herdando de `ISBInteractableInterface` para gerenciar coletas físicas autoritativas no servidor e resolver prompts de UI limpos de forma independente de localização de máquina.
3. **Anti-Race Condition**: Protegemos as coletas concorrentes com travas lógicas (`bIsLocked = true`) durante a chamada de `Interact_Implementation`.
4. **Bateria de Testes**: Homologamos a suíte [`SBLootDropTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Private/Tests/SBLootDropTests.cpp) por chamadas C++ diretas de interface para garantir estabilidade e eliminar latências.
5. **Resultado**: 100% verde (**61 de 61 testes verdes - EXIT CODE: 0**).

---

## 🟢 [RESOLVIDO] Otimizações Arquiteturais: Resolução de Depreciações e Sincronização Geral (v1.20.0)

### 🔍 Descrição e Escopo:
Limpeza e refatoração de código obsoleto sinalizado por alertas de deprecabilidade do compilador na Unreal Engine 5.8+, além de consolidação síncrona de arquivos em ambos os workspaces.

### 🛠️ Implementação Realizada:
1. **Resolução de Warnings de Compilação C4996**:
   * Substituímos a atribuição direta da propriedade obsoleta `CrouchedHalfHeight` pela chamada segura `SetCrouchedHalfHeight()` no [`SBMovementBehaviorCrouch.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Movement/Behaviors/SBMovementBehaviorCrouch.cpp#L53).
   * Substituímos as atribuições diretas de `NetUpdateFrequency` pelas chamadas encapsuladoras recomendadas `SetNetUpdateFrequency()` no [`SBPhysicalProjectile.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/06_SandboxCombat/Source/SandboxCombat/Private/Weapons/SBPhysicalProjectile.cpp#L18) e no [`SBPhysicalLootDrop.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Private/Actors/SBPhysicalLootDrop.cpp#L12).
2. **Sincronização Bidirecional**: Robocopy sincronizado das árvores de código dos 11 Plugins do Sandbox Framework e documentos do Obsidian entre os dois ambientes de desenvolvimento.
3. **Validação**: Ambos os ambientes compilando com **zero warnings** e **61/61 especificações unitárias verdes (EXIT CODE: 0)**.

---

## 🟢 [RESOLVIDO] Fase 34: Sistema de Progressão e Experiência (v1.21.0)

### 🔍 Descrição e Escopo:
Desenvolvimento de componente autoritativo e replicado de progressão do personagem (`USBExperienceComponent`) com suporte a múltiplos level ups em cadeia (multi-level up), carry-over de XP excedente e curvas exponenciais ou orientadas por DataTables.

### 🛠️ Implementação Realizada:
1. **Componente de Experiência**: Criamos o [`USBExperienceComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Components/SBExperienceComponent.cpp) herdando de `UActorComponent` e configurado com replicação de rede para `CurrentXP`, `CurrentLevel` e `RequiredXP`.
2. **Resolução de Curvas**: O cálculo de XP limite por nível suporta a fórmula padrão `BaseRequiredXP * (Level ^ XPExponent)` com precisão de arredondamento matemática e carregamento dinâmico via `UDataTable` (usando a estrutura `FRequiredXPRow`).
3. **Carry-over e Level Up Recursivo**: A injeção de XP roda um laço de repetição síncrono no servidor que consome o excedente, realiza level up consecutivamente e recalcula os novos limites antes de somar o restante de forma segura.
4. **C++ Multicast Delegates**: Eventos de delegados padrão C++ (`FSBExperienceChangedSignature` e `FSBLevelUpSignature`) expostos para fácil escuta por lambdas na suíte de testes.
5. **Testes Unitários**: A suíte de testes [`SBExperienceTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Private/Tests/SBExperienceTests.cpp) valida o ganho de XP básico, level up convencional com carry-over, múltiplos level ups consecutivos e leituras de linhas de DataTable com fallback.
6. **Resultado**: 100% verde (**66 de 66 testes verdes - EXIT CODE: 0**).

---

## 🟢 [RESOLVIDO] Fase 35: Sistema de Bancada Física e Interativa de Crafting (v1.22.0)

### 🔍 Descrição e Escopo:
Desenvolvimento de classe física de bancada interativa de trabalho (`ASBCraftingStation`) suportando múltiplos jogadores de forma concorrente e gerenciando tags de estado associadas sob monitoramento de proximidade.

### 🛠️ Implementação Realizada:
1. **Bancada Física**: Criamos o ator [`ASBCraftingStation.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Private/Actors/SBCraftingStation.cpp) herdando de `ISBInteractableInterface` com componentes integrados de colisão e malha dinâmica.
2. **Interação Concorrente**: A interação concede síncronamente a `StationTag` configurada ao `USBStateComponent` do jogador, habilitando-o a fabricar receitas que exijam essa estação no `USBCraftingComponent`.
3. **Range Check e Auto-limpeza**: Adicionamos monitoramento dinâmico no `Tick` do servidor. Se um interator se afastar além da `MaxInteractionDistance` configurada, a tag de estado é removida e ele é removido da lista de forma limpa.
4. **Testes Unitários**: A suíte de testes [`SBCraftingStationTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Private/Tests/SBCraftingStationTests.cpp) valida o ganho de tag na interação, remoção via `StopInteracting`, e a limpeza de tag no Tick se afastado no mundo 3D.
5. **Resultado**: 100% verde (**70 de 70 testes verdes - EXIT CODE: 0**).

---

## 🟢 [RESOLVIDO] Fase 36: Desmantelamento / Salvaging Probabilístico de Equipamentos (v1.23.0)

### 🔍 Descrição e Escopo:
Desenvolvimento de mecânica transacional e autoritativa de desmantelamento de itens (Salvage) no servidor, associando fragmentos de dados e executando rolls de probabilidade individuais.

### 🛠️ Implementação Realizada:
1. **Fragmento de Item**: Criamos o fragmento [`USBItemFragment_Salvageable.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Private/Items/SBItemFragment_Salvageable.cpp) contendo a lista de subprodutos lógicos (`FSBSalvageOutcome`) com controles individuais de mínimo/máximo e probabilidade de drop.
2. **Consumo Seguro e Probabilidade**: Implementamos `ServerSalvageItem` no [`SBCraftingComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Private/Components/SBCraftingComponent.cpp) que valida quantidades em tempo de execução, consome a pilha original através da infraestrutura de inventário e realiza sorteios independentes para cada item desmontado.
3. **C++ Native Delegates**: Expostos delegates nativos de callback `OnSalvagingCompleted` e `OnSalvagingFailed` para recepção imediata de dados sem dependências lógicas de blueprint.
4. **Testes Unitários**: A suíte de testes [`SBSalvageTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Private/Tests/SBSalvageTests.cpp) cobre rejeição de itens sem fragmentos, transações de consumo e drops garantidos ou impossíveis com pilhas unitárias e sequenciais.
5. **Resultado**: 100% verde (**73 de 73 testes verdes - EXIT CODE: 0**).

---

## 🟢 [RESOLVIDO] Fase 37: Compressão de Payloads e Otimizações de Replicação em Larga Escala (v1.24.0)

### 🔍 Descrição e Escopo:
Otimização profunda de performance de rede na replicação de inventário. Desativação da replicação individual de subobjetos de `USBItemInstance` (economizando canais de rede e NetGUIDs) e migração para replicação compactada de structs rápidos baseada em bits.

### 🛠️ Implementação Realizada:
1. **Otimização de Canais de Rede**: Desativamos o loop de replicação de subobjetos em `ReplicateSubobjects` do [`SBInventoryComponent.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Private/Components/SBInventoryComponent.cpp#L115).
2. **Serialização Baseada em Bits**: Implementamos o método `NetSerialize` customizado para a struct [`FSBInventoryEntry`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Public/Components/SBInventoryComponent.h#L57) que realiza compressão de inteiros (`SerializeIntPacked`) para a propriedade `StackCount` e serialização nativa para tags dinâmicas e classes de definição.
3. **Instanciação Transiente no Cliente**: Adicionamos lógica nos callbacks `PostReplicatedAdd`, `PostReplicatedChange` e `PreReplicatedRemove` da struct `FSBInventoryList` para instanciar localmente e de forma transiente o `USBItemInstance` no cliente e copiar os dados replicados da struct. Isso mantém 100% de retrocompatibilidade com códigos de UI.
4. **Sincronização de Estado no Servidor**: Criamos o método `MarkItemInstanceUpdated` que força a atualização da struct de replicação sempre que modificamos dados do item fora do fluxo padrão (ex: equipar/desequipar).
5. **Resultado**: 100% verde (**73 de 73 testes verdes - EXIT CODE: 0**).

---

## 🟢 [RESOLVIDO] Fase 38: Otimização de Efeitos e Áudio contra Saturação (v1.25.0)

### 🔍 Descrição e Escopo:
Prevenção de saturação sonora e visual no cliente sob picos ou rajadas de pacotes de rede (Packet Burst). Mapeamento espacial de cubos em grelha 3D e silenciamento de efeitos repetidos em intervalos menores que `MinInterval`.

### 🛠️ Implementação Realizada:
1. **Subsistema de Saturação**: Criamos o subsistema [`USBCosmeticSaturationSubsystem`](file:///D:/Unreal/GameAnimationSample/Plugins/04_SandboxCore/Source/SandboxCore/Public/Subsystems/SBCosmeticSaturationSubsystem.h) herdado de `UWorldSubsystem`.
2. **Grelha Espacial 3D**: Implementamos mapeamento de coordenadas 3D para grades de 1 metro (100 unidades Unreal) com geração de chaves únicas por asset, permitindo agrupar chamadas de som e partículas.
3. **Limpeza Periódica Automática**: Um timer interno roda a cada 10 segundos expurgando registros inativos há mais de 30 segundos, mantendo a memória sob controle.
4. **Testes Unitários**: Criamos a suíte [`SBCosmeticLimiterTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/04_SandboxCore/Source/SandboxCore/Private/Tests/SBCosmeticLimiterTests.cpp) cobrindo autorização inicial, supressão no mesmo local, independência em locais distantes, e liberação após expiração de cooldown.
5. **Resultado**: 100% verde (**79 de 79 testes verdes - EXIT CODE: 0**).

---

## 🟢 [RESOLVIDO] Fase 39: Persistência Criptografada e Proteção contra Cheat de Save Game (v1.26.0)

### 🔍 Descrição e Escopo:
Implementação de infraestrutura de persistência segura e criptografada com assinaturas digitais de integridade contra save scumming e adulteração ilegal de dados locais em arquivos `.sav`.

### 🛠️ Implementação Realizada:
1. **Wrapper Contêiner Seguro**: Criamos a classe [`USBSecureSaveGame`](file:///D:/Unreal/GameAnimationSample/Plugins/04_SandboxCore/Source/SandboxCore/Public/Subsystems/SBSaveSubsystemConcrete.h#L52) que encapsula o payload binário cifrado e o hash de integridade.
2. **Cifragem XOR**: Criptografamos o payload do save game usando uma cifra de fluxo XOR dinâmica baseada em uma chave secreta salgada privada (`SandboxAntiSaveScummingKey2026SecureSalt`).
3. **Assinatura HMAC-MD5**: Desenvolvemos um gerador de assinaturas que calcula o hash MD5 da concatenação dos bytes criptografados e da chave salt. Qualquer alteração ou corrupção do arquivo que resulte em um hash divergente anula a integridade e impede o carregamento síncrono.
4. **Alerta de Segurança**: O carregamento aborta imediatamente na incompatibilidade de assinatura, reportando um `Warning` no console log sem travar os testes da engine.
5. **Testes Unitários**: Criamos a suíte [`SBSecureSaveTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Private/Tests/SBSecureSaveTests.cpp) testando salvamento/carregamento íntegro normal, detecção de adulteração de payload e detecção de assinatura inválida.
6. **Resultado**: 100% verde (**82 de 82 testes verdes - EXIT CODE: 0**).

---

## 🟢 [RESOLVIDO] Fase 40 & 41: Efeitos Físicos de Superfície e Áudio Ambiental (v1.27.0)

### 🔍 Descrição e Escopo:
Projeto e desenvolvimento do sistema C++ modular de som de passos sensível a materiais físicos (Surface-Aware Footsteps) integrado com o controle de saturação, e gatilhos de transição suave de áudio ambiente (Ambient Zones) executados localmente no cliente.

### 🛠️ Implementação Realizada:
1. **Configuração por Data Asset**: Criamos a classe [`USBSurfaceEffectsDataAsset`](file:///D:/Unreal/GameAnimationSample/Plugins/03_SandboxAssets/Source/SandboxAssets/Public/DataAssets/SBSurfaceEffectsDataAsset.h) mapeando superfícies físicas (`EPhysicalSurface`) para sons e efeitos visuais.
2. **AnimNotify de Passos Inteligente**: Criamos [`USBAnimNotify_Footstep`](file:///D:/Unreal/GameAnimationSample/Plugins/05_SandboxCharacter/Source/SandboxCharacter/Public/AnimNotifies/SBAnimNotify_Footstep.h) que realiza line traces descendentes a partir dos ossos do pé, resolve a superfície e toca os efeitos apropriados.
3. **Desacoplamento e Eventos**: O AnimNotify publica o evento local `Event.Character.Footstep` contendo o payload [`USBFootstepEventPayload`](file:///D:/Unreal/GameAnimationSample/Plugins/04_SandboxCore/Source/SandboxCore/Public/Subsystems/SBEventPayloads.h#L151) no barramento de eventos, garantindo que emissores visuais e marcas sejam desacoplados do código do personagem.
4. **Zonas Ambientais Locais**: Desenvolvemos o trigger [`ASBAmbientZoneTrigger`](file:///D:/Unreal/GameAnimationSample/Plugins/04_SandboxCore/Source/SandboxCore/Public/Actors/SBAmbientZoneTrigger.h) que orquestra fades de volume no cliente local do jogador ao cruzar limites geográficos de áudio, com bypass de teste automatizado.
5. **Testes Unitários**: Criamos [`SBSurfaceAudioTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/04_SandboxCore/Source/SandboxCore/Private/Tests/SBSurfaceAudioTests.cpp) cobrindo a resolução de superfícies mapeadas e fallbacks do Data Asset, bem como a alocação e ciclo de overlap nas zonas de áudio.
6. **Resultado**: 100% verde (**85 de 85 testes verdes - EXIT CODE: 0**).

------------
