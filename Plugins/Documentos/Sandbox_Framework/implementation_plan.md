# Plano de Implementação - Marco 8: Persistência Segura e Proteção contra Cheat (Fase 39)

Este plano descreve o design, a arquitetura e as etapas de implementação para a **Fase 39** do Sandbox Framework, introduzindo a persistência de saves criptografados e assinados digitalmente (`Anti-Save Scumming`) no subsistema `USBSaveSubsystemConcrete`.

---

## 📐 Especificação da Nova Fase

### Fase 39: Criptografia de Saves e Proteção Anti-Save Scumming (`04_SandboxCore`)
*   **Objetivo**: Prevenir que jogadores burlem mecânicas duplicando, modificando ou forjando arquivos de Save Game locais (cheating/save scumming).
*   **Regras de Negócio**:
    *   **Wrapper Seguro (`USBSecureSaveGame`)**: Criar uma classe contêiner de Save Game contendo o payload encriptado e uma assinatura digital.
    *   **Criptografia XOR de Fluxo Dinâmico**: Obfuscar os dados de serialização usando um algoritmo XOR com chave secreta salt privada (`SandboxAntiSaveScummingKey2026SecureSalt`).
    *   **Assinatura HMAC-MD5 de Integridade**: Gerar um checksum MD5 seguro baseado na concatenação do payload cifrado e da chave secreta.
    *   **Bloqueio de Carregamento**: Ao tentar carregar o save, o subsistema calcula a assinatura e compara com a armazenada. Se divergir (arquivo adulterado), o carregamento falha instantaneamente com alerta de segurança no log.

---

## 🛠️ Modificações Propostas por Componente

### Public Header: [`SBSaveSubsystemConcrete.h`](file:///D:/Unreal/GameAnimationSample/Plugins/04_SandboxCore/Source/SandboxCore/Public/Subsystems/SBSaveSubsystemConcrete.h)
*   Declarar a classe contêiner `USBSecureSaveGame` herdada de `USaveGame`.
*   Declarar os métodos privados helpers `EncryptDecryptData` e `CalculateSignature` na classe `USBSaveSubsystemConcrete`.

### Private Source: [`SBSaveSubsystemConcrete.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/04_SandboxCore/Source/SandboxCore/Private/Subsystems/SBSaveSubsystemConcrete.cpp)
*   Implementar as funções `EncryptDecryptData` e `CalculateSignature`.
*   Adaptar o método `SaveGame` para serializar o save game normal em memória, encriptar, assinar e persistir usando o wrapper `USBSecureSaveGame`.
*   Adaptar o método `LoadGame` para carregar o wrapper, validar a assinatura digital, decifrar o payload e reconstituir o save original a partir da memória.

---

## 🔬 Plano de Verificação

### Testes Automatizados (C++)
Escreveremos a suíte de testes unitários [`SBSecureSaveTests.cpp`](file:///D:/Unreal/GameAnimationSample/Plugins/08_SandboxInventory/Source/SandboxInventory/Private/Tests/SBSecureSaveTests.cpp) no plugin de inventário (onde ficam os testes de integração do save) cobrindo:
1.  **Salvamento e Carregamento Seguros**: Confirmar que salvar e carregar de forma legítima funciona perfeitamente, mantendo todos os dados íntegros.
2.  **Detecção de Adulteração (Tampering)**: Simular uma modificação maliciosa no array de bytes criptografado do save e validar que o carregamento é rejeitado com falha.
3.  **Assinatura Inválida**: Modificar apenas a assinatura armazenada e validar que a integridade é violada, resultando em rejeição segura.

---

## ❓ Perguntas Abertas para o Usuário

> [!NOTE]
> Nenhum impedimento técnico ou pergunta aberta foi identificado. A implementação usará inteiramente as APIs nativas do Unreal Engine (`FMD5`, `UGameplayStatics::SaveGameToMemory`/`LoadGameFromMemory`).
