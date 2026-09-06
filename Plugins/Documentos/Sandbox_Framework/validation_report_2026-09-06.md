# 🔎 Relatório de Validação — 06/09/2026

**Workspace**: `D:\Unreal\GameAnimationSample` (único — ver [[implementation_plan_pos_auditoria_2026-09-05|Bloco 5]])
**Escopo**: varrer o código-base atrás de novas ocorrências dos padrões de defeito que a
auditoria de 05/09/2026 descobriu, corrigir o que for defeito e registrar o que não for.

**Motivação**: o plano pós-auditoria deixou **duas varreduras explicitamente recomendadas e
nunca executadas**. Cada padrão foi descoberto por instrumentação em um único componente, e a
suspeita registrada era de que houvesse mais ocorrências.

---

## 📊 Resultado

| # | Padrão varrido | Ocorrências | Defeitos | Ação |
| :-: | :--- | :---: | :---: | :--- |
| 1 | Contador de tempo com `if/else` excludente | 27 acumuladores de `DeltaTime` | **0** | Nenhuma — todos são contagem regressiva pura |
| 2 | Sentinela sobre valor de domínio válido | 5 comparações com `NAME_None` | **0** | Nenhuma — `NAME_None` é ausência legítima ali |
| 3 | `TestEqual` com `enum` sem cast (regressão) | 12 candidatos | **1** | ✅ Corrigido |
| 4 | `AddLambda` sobre delegate dinâmico | 30 usos | **0** | Nenhuma — provado pelo compilador |
| 5 | Ator de teste sem posicionamento | 104 spawns em 71 arquivos | **0** | Nenhuma — nenhum teste afetado depende de posição |
| 6 | Teste desabilitado ou fora do filtro | 109 declarações | **0** | Nenhuma |
| 7 | Teste fora do prefixo `Sandbox.` | 109 nomes | **0** | Nenhuma — a suíte alcança todos |

**Suíte após a correção: 445 de 445 verdes, EXIT CODE: 0.**

---

## 🐛 O defeito encontrado

`SBMachineryTests.cpp:118` afirmava mais do que verificava:

```cpp
// Antes — a descrição diz "Idling", a asserção só exclui "Lifting"
TestEqual("State is Idling after detach", MachineryComp->GetMachineryState() != ESBMachineryState::Lifting, true);
```

`ESBMachineryState` tem cinco valores (`Parked`, `Idling`, `Operating`, `Lifting`,
`Excavating`). A asserção passava com **quatro** deles. Se `DetachPayload()` deixasse a
máquina em `Parked` ou `Excavating` — estados errados —, o teste continuaria verde
anunciando "State is Idling".

```cpp
// Depois — verifica o que a descrição promete
TestEqual("State is Idling after detach", (int32)MachineryComp->GetMachineryState(), (int32)ESBMachineryState::Idling);
```

Confirmado em `USBMachineryComponent::DetachPayload` que `Idling` é mesmo o estado correto
quando há operador (`MachineryState = CurrentOperator.IsValid() ? Idling : Parked`), então a
asserção apertada é verdadeira e não mascara defeito de produto.

> **Origem**: sobra da conversão das 166 asserções de `enum` (§13 da auditoria). A linha
> imediatamente acima já usava `(int32)` nos dois lados; esta escapou por ter sido escrita
> como comparação booleana, forma que a varredura original não pegava.

---

## ✅ O que foi verificado e está limpo

### 1 · Contador de tempo com `if/else` excludente
A auditoria encontrou o padrão duas vezes (`USBAtmosphericSafetyComponent`,
`USBPoiseComponent`), onde o tick que zerava o contador não aplicava efeito nenhum e o tempo
excedente era descartado.

Os 27 acumuladores de `DeltaTime` do código-base foram inspecionados um a um. Todos os demais
são **contagem regressiva pura para expiração** — decrementam e, ao chegar a zero, disparam um
encerramento (`ResetCombo`, `ResetHitStop`, `RemoveTag`, remoção de item de lista). Nenhum tem
ramo `else` aplicando efeito contínuo, que é a condição necessária para o defeito existir.

### 2 · Sentinela sobre valor de domínio válido
A auditoria encontrou `FVector::ZeroVector` usado como "ausente" quando a origem é posição
válida do mundo. Não há mais nenhuma comparação com `FVector::ZeroVector` em produção. As 5
comparações com `NAME_None` restantes são legítimas: nome de osso e nome de nível não têm
valor de domínio "vazio" — `NAME_None` significa mesmo ausência.

### 4 · `AddLambda` sobre delegate dinâmico
Os 30 usos restantes são todos sobre delegates nativos. Não precisam de varredura manual: um
`AddLambda` sobre delegate dinâmico **não compila**, e o build está verde.

### 5 · Ator de teste sem posicionamento
Foi o defeito sistêmico de maior alcance da auditoria (36 ocorrências) e a causa real das
falhas de `MotionWarp`, `HitTrace` e `LockOn`. Restam 104 spawns de `AActor` puro em testes,
dos quais muitos sem `SetActorLocation`.

**Não são defeitos**, e a distinção importa: o padrão só causa dano quando o teste depende de
posição. Foram inspecionados individualmente os candidatos de maior risco:

| Teste | Depende de posição? | Situação |
| :--- | :--- | :--- |
| `SBDynamicTickThrottlingTests` | Sim — LOD por distância | ✅ `ActorB` e `ActorC` posicionados; o ator na origem é deliberadamente o "perto", e o viewer também está na origem |
| `SBExecutionTests` | Sim — proximidade | ✅ Vítima posicionada em (500,500,0); a segunda vítima sem posição só testa bloqueio de execução concorrente |
| `SBStealthTests` | Não | Alerta acumula por chamada de API, não por consulta espacial |
| `SBCoverTests` | Não | Exercita tags e estado de cobertura |
| Demais (aflição, nutrição, térmico, imunológico…) | Não | O ator é recipiente de componentes; origem é irrelevante |

### 6 e 7 · Alcance da suíte
Nenhum teste desabilitado, nenhum `xIt`/`xDescribe`. Todas as 109 declarações usam
`ProductFilter` (100) ou `EngineFilter` (9), ambos alcançados pela execução. E **nenhum nome
de teste está fora do prefixo `Sandbox.`** — não há teste órfão que a suíte deixe de executar,
que seria a falha mais silenciosa possível: um teste que nunca roda nunca falha.

---

## 🧭 Conclusão

Os padrões sistêmicos da auditoria estão **esgotados**. Cinco das sete varreduras não
encontraram nada, e a única correção foi de uma asserção fraca, não de defeito de produto.

Isso confirma o que a auditoria já registrava em §16: as raízes sistêmicas foram todas
exploradas, e daqui em diante os defeitos são individuais, com rendimento por hora menor. A
diferença é que agora isso está **verificado**, e não suposto.

> **O que esta validação não cobre**: comportamento não exercitado por teste algum. A suíte
> mede 445 specs verdes; ela não mede o que ninguém escreveu. Nenhuma varredura de padrão
> substitui essa lacuna.
