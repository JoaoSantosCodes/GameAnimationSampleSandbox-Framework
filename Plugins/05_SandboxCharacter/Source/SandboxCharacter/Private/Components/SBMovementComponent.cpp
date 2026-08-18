#include "Components/SBMovementComponent.h"
#include "Movement/Behaviors/SBMovementBehavior.h"
#include "Movement/DataAssets/SBMovementBehaviorDefinition.h"
#include "Movement/DataAssets/SBMovementConfigDataAsset.h"
#include "Movement/Aggregator/SBMovementModifierAggregator.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "Utilities/SBLogCategories.h"

USBMovementComponent::USBMovementComponent()
	: Super()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics; // Garante processamento antes da física
}

void USBMovementComponent::OnReady_Implementation()
{
	Super::OnReady_Implementation();

	AActor* Owner = GetOwner();
	ACharacter* CharOwner = Cast<ACharacter>(Owner);
	if (CharOwner && CharOwner->GetCharacterMovement() && Owner->HasAuthority())
	{
		USBAttributeComponent* AttrComp = Owner->FindComponentByClass<USBAttributeComponent>();
		if (AttrComp)
		{
			FGameplayTag SpeedTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Speed"), false);
			FSBAttribute SpeedAttribute;
			if (SpeedTag.IsValid() && AttrComp->GetAttribute(SpeedTag, SpeedAttribute))
			{
				float CmcMaxWalkSpeed = CharOwner->GetCharacterMovement()->MaxWalkSpeed;
				if (!FMath::IsNearlyEqual(SpeedAttribute.BaseValue, CmcMaxWalkSpeed))
				{
					ensureMsgf(false, TEXT("Anti-Cheat Desync detected on startup for %s! Attribute.Speed BaseValue (%f) diverges from CMC MaxWalkSpeed (%f). Auto-synchronizing base value."), 
						*Owner->GetName(), SpeedAttribute.BaseValue, CmcMaxWalkSpeed);

					AttrComp->SetAttributeBaseValue(SpeedTag, CmcMaxWalkSpeed);
				}
			}

			FGameplayTag StaminaTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Stamina"), false);
			FSBAttribute DummyStamina;
			if (StaminaTag.IsValid() && !AttrComp->GetAttribute(StaminaTag, DummyStamina))
			{
				FSBAttribute StaminaAttr;
				StaminaAttr.BaseValue = 100.f;
				StaminaAttr.CurrentValue = 100.f;
				StaminaAttr.MaxValue = 100.f;
				StaminaAttr.MinValue = 0.f;
				StaminaAttr.bIsPrivate = true; // Replicada como COND_OwnerOnly
				AttrComp->RegisterAttribute(StaminaTag, StaminaAttr);
			}
		}
	}
}

