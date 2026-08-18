# Sandbox Framework Coding Standards & Manifesto (v1.0.0)

Este documento define os princípios fundamentais e as regras de desenvolvimento estritas do **Sandbox Framework**. Ele serve como guia arquitetural para garantir consistência, performance e modularidade à medida que novas extensões e módulos de gameplay (como Movement, Combat e Weapons) forem integrados.

---

## 1. Sandbox Framework Manifesto: Os 10 Princípios Core

1. **Modularidade Absoluta (`Everything is Modular`)**
   Cada funcionalidade deve ser encapsulada em um plugin autocontido. Habilitar ou desabilitar um plugin não deve quebrar a execução de outros sistemas independentes.

2. **Orientação a Dados (`Everything is Data Driven`)**
   Nenhum parâmetro ou classe de gameplay deve ser estático ou *hardcoded*. Configurações visuais, físicas e lógicas devem ser expostas em `Data Assets` ou tabelas.

3. **Controle por Estado Físico (`State via Gameplay Tags`)**
   Os estados do personagem (ex: correndo, caindo, mirado) devem ser representados por tags no `SBStateComponent`. Consultas de estado de gameplay devem ler exclusivamente este componente.

4. **Desacoplamento por Interfaces (`Interfaces First`)**
   Atores e componentes nunca devem referenciar tipos concretos de outros plugins. Qualquer chamada lógica externa deve ser feita por meio de interfaces (`ISBCharacterInterface`, `ISBInteractableInterface`, etc.).

5. **Injeção Dinâmica de Componentes (`Component Injection`)**
   Nenhum componente deve ser instanciado diretamente no construtor do personagem. A injeção e ciclo de vida de componentes são controlados dinamicamente via `PawnData` e orquestrados pelo `SBComponentFactory`.

6. **Separação Estrita de Runtime e Editor (`Zero Editor Leaks`)**
   Códigos do editor (customização de painéis, seletores visuais, validadores) não devem vazar para os módulos de runtime. Módulos de editor devem residir exclusivamente em plugins com tipo `Editor`.

7. **Zero Dependências Circulares (`Circular Dependency Prohibition`)**
   A árvore de dependência deve ser unidirecional. Módulos de fundação (Common, Interfaces, Assets, Core) não podem importar ou depender de módulos de gameplay (Movement, Combat, Weapons).

8. **Suporte Nativo a Redes (`Multiplayer Ready`)**
   Toda lógica de gameplay, modificação de atributos e ativação de habilidades deve ser pensada para ambientes em rede com replicação apropriada, priorizando o controle do Servidor.

9. **Carregamento Otimizado (`Async Loading First`)**
   Assets pesados (Meshes, Animações, Efeitos) devem ser carregados assincronamente por meio do `SBAssetManager` a partir das IDs de Primary Assets dos Data Assets.

10. **Blueprint Opcional (`C++ Core, BP Configurable`)**
    Toda a lógica estrutural complexa e de alto desempenho deve ser escrita em C++. Blueprints devem ser utilizados para configuração de dados, ajustes artísticos e layouts visuais.

---

## 2. Sandbox Framework Coding Standards (Padrões de Engenharia)

### A. Regras para Componentes Modulares
- **Implementação Obrigatória**: Todos os componentes dinâmicos do framework devem herdar de `UModularActorComponent` (ou `USBAttributeComponent` / `USBStateComponent` quando estendendo infraestrutura existente) e implementar a interface `ISBComponentInterface`.
- **Proibição de Spawn Direto**: Um componente nunca deve criar outro componente diretamente usando `NewObject` ou `CreateDefaultSubobject`. Todo spawn de componentes de gameplay deve ser delegado ao `SBComponentFactory` via `ComponentSet`.
- **Ciclo de Vida Padronizado**:
  - `OnComponentCreated`: Instanciação inicial e alocações de memória internas.
  - `OnPreInitialize`: Registro preliminar de dados estáticos.
  - `OnInitialize`: Registro no `SBEventSubsystem` para escutar eventos.
  - `OnPostInitialize`: Consulta e cacheamento de outros componentes locais (via interfaces).
  - `OnReady`: Atores e subcomponentes prontos para gameplay.
  - `OnShutdown`: Cleanup, remoção de delegates e liberação de ponteiros.

