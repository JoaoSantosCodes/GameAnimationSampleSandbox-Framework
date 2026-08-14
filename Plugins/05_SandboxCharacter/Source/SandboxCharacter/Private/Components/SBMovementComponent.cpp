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
#include "Utilities/SBLogCategories.h"

USBMovementComponent::USBMovementComponent()
	: Super()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics; // Garante processamento antes da física
}

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

	if (!bHasLastValidatedLocation)
	{
		LastValidatedLocation = Owner->GetActorLocation();
		bHasLastValidatedLocation = true;
		return;
	}

	FVector CurrentLocation = Owner->GetActorLocation();
	
	// Validação 2D e Total
	float Distance2D = FVector::Dist2D(CurrentLocation, LastValidatedLocation);
	float MaxSpeed = CharOwner->GetCharacterMovement()->GetMaxSpeed();

	// Margem de segurança para acomodar latência, frames lentos e desvios aceitáveis
	float ExtraTolerance = 300.0f;
	float MaxAllowedDistance = (MaxSpeed + ExtraTolerance) * DeltaTime + 200.0f;

	bool bIsCheatDetected = false;

	// 1. Detecção de Velocidade excessiva (Speedhack)
	if (Distance2D > MaxAllowedDistance)
	{
		bIsCheatDetected = true;
	}

	// 2. Detecção de Teleporte instantâneo (Warp Hack - limite de 3000 unidades)
	if (FVector::Dist(CurrentLocation, LastValidatedLocation) > 3000.0f)
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
		LastValidatedLocation = Owner->GetActorLocation();
	}
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
				AvailableBehaviors.Add(BehaviorInstance);
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