float USBMovementComponent::GetCalculatedMaxSpeed() const
{
	AActor* Owner = GetOwner();
	if (!Owner) return 0.0f;

	ACharacter* CharOwner = Cast<ACharacter>(Owner);
	if (!CharOwner || !CharOwner->GetCharacterMovement()) return 0.0f;

	// Bloqueia velocidade de locomoção se estiver Atordoado (Stunned) ou Congelado (Frozen)
	USBStateComponent* StateComp = Owner->FindComponentByClass<USBStateComponent>();
	if (StateComp)
	{
		FGameplayTag StunnedTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Stunned"), false);
		FGameplayTag FrozenTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Frozen"), false);
		if ((StunnedTag.IsValid() && StateComp->HasTag(StunnedTag)) || (FrozenTag.IsValid() && StateComp->HasTag(FrozenTag)))
		{
			return 0.0f;
		}
	}

	float CmcMaxWalkSpeed = CharOwner->GetCharacterMovement()->MaxWalkSpeed;
	float BaseSpeed = CmcMaxWalkSpeed;

	if (CharOwner->GetCharacterMovement()->IsCrouching())
	{
		// Se o comportamento de Crouch NÃO estiver ativo na pilha (agachamento nativo fora da pilha),
		// usamos MaxWalkSpeedCrouched como base direta. Caso contrário, usamos a base padrão (MaxWalkSpeed)
		// e deixamos que o Aggregator aplique o modificador correspondente de forma predição síncrona.
		FGameplayTag CrouchTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Crouching"), false);
		bool bIsCrouchBehaviorActive = CrouchTag.IsValid() && HasBehavior(CrouchTag);
		if (!bIsCrouchBehaviorActive)
		{
			BaseSpeed = CharOwner->GetCharacterMovement()->MaxWalkSpeedCrouched;
		}
	}

	float MaxSpeed = BaseSpeed;
	if (SpeedModifierAggregator)
	{
		MaxSpeed = SpeedModifierAggregator->CalculateFinalValue(BaseSpeed);
	}

	USBAttributeComponent* AttrComp = Owner->FindComponentByClass<USBAttributeComponent>();
	if (AttrComp)
	{
		FGameplayTag SpeedTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Speed"), false);
		FSBAttribute SpeedAttribute;
		if (SpeedTag.IsValid() && AttrComp->GetAttribute(SpeedTag, SpeedAttribute))
		{
			// Validação leve de desvios pós-inicialização para alertar em ambiente de desenvolvimento
			if (GEngine && !FMath::IsNearlyEqual(SpeedAttribute.BaseValue, CmcMaxWalkSpeed))
			{
				double CurrentTime = FPlatformTime::Seconds();
				if (CurrentTime - LastLogDesyncTime > 5.0)
				{
					UE_LOG(LogSandboxCharacter, Warning, TEXT("Anti-Cheat Warning: Attribute.Speed BaseValue (%f) diverges from CMC MaxWalkSpeed (%f)! Ensure you modify both consistently in runtime."), 
						SpeedAttribute.BaseValue, CmcMaxWalkSpeed);
					LastLogDesyncTime = CurrentTime;
				}
			}

			float BaseAttrVal = SpeedAttribute.BaseValue;
			if (BaseAttrVal > 0.0f)
			{
				float Ratio = SpeedAttribute.CurrentValue / BaseAttrVal;
				MaxSpeed *= Ratio;
			}
		}
	}

	return MaxSpeed;
}

void USBMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (!Owner || DeltaTime <= 0.0f)
	{
		return;
	}

	// 1. Processamento de Estamina (Consumo e Regeneração) - Roda em Cliente e Servidor para predição local
	USBAttributeComponent* AttrComp = Owner->FindComponentByClass<USBAttributeComponent>();
	USBStateComponent* StateComp = Owner->FindComponentByClass<USBStateComponent>();
	if (AttrComp && StateComp)
	{
		float SprintCost = DefaultMovementConfig ? DefaultMovementConfig->StaminaConfig.SprintCost : 15.0f;
		float JumpCost = DefaultMovementConfig ? DefaultMovementConfig->StaminaConfig.JumpCost : 20.0f;
		float RegenRate = DefaultMovementConfig ? DefaultMovementConfig->StaminaConfig.RegenRate : 10.0f;
		float RegenDelay = DefaultMovementConfig ? DefaultMovementConfig->StaminaConfig.RegenDelay : 1.5f;
		float ExhaustionThreshold = DefaultMovementConfig ? DefaultMovementConfig->StaminaConfig.ExhaustionRecoveryThreshold : 30.0f;

		float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
		FGameplayTag SprintStateTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Sprinting"), false);
		FGameplayTag ExhaustedTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Exhausted"), false);
		FGameplayTag StaminaTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Stamina"), false);

		bool bIsSprinting = SprintStateTag.IsValid() && StateComp->HasTag(SprintStateTag);

		if (bIsSprinting)
		{
			float CurrentStamina = AttrComp->GetAttributeValue(StaminaTag);
			float NewStamina = FMath::Max(0.f, CurrentStamina - (SprintCost * DeltaTime));
			AttrComp->SetAttributeBaseValue(StaminaTag, NewStamina);
			LastStaminaConsumptionTime = CurrentTime;
		}
		else
		{
			if (CurrentTime - LastStaminaConsumptionTime >= RegenDelay)
			{
				FSBAttribute StaminaAttr;
				if (StaminaTag.IsValid() && AttrComp->GetAttribute(StaminaTag, StaminaAttr))
				{
					if (StaminaAttr.BaseValue < StaminaAttr.MaxValue)
					{
						float NewStamina = FMath::Min(StaminaAttr.MaxValue, StaminaAttr.BaseValue + (RegenRate * DeltaTime));
						AttrComp->SetAttributeBaseValue(StaminaTag, NewStamina);
					}
				}
			}
		}

		// Validação unificada do estado de exaustão baseado no valor final
		if (StaminaTag.IsValid() && ExhaustedTag.IsValid())
		{
			float CurrentStamina = AttrComp->GetAttributeValue(StaminaTag);
			if (CurrentStamina <= 0.f && !StateComp->HasTag(ExhaustedTag))
			{
				StateComp->AddTag(ExhaustedTag);
				FGameplayTag SprintBehaviorTag = FGameplayTag::RequestGameplayTag(TEXT("Movement.Action.Sprint"), false);
				if (SprintBehaviorTag.IsValid())
				{
					StopBehavior(SprintBehaviorTag);
				}
			}
			else if (CurrentStamina >= ExhaustionThreshold && StateComp->HasTag(ExhaustedTag))
			{
				StateComp->RemoveTag(ExhaustedTag);
			}
		}
	}

	// 2. Validações de Segurança (Anti-Cheat) - Apenas Servidor Autoritativo
	if (!Owner->HasAuthority())
	{
		return;
	}

	ACharacter* CharOwner = Cast<ACharacter>(Owner);
	if (!CharOwner || !CharOwner->GetCharacterMovement())
	{
		return;
	}

	FVector CurrentLocation = Owner->GetActorLocation();

	// Se for o primeiro frame ou acabamos de iniciar, inicializa LastValidatedLocation
	if (!bHasLastValidatedLocation)
	{
		LastValidatedLocation = CurrentLocation;
		bHasLastValidatedLocation = true;
		return;
	}

	// Mecanismo de Realocação Autorizada (Teleportes lícitos do servidor)
	if (bServerAuthorizedRelocation)
	{
		bServerAuthorizedRelocation = false;
		LastValidatedLocation = CurrentLocation;
		return;
	}

	// Validação 2D e Total
	float Distance2D = FVector::Dist2D(CurrentLocation, LastValidatedLocation);
	
	// Consulta a velocidade teórica máxima calculada de forma centralizada
	float MaxSpeed = GetCalculatedMaxSpeed();

	// Margem de segurança para acomodar latência, frames lentos e desvios aceitáveis
	float SpeedTolerance = DefaultMovementConfig ? DefaultMovementConfig->AntiCheatConfig.SpeedTolerance : 300.0f;
	float BaseDistanceMargin = DefaultMovementConfig ? DefaultMovementConfig->AntiCheatConfig.BaseDistanceMargin : 200.0f;
	float WarpThreshold = DefaultMovementConfig ? DefaultMovementConfig->AntiCheatConfig.WarpThreshold : 3000.0f;

	float MaxAllowedDistance = (MaxSpeed + SpeedTolerance) * DeltaTime + BaseDistanceMargin;

	bool bIsCheatDetected = false;

	// 1. Detecção de Velocidade excessiva (Speedhack)
	if (Distance2D > MaxAllowedDistance)
	{
		bIsCheatDetected = true;
	}

	// 2. Detecção de Teleporte instantâneo (Warp Hack)
	if (FVector::Dist(CurrentLocation, LastValidatedLocation) > WarpThreshold)
	{
		bIsCheatDetected = true;
	}

	if (bIsCheatDetected)
	{
		UE_LOG(LogSandboxCharacter, Warning, TEXT("Anti-Cheat: Movimento anômalo detectado em %s! Distancia2D: %f (Max Permitido: %f). Executando Rollback."), *Owner->GetName(), Distance2D, MaxAllowedDistance);
		
		// Força o rollback
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

void USBMovementComponent::OnInitialize_Implementation()
{
	Super::OnInitialize_Implementation();

	// Instancia o registro de comportamentos e o agregador de velocidade
	BehaviorRegistry = NewObject<USBBehaviorRegistry>(this);
	SpeedModifierAggregator = NewObject<USBMovementModifierAggregator>(this);

	if (DefaultMovementConfig)
	{
		LoadMovementConfig(DefaultMovementConfig);
	}
}

bool USBMovementComponent::RequestBehavior(FGameplayTag BehaviorTag)
{
	if (USBBehaviorStackComponent::RequestBehavior(BehaviorTag))
	{
		AActor* Owner = GetOwner();
		const bool bIsServer = Owner && Owner->HasAuthority();
		const bool bIsLocallyControlled = (Owner && Cast<APawn>(Owner) && Cast<APawn>(Owner)->IsLocallyControlled());

		// Se for o cliente local (predição), notifica o servidor para validação autoritativa
		if (!bIsServer && bIsLocallyControlled)
		{
			ServerRequestBehavior(BehaviorTag);
		}
		return true;
	}
	return false;
}

USBMovementBehavior* USBMovementComponent::GetCurrentBehavior() const
{
	return Cast<USBMovementBehavior>(USBBehaviorStackComponent::GetCurrentBehavior());
}

void USBMovementComponent::LoadMovementConfig(USBMovementConfigDataAsset* NewConfig)
{
	if (!NewConfig || !BehaviorRegistry) return;

	for (const FSBMovementBehaviorConfigEntry& ConfigEntry : NewConfig->ConfiguredBehaviors)
	{
		if (ConfigEntry.BehaviorClass && ConfigEntry.DefinitionAsset)
		{
			FGameplayTag Tag = ConfigEntry.DefinitionAsset->BehaviorTag;
			UObject* InstancedObj = BehaviorRegistry->GetOrInstantiateBehavior(Tag, ConfigEntry.BehaviorClass);
			USBMovementBehavior* BehaviorInstance = Cast<USBMovementBehavior>(InstancedObj);
			if (BehaviorInstance)
			{
				BehaviorInstance->Initialize(this, ConfigEntry.DefinitionAsset);
				AvailableBehaviors.AddUnique(BehaviorInstance);
			}
		}
	}
}

void USBMovementComponent::ApplyMovementModifiers(FGameplayTag SourceTag, const TArray<FSBModifierEntry>& Entries)
{
	if (!SourceTag.IsValid()) return;

	TArray<FSBModifierEntry> SpeedModifiers;

	for (const FSBModifierEntry& Entry : Entries)
	{
		// Validação estrita contra tags vazias/inválidas no Data Asset para evitar falhas silenciosas
		if (!ensureMsgf(Entry.TargetStatTag.IsValid(), TEXT("ApplyMovementModifiers: Modifier Entry de %s possui TargetStatTag inválida ou vazia!"), *SourceTag.ToString()))
		{
			continue;
		}

		// Distribui modificadores direcionados a velocidade para o agregador correspondente
		if (Entry.TargetStatTag.ToString().Contains(TEXT("Speed")))
		{
			FSBModifierEntry OverrideEntry = Entry;
			OverrideEntry.SourceTag = SourceTag; // Força a consistência lógica programática contra erros do Data Asset
			SpeedModifiers.Add(OverrideEntry);
		}
	}

	if (SpeedModifiers.Num() > 0 && SpeedModifierAggregator)
	{
		SpeedModifierAggregator->SetModifiersForSource(SourceTag, SpeedModifiers);
	}
}

void USBMovementComponent::RemoveMovementModifiers(FGameplayTag SourceTag)
{
	if (!SourceTag.IsValid()) return;

	if (SpeedModifierAggregator)
	{
		SpeedModifierAggregator->ClearModifiersForSource(SourceTag);
	}
}

void USBMovementComponent::OnBehaviorEjected(FGameplayTag BehaviorTag, bool bSkipServerNotify, bool bSkipClientNotify)
{
	AActor* Owner = GetOwner();
	const bool bIsServer = Owner && Owner->HasAuthority();

	// Se for o servidor, o dono for controlado por um player controller remoto e não tivermos o skip ativo,
	// notifica o cliente proprietário para ejetar localmente também e sincronizar o Aggregator
	if (bIsServer && Owner && !bSkipClientNotify)
	{
		APawn* PawnOwner = Cast<APawn>(Owner);
		if (PawnOwner && PawnOwner->GetController() && PawnOwner->GetController()->IsPlayerController() && !PawnOwner->IsLocallyControlled())
		{
			ClientStopBehavior(BehaviorTag);
		}
	}
	// Se for o cliente local, notifica o servidor para parar autoritativamente (se não skipado)
	else if (!bIsServer && !bSkipServerNotify)
	{
		const bool bIsLocallyControlled = (Owner && Cast<APawn>(Owner) && Cast<APawn>(Owner)->IsLocallyControlled());
		if (bIsLocallyControlled)
		{
			ServerStopBehavior(BehaviorTag);
		}
	}
}

USBMovementBehavior* USBMovementComponent::FindAvailableBehaviorByTag(FGameplayTag Tag) const
{
	return Cast<USBMovementBehavior>(USBBehaviorStackComponent::FindAvailableBehaviorByTag(Tag));
}

USBMovementBehavior* USBMovementComponent::FindActiveBehaviorByTag(FGameplayTag Tag) const
{
	return Cast<USBMovementBehavior>(USBBehaviorStackComponent::FindActiveBehaviorByTag(Tag));
}

bool USBMovementComponent::ServerRequestBehavior_Validate(FGameplayTag BehaviorTag)
{
	if (!MovementRPCLimiter.AllowRPC(GetWorld(), 20.0f))
	{
		return false;
	}

	if (!FindAvailableBehaviorByTag(BehaviorTag))
	{
		return false;
	}

	return true;
}

void USBMovementComponent::ServerRequestBehavior_Implementation(FGameplayTag BehaviorTag)
{
	// O servidor tenta ativar o comportamento autoritativamente.
	// Se não conseguir (CanEnter falhar), força o rollback no cliente.
	if (!RequestBehavior(BehaviorTag))
	{
		ClientStopBehavior(BehaviorTag);
	}
}

bool USBMovementComponent::ServerStopBehavior_Validate(FGameplayTag BehaviorTag)
{
	if (!MovementRPCLimiter.AllowRPC(GetWorld(), 20.0f))
	{
		return false;
	}
	return true;
}

void USBMovementComponent::ServerStopBehavior_Implementation(FGameplayTag BehaviorTag)
{
	// O servidor processa a parada autoritativa confirmando o pedido do cliente (pula notificar o cliente de volta)
	StopBehavior(BehaviorTag, true, true);
}

void USBMovementComponent::ClientStopBehavior_Implementation(FGameplayTag BehaviorTag)
{
	// O cliente executa a parada imediata e a limpeza (pulando re-envio de RPC ao servidor)
	StopBehavior(BehaviorTag, true, true);
}

void USBTestMovementComponent::ClientStopBehavior_Implementation(FGameplayTag BehaviorTag)
{
	LastClientStopBehaviorTag = BehaviorTag;
	Super::ClientStopBehavior_Implementation(BehaviorTag);
}

bool USBMovementComponent::ConsumeJumpStamina()
{
	USBAttributeComponent* AttrComp = GetOwner()->FindComponentByClass<USBAttributeComponent>();
	USBStateComponent* StateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	if (AttrComp && StateComp)
	{
		float JumpCost = DefaultMovementConfig ? DefaultMovementConfig->StaminaConfig.JumpCost : 20.0f;
		float ExhaustionThreshold = DefaultMovementConfig ? DefaultMovementConfig->StaminaConfig.ExhaustionRecoveryThreshold : 30.0f;

		FGameplayTag ExhaustedTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Exhausted"), false);
		if (ExhaustedTag.IsValid() && StateComp->HasTag(ExhaustedTag))
		{
			return false; // Bloqueado
		}

		FGameplayTag StaminaTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Stamina"), false);
		float CurrentStamina = AttrComp->GetAttributeValue(StaminaTag);
		if (CurrentStamina >= JumpCost)
		{
			float NewStamina = FMath::Max(0.f, CurrentStamina - JumpCost);
			AttrComp->SetAttributeBaseValue(StaminaTag, NewStamina);
			LastStaminaConsumptionTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

			if (NewStamina <= 0.f && ExhaustedTag.IsValid())
			{
				StateComp->AddTag(ExhaustedTag);
				FGameplayTag SprintBehaviorTag = FGameplayTag::RequestGameplayTag(TEXT("Movement.Action.Sprint"), false);
				if (SprintBehaviorTag.IsValid())
				{
					StopBehavior(SprintBehaviorTag);
				}
			}
			return true;
		}
	}
	return false;
}