### B. Regras de Comunicação e Eventos
- **Event Bus vs. Referências Diretas**: Não faça acoplamento para reportar ações. Se uma arma disparou, não chame o HUD ou o sistema de áudio diretamente. Dispare um evento no `SBEventSubsystem` (ex: `Event.Weapon.Fire`) e deixe os respectivos sistemas reagirem.
- **Assinaturas Priorizadas**: Ao se inscrever no Event Bus, classifique a prioridade (`ESBEventPriority`) do listener de acordo com o impacto de desempenho:
  - **High (`0`)**: AI, Física e cálculos internos.
  - **Medium (`10`)**: Animações, aplicação de modificadores ou efeitos de combate.
  - **Low (`20`)**: Atualizações de HUD, Widgets e telas do usuário.
  - **Lowest (`30`)**: Conquistas, logs analíticos e salvamentos locais.

### C. Persistência Desacoplada
- Se um componente necessita salvar dados persistentes (ex: vida atual, posição de inventário, munição), ele deve implementar a interface `ISBSaveInterface`.
- Os dados devem ser serializados no payload genérico fornecido no método `SaveComponentData` e restaurados em `LoadComponentData`.

### D. Regras para UI e Desenvolvimento de Widgets (UMG)
- **Foco do C++ (Lógica compilada)**: Toda lógica operacional de UI (assinaturas assíncronas do barramento de eventos, guards de nulidade, verificações de escopo local e interpolações matemáticas) deve residir em classes de suporte C++ (*backing classes*). Os Widget Blueprints (WBPs) devem ser herdados destas classes C++ e limitar-se a aspectos de design visual, layouts, âncoras e animações cosméticas.
- **Validação de Payload (Fail-Closed)**: Manipuladores de eventos de widget devem, obrigatoriamente, testar o payload e executar guards contra falha de cast e nulos síncronamente na entrada da função. Em caso de falha de cast ou tag ausente, o widget deve sair cedo (comportamento fail-closed) e opcionalmente logar um aviso explícito (usando `LogSandboxUI`), prevenindo bugs de processamento silencioso ou vazamentos.
- **Desinscrição Recursiva Obrigatória**: Todo widget que assina o barramento de eventos deve gerenciar suas inscrições em um array transiente e realizar a remoção cirúrgica de cada escuta individual no método `NativeDestruct()` (auto-unsubscribe). Isso mitiga vazamentos de memória e falhas de desreferenciamento pós-destruição (*Use-After-Free*).
- **Sem Polling de Rede (Interpolação Cosmética)**: É proibido ler variáveis replicadas por polling a cada frame no tick da UI (como o tempo de cooldown de habilidades ou progresso de interação). A UI deve escutar o evento inicial com a duração/percentual estáticos e realizar uma interpolação cosmética local (client-side) no tick do widget, minimizando o tráfego de rede e chamadas redundantes.
- **Logs de UI Qualificados**: Mensagens de diagnóstico ou avisos de má configuração de widgets no editor devem utilizar a categoria de domínio `LogSandboxUI`, permitindo filtros limpos no Output Log.

### E. Diretrizes de Segurança, Anti-Cheat e RPC Validation
- **RPC Validation vs. Disconnection (Princípio de Severidade)**:
  - **Validação de Protocolo (`_Validate`)**: Deve ser usada para verificar integridade estrutural, tipos corretos de parâmetros, tags de ação existentes e limites básicos de proximidade (ex: `InteractionRange + 150.f`). Falhas de `_Validate` indicam manipulação maliciosa de pacotes ou requisições inviáveis na simulação e resultam em **desconexão imediata** do cliente.
  - **Tolerância a Jitter em Interações**: A tolerância de `150.f` unidades em interações é um compromisso deliberado e consciente para acomodar a latência natural da rede sem causar desconexões acidentais de jogadores legítimos com alta latência.
  - **Validação Contínua de Simulação (Anti-Cheat de Movimento)**: Validações de movimento físico contínuo (como speedhacks ou warps) devem rodar em `TickComponent` e utilizar a velocidade dinâmica do personagem (calculada via `SpeedModifierAggregator` e atributos de modificadores ativos) somada a tolerâncias de rede.
  - **Rollback em vez de Kick no Movimento**: Flutuações de lag no movimento não devem desconectar o jogador. Em vez disso, o servidor deve forçar o rollback físico do transform (`TeleportTo`) para a última posição autorizada (`LastValidatedLocation`).
  - **Tokens de Realocação Autorizada**: Qualquer comportamento que desloque legitimamente o personagem de forma abrupta (como habilidades de teleporte) deve registrar um token transiente de autorização (`AuthorizeServerRelocation`) para que a validação de movimento do próximo frame ignore a discrepância de coordenadas e previna falsos positivos.

